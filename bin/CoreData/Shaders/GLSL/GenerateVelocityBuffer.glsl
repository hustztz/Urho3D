#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;
varying vec3 frustumSize;
varying mat4 viewInv;

#ifdef COMPILEPS
uniform mat4 cLastVPMatrix;
#endif

#ifdef COMPILEPS
//----------------------------------------------------------------------------------

vec3 fetch_eye_pos(vec2 uv)
{
    vec4 depthInput = texture2D(sDepthBuffer, uv);
    #ifdef HWDEPTH
        float depth = ReconstructDepth(depthInput.r);
	#else
       float depth = DecodeDepth(depthInput.rgb);
    #endif
	vec3 vFarRay = vec3(frustumSize.xy*uv, frustumSize.z);
    return vFarRay * depth;
}


#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    frustumSize = cFrustumSize;
	vScreenPos = GetScreenPosPreDiv(gl_Position);
	viewInv = cViewInv;
}


void PS()
{
	vec3 posInView = fetch_eye_pos(vScreenPos);
	
	vec4 posInWorld = vec4(posInView, 1.) * viewInv;
	
	vec4 rp_cs_pos = posInWorld * cLastVPMatrix;
	vec2 rp_ss_ndc = rp_cs_pos.xy / rp_cs_pos.w;
	vec2 rp_ss_txc = 0.5 * rp_ss_ndc + 0.5;
	
	gl_FragColor = vec4(vScreenPos - rp_ss_ndc, 0., 0.);
}
