#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;
varying vec3 frustumSize;
varying vec2 vFocalLen;

#ifdef COMPILEPS
uniform float cAORadius;
uniform float cAORadiusInv;
uniform float cAORadiusSqr;
uniform float cNumSteps;
uniform bool cUseNormal;
uniform float cAONumDir;
uniform float cAOAttenuation;
uniform float cAOAngleBias;
uniform float cAOTanAngleBias;
uniform float cAOContrast;
#endif

#ifdef COMPILEPS
//----------------------------------------------------------------------------------
float tangent(vec3 P, vec3 S)
{
    return (P.z - S.z) / length(S.xy - P.xy);
}

//----------------------------------------------------------------------------------
vec3 fetch_eye_pos(vec2 uv)
{
    vec4 depthInput = texture2D(sDepthBuffer, uv);
    #ifdef HWDEPTH
        float depth = ReconstructDepth(depthInput.r);
		//float depth = clamp(2. * cNearClipPS / (cFarClipPS + cNearClipPS - (depthInput.r*2.-1.) * ( cFarClipPS - cNearClipPS )), 0., 1.);
	#else
       float depth = DecodeDepth(depthInput.rgb);
    #endif
	vec3 vFarRay = vec3(frustumSize.xy*uv, frustumSize.z);
    return vFarRay * depth;
}

//----------------------------------------------------------------------------------
vec3 tangent_eye_pos(vec2 uv, vec4 tangentPlane)
{
    // view vector going through the surface point at uv
    vec3 V = fetch_eye_pos(uv);
    float NdotV = dot(tangentPlane.xyz, V);
    // intersect with tangent plane except for silhouette edges
    if (NdotV < 0.0) V *= (tangentPlane.w / NdotV);
    return V;
}

float length2(vec3 v) { return dot(v, v); } 

