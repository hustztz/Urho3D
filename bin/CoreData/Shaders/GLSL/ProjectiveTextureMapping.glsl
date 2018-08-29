#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

#ifdef COMPILEPS
uniform mat4 cPTMViewProjectionMatrix;
#endif

varying vec2 vScreenPos;
varying vec3 frustumSize;
varying mat4 viewInv;

#ifdef COMPILEPS
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
	vec3 vFarRay = vec3(frustumSize.xy*(uv*2.-1.), frustumSize.z);
    return vFarRay * depth;
}

vec4 clipPosition;
vec2 getTexCoordInMap(vec3 worldPosition, mat4 viewProjectionMatrix)
{
	clipPosition = vec4(worldPosition, 1.) * viewProjectionMatrix;
	clipPosition /= clipPosition.w;
	if(clipPosition.z < 0. || clipPosition.z > 1.)
	{
		return vec2(0.);
	}else
	{
		clipPosition.xy = clipPosition.xy * 0.5 + 0.5;
		clipPosition.y = 1. - clipPosition.y;
		return clipPosition.xy;
	}
}

bool check(vec2 mapTc)
{
	return mapTc.x > 0. && mapTc.x < 1. && mapTc.y > 0. && mapTc.y < 1.;
}

#endif
void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
	frustumSize = cFrustumSize;
	viewInv = cViewInv;
}

void PS()
{
	vec4 color = texture2D(sDiffMap, vScreenPos.xy);
	
	float D = texture2D(sDepthBuffer, vScreenPos).r;
	if( D >= 1.)
	{
		gl_FragColor = color;
		return;
	}
	
	vec3 vPosition = fetch_eye_pos(vScreenPos);
	vec4 wPosition = vec4(vPosition, 1.) * viewInv;
	wPosition /= wPosition.w;
	
	gl_FragColor = color;
	
	vec2 mapTc1 = getTexCoordInMap(wPosition.xyz, cPTMViewProjectionMatrix);
	if(check(mapTc1))
	{
		vec4 projectionColor = texture2D(sNormalMap, mapTc1);
		gl_FragColor = projectionColor;//color*(1.-projectionColor.a)+projectionColor*projectionColor.a;
	}
}
