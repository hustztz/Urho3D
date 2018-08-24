#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;
varying vec3 frustumSize;

#ifdef COMPILEPS
uniform float cMaxCoC;
uniform float cRcpMaxCoC;
uniform float cDofDistance;
uniform float cDofLensCoeff;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    
	vScreenPos = GetScreenPosPreDiv(gl_Position);
	frustumSize = cFrustumSize;
}

#ifdef COMPILEPS
float LinearEyeDepth(vec2 uv)
{
	vec4 depthInput = texture2D(sDepthBuffer, uv);
    #ifdef HWDEPTH
        float depth = ReconstructDepth(depthInput.r);
		//float depth = clamp(2. * cNearClipPS / (cFarClipPS + cNearClipPS - (depthInput.r*2.-1.) * ( cFarClipPS - cNearClipPS )), 0., 1.);
	#else
		float depth = DecodeDepth(depthInput.rgb);
    #endif
	return depth * frustumSize.z - cNearClipPS;
}
#endif

void PS()
{
	// Sample source colors.
    vec3 duv = cGBufferInvSize.xyx * vec3(0.5, 0.5, -0.5);
    vec3 c0 = texture2D(sDiffMap, vScreenPos - duv.xy).rgb;
    vec3 c1 = texture2D(sDiffMap, vScreenPos - duv.zy).rgb;
    vec3 c2 = texture2D(sDiffMap, vScreenPos + duv.zy).rgb;
    vec3 c3 = texture2D(sDiffMap, vScreenPos + duv.xy).rgb;

    // Sample linear depths.
    float d0 = LinearEyeDepth(vScreenPos - duv.xy);
    float d1 = LinearEyeDepth(vScreenPos - duv.zy);
    float d2 = LinearEyeDepth(vScreenPos + duv.zy);
    float d3 = LinearEyeDepth(vScreenPos + duv.xy);
    vec4 depths = vec4(d0, d1, d2, d3);

    // Calculate the radiuses of CoCs at these sample points.
    vec4 cocs = (depths - cDofDistance) * cDofLensCoeff / depths;
    cocs = clamp(cocs, -cMaxCoC, cMaxCoC);

    // Premultiply CoC to reduce background bleeding.
    vec4 weights = clamp(abs(cocs) * cRcpMaxCoC, 0., 1.);

    // Weighted average of the color samples
    vec3 avg = c0 * weights.x + c1 * weights.y + c2 * weights.z + c3 * weights.w;
    avg /= dot(weights, vec4(1.));

    // Output CoC = average of CoCs
    float coc = dot(cocs, vec4(0.25));

    // Premultiply CoC again.
    avg *= smoothstep(0., cGBufferInvSize.y * 2., abs(coc));

    gl_FragColor =  vec4(avg, coc);
}
