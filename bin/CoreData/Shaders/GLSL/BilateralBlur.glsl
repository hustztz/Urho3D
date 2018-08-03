#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform float cBilateralBlurRadius;
uniform float cBilateralSharpness;
uniform bool cBilateralBlurCombine;

float BlurFunction(vec2 uv, float r, float center_c, float center_d, inout float w_total)
{
    float c = texture2D(sDiffMap, uv).r;
    vec4 depthInput = texture2D(sDepthBuffer, uv);
#ifdef HWDEPTH
	float d = ReconstructDepth(depthInput.r);
#else
	float d = DecodeDepth(depthInput.rgb);
#endif	

    float ddiff = d - center_d;
	float sigma = (cBilateralBlurRadius+1.)/2.;
	float inv_sigma2 = 1./(2.*sigma*sigma);
    float w = exp(-r*r*inv_sigma2 - ddiff*ddiff*cBilateralSharpness);
    w_total += w;

    return w*c;
}
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    
	vScreenPos = GetScreenPosPreDiv(gl_Position);
}


void PS()
{
	float b = 0.;
	float w_total = 0.;
	float center_c = texture2D(sDiffMap, vScreenPos).r;
    vec4 depthInput = texture2D(sDepthBuffer, vScreenPos);
#ifdef HWDEPTH
	float center_d = ReconstructDepth(depthInput.r);
#else
	float center_d = DecodeDepth(depthInput.rgb);
#endif	
	for( float r = -cBilateralBlurRadius; r <= cBilateralBlurRadius; ++r )
	{
#ifdef XBLUR
        vec2 uv = vScreenPos + vec2(r*cGBufferInvSize.x , 0.);
#else
		vec2 uv = vScreenPos + vec2(0., r*cGBufferInvSize.y );
#endif
        b += BlurFunction(uv, r, center_c, center_d, w_total);	
	}
	if(cBilateralBlurCombine)
	{
		gl_FragColor.rgb = texture2D(sSpecMap, vScreenPos).rgb * b/w_total;
		return;
	}
	gl_FragColor.rgb = vec3(b/w_total);
}