//----------------------------------------------------------------------------------
vec3 min_diff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (length2(V1) < length2(V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
float falloff(float r)
{
    return 1.0 - cAOAttenuation*r*r;
}

//----------------------------------------------------------------------------------
vec2 snap_uv_offset(vec2 uv)
{
    return round(uv / cGBufferInvSize) * cGBufferInvSize;
}

vec2 snap_uv_coord(vec2 uv)
{
    return uv - (fract(uv / cGBufferInvSize) - 0.5) * cGBufferInvSize;
}

//----------------------------------------------------------------------------------
float tan_to_sin(float x)
{
    return x / sqrt(x*x + 1.0);
}

//----------------------------------------------------------------------------------
vec3 tangent_vector(vec2 deltaUV, vec3 dPdu, vec3 dPdv)
{
    return deltaUV.x * dPdu + deltaUV.y * dPdv;
}

//----------------------------------------------------------------------------------
float invlength(vec2 v)
{
    return 1. / sqrt(dot(v,v));
}

//----------------------------------------------------------------------------------
float tangent(vec3 T)
{
    return -T.z * invlength(T.xy);
}

//----------------------------------------------------------------------------------
float biased_tangent(vec3 T)
{
    float phi = atan(tangent(T)) + cAOAngleBias;
    return tan(min(phi, 3.1415926*0.5));
}

//----------------------------------------------------------------------------------
void integrate_direction(inout float ao, vec3 P, vec2 uv, vec2 deltaUV,
                         float numSteps, float tanH, float sinH)
{
    for (float j = 1.; j <= numSteps; ++j) {
        uv += deltaUV;
        vec3 S = fetch_eye_pos(uv);
        
        // Ignore any samples outside the radius of influence
        float d2  = length2(S - P);
        if (d2 < cAORadiusSqr) {
            float tanS = tangent(P, S);

            if(tanS > tanH) {
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0 + tanS*tanS);
                float r = sqrt(d2) * cAORadiusInv;
                ao += falloff(r) * (sinS - sinH);
                
                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        }
    }
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion_LowQuality(vec2 deltaUV, 
                                             vec2 uv0, 
                                             vec3 P, 
                                             float numSteps, 
                                             float randstep)
{
    // Randomize starting point within the first sample distance
    vec2 uv = uv0 + snap_uv_offset( randstep * deltaUV );
    
    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    deltaUV = snap_uv_offset( deltaUV );

    // Add an epsilon in case cAOAngleBias==0.0
    float tanT = tan(-3.1415926*0.5 + cAOAngleBias + 1.e-5);
    float sinT = (cAOAngleBias != 0.0) ? tan_to_sin(tanT) : -1.0;

    float ao = 0;
    integrate_direction(ao, P, uv, deltaUV, numSteps, tanT, sinT);

    // Integrate opposite directions together
    deltaUV = -deltaUV;
    uv = uv0 + snap_uv_offset( randstep * deltaUV );
    integrate_direction(ao, P, uv, deltaUV, numSteps, tanT, sinT);

    // Divide by 2 because we have integrated 2 directions together
    // Subtract 1 and clamp to remove the part below the surface
    return max(ao * 0.5 - 1.0, 0.0);
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion(vec2 deltaUV, 
                                  vec2 uv0, 
                                  vec3 P, 
                                  float numSteps, 
                                  float randstep,
                                  vec3 dPdu,
                                  vec3 dPdv )
{
    // Randomize starting point within the first sample distance
    vec2 uv = uv0 + snap_uv_offset( randstep * deltaUV );
    
    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    deltaUV = snap_uv_offset( deltaUV );

    // Compute tangent vector using the tangent plane
    vec3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    float tanH = biased_tangent(T);
    float sinH = tanH / sqrt(1.0 + tanH*tanH);

    float ao = 0.;
    for(float j = 1.; j <= numSteps; ++j) {
        uv += deltaUV;
        vec3 S = fetch_eye_pos(uv);
        
        // Ignore any samples outside the radius of influence
        float d2  = length2(S - P);
        if (d2 < cAORadiusSqr) {
            float tanS = tangent(P, S);

            if(tanS > tanH) {
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0 + tanS*tanS);
                float r = sqrt(d2) * cAORadiusInv;
                ao += falloff(r) * (sinS - sinH);
                
                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        } 
    }

    return ao;
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion_Quality(vec2 deltaUV, 
                                          vec2 uv0, 
                                          vec3 P, 
                                          float numSteps, 
                                          float randstep,
                                          vec3 dPdu,
                                          vec3 dPdv )
{
    // Jitter starting point within the first sample distance
    vec2 uv = (uv0 + deltaUV) + randstep * deltaUV;
    
    // Snap first sample uv and initialize horizon tangent
    vec2 snapped_duv = snap_uv_offset(uv - uv0);
    vec3 T = tangent_vector(snapped_duv, dPdu, dPdv);
    float tanH = tangent(T) + cAOTanAngleBias;

    float ao = 0.;
    float h0 = 0.;
    for(float j = 0.; j < numSteps; ++j) {
        vec2 snapped_uv = snap_uv_coord(uv);
        vec3 S = fetch_eye_pos(snapped_uv);
        uv += deltaUV;

        // Ignore any samples outside the radius of influence
        float d2 = length2(S - P);
        if (d2 < cAORadiusSqr) {
            float tanS = tangent(P, S);

            if (tanS > tanH) {
                // Compute tangent vector associated with snapped_uv
                vec2 snapped_duv = snapped_uv - uv0;
                vec3 T = tangent_vector(snapped_duv, dPdu, dPdv);
                float tanT = tangent(T) + cAOTanAngleBias;

                // Compute AO between tangent T and sample S
                float sinS = tan_to_sin(tanS);
                float sinT = tan_to_sin(tanT);
                float r = sqrt(d2) * cAORadiusInv;
                float h = sinS - sinT;
                ao += falloff(r) * (h - h0);
                h0 = h;

                // Update the current horizon angle
                tanH = tanS;
            }
        }
    }
    return ao;
}

//----------------------------------------------------------------------------------
vec2 rotate_direction(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y, 
                  Dir.x*CosSin.y + Dir.y*CosSin.x);
}

vec2 random2(vec2 uv)
{
	//vec2 noiseTc = fract(uv / cGBufferInvSize * vec2(64.));
	vec2 noiseTc = mod(uv / cGBufferInvSize , vec2(64.))/ vec2(64.);
	vec2 rand = texture2D(sDiffMap, noiseTc).xy;
	return rand;
}
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    
	vScreenPos = GetScreenPosPreDiv(gl_Position);
    //vFarRay = vec3(
    //    gl_Position.x / gl_Position.w * cFrustumSize.x,
     //   gl_Position.y / gl_Position.w * cFrustumSize.y,
    //    cFrustumSize.z);
	frustumSize = cFrustumSize;
	vFocalLen = cFrustumSize.xy/cFrustumSize.z;
}


void PS()
{
	vec3 P = fetch_eye_pos(vScreenPos);
	vec2 step_size = abs(0.5 * cAORadius * vFocalLen / P.z);

	// Early out if the projected radius is smaller than 1 pixel.
	float numSteps = min(cNumSteps, min(step_size.x / cGBufferInvSize.x, step_size.y / cGBufferInvSize.y));

	if(numSteps < 1.)
	{
		gl_FragColor = vec4(1.);
		return;
	}
	step_size = step_size / ( numSteps + 1 );

	// Nearest neighbor pixels on the tangent plane
    vec3 Pr, Pl, Pt, Pb;

	if(cUseNormal)
	{
		vec4 normalInput = texture2D(sNormalBuffer, vScreenPos);
		if(isnan(normalInput.x)||isnan(normalInput.y)||isnan(normalInput.z))
		{
			gl_FragColor = vec4(1.);
			return;
		}
		vec3 N = normalInput.rgb;
		N = (vec4(N,0.)*cView).xyz;
		//N = vec3(-N.xy, -N.z);
		N = normalize(N);
		//gl_FragColor = vec4(N, 1.);
		//return;

        vec4 tangentPlane = vec4(N, dot(P, N));
        Pr = tangent_eye_pos(vScreenPos + vec2(cGBufferInvSize.x, 0.), tangentPlane);
        Pl = tangent_eye_pos(vScreenPos + vec2(-cGBufferInvSize.x, 0.), tangentPlane);
        Pt = tangent_eye_pos(vScreenPos + vec2(0., cGBufferInvSize.y), tangentPlane);
        Pb = tangent_eye_pos(vScreenPos + vec2(0., -cGBufferInvSize.y), tangentPlane);		
	}else
	{
		Pr = fetch_eye_pos(vScreenPos + vec2(cGBufferInvSize.x, 0.));
        Pl = fetch_eye_pos(vScreenPos + vec2(-cGBufferInvSize.x, 0.));
        Pt = fetch_eye_pos(vScreenPos + vec2(0., cGBufferInvSize.y));
        Pb = fetch_eye_pos(vScreenPos + vec2(0., -cGBufferInvSize.y));
        //vec3 N = normalize(cross(Pr - Pl, Pt - Pb));
	}
	
    // Screen-aligned basis for the tangent plane
    vec3 dPdu = min_diff(P, Pr, Pl);
    vec3 dPdv = min_diff(P, Pt, Pb) * (cGBufferInvSize.x / cGBufferInvSize.y);

    vec2 randVec2 = random2(vScreenPos);
	vec3 rand;
	float angle = 2. * 3.1415926 * randVec2.x / cNumSteps; 
	rand.x = cos(angle);
	rand.y = sin(angle);
	rand.z = randVec2.y;
	
	float ao = 0.;
	float d;
	float alpha = 2. * 3.1415926 / cAONumDir;
    
//	switch(2)
//	{
//	case 0:
//		for (d = 0.; d < cAONumDir*0.5; ++d) {
//           float angle = alpha * d;
//            vec2 dir = vec2(cos(angle), sin(angle));
//           vec2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
//            ao += AccumulatedHorizonOcclusion_LowQuality(deltaUV, vScreenPos, P, numSteps, rand.z);
//		}
//		ao *= 2.0;
//       break;
//    case 1:
//       for (d = 0.; d < cAONumDir; d++) {
//            float angle = alpha * d;
//            vec2 dir = vec2(cos(angle), sin(angle));
//            vec2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
//            ao += AccumulatedHorizonOcclusion(deltaUV, vScreenPos, P, numSteps, rand.z, dPdu, dPdv);
//        }
//        break;
//    case 2:
        for (d = 0.; d < cAONumDir; d++) {
            float angle = alpha * d;
            vec2 dir = vec2(cos(angle), sin(angle));
            vec2 deltaUV = rotate_direction(dir, rand.xy) * step_size.xy;
            ao += AccumulatedHorizonOcclusion_Quality(deltaUV, vScreenPos, P, numSteps, rand.z, dPdu, dPdv);
        }
//        break;
//   }
	ao = 1.0 - ao / cAONumDir * cAOContrast;
	gl_FragColor = vec4(ao, ao, ao, 1.);
}
