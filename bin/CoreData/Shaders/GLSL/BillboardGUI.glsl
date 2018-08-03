#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

#ifdef DIFFMAP
varying vec2 vTexCoord;
#endif
varying float alpha;

#ifdef COMPILEVS
uniform vec2 cBillboardSize;
uniform float cBillBoardFadeNearDistance;
uniform float cBillBoardFadeDistance;
uniform float cBillBoardZOffset;
uniform vec2 cBillBoardScreenOffset;
#endif

#ifdef LOGDEPTH
	varying float positionW;
#endif	

void VS()
{
    mat4 modelMatrix = iModelMatrix;
#ifdef DIFFMAP
    vTexCoord = iTexCoord;
#endif
	
	if (cBillBoardFadeDistance > 0.  || cBillBoardFadeNearDistance > 0.) 
	{
		vec4 startWorldPos = vec4(0., 0., 0., 1.) * modelMatrix;
		float dist = distance(cCameraPos, startWorldPos.xyz);
		if(cBillBoardFadeDistance > 0.)
		{
			float fadeNear = 0.9 * cBillBoardFadeDistance;
			if (dist <= fadeNear) {
				alpha = 1.;
			} else if (dist < cBillBoardFadeDistance) {
				alpha = 1. - abs(dist - fadeNear) / (0.1 * cBillBoardFadeDistance);
			} else {
				alpha = 0.;
				//		discard;
				gl_Position = vec4(0.);
				return;
			}
		}
		if (cBillBoardFadeNearDistance > 0.) 
		{
			float fadeNear = 1.5 * cBillBoardFadeNearDistance;
			if (dist >= fadeNear) {
				alpha = 1.;
			} else if (dist > cBillBoardFadeNearDistance) {
				alpha = abs(dist - cBillBoardFadeNearDistance) / (fadeNear - cBillBoardFadeNearDistance);
			} else {
				alpha = 0.;
				//		discard;
				gl_Position = vec4(0.);
				return;
			}
		}
	}
	vec4 startClipPos = vec4(0., 0., 0., 1.) * modelMatrix * cViewProj;
#ifdef LOGDEPTH
	positionW = startClipPos.w;
#endif		
	startClipPos /= startClipPos.w;
	vec2 endClipPos = startClipPos.xy + cBillboardSize * cGBufferInvSize * 2.;
	startClipPos.xy +=  cBillBoardScreenOffset * cGBufferInvSize * 2.;
	endClipPos.xy +=  cBillBoardScreenOffset * cGBufferInvSize * 2.;

	gl_Position = vec4(startClipPos.xy + iPos.xy * (endClipPos - startClipPos.xy), startClipPos.z,
			1.);

	gl_Position /= gl_Position.w;
	gl_Position.z += cBillBoardZOffset;
}

void PS()
{
#ifdef LOGDEPTH
	if(!cCameraOrthoPS)
		gl_FragDepth = log2(1. + positionW)/log2(1. + cFarClipPS);
#endif
    vec4 diffColor = cMatDiffColor;
	diffColor.a *= alpha;

    #if (!defined(DIFFMAP)) && (!defined(ALPHAMAP))
        gl_FragColor = diffColor;
    #endif
    #ifdef DIFFMAP
        vec4 diffInput = texture2D(sDiffMap, vTexCoord);
        if (diffInput.a < 0.1)
            discard;
        gl_FragColor = diffColor * diffInput;
    #endif
}
