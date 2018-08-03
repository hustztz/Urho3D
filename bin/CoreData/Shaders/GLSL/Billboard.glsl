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
	mat4 modelView = modelMatrix * cView;
	// First colunm.
	modelView[0][0] = cBillboardSize.x;
	modelView[0][1] = 0.0;
	modelView[0][2] = 0.0;

	// Second colunm.
	modelView[1][0] = 0.0;
	modelView[1][1] = cBillboardSize.y;
	modelView[1][2] = 0.0;

	// Thrid colunm.
	modelView[2][0] = 0.0;
	modelView[2][1] = 0.0;
	modelView[2][2] = cBillboardSize.y;
	
	vec4 p = iPos * modelView;
	gl_Position = p * cViewInv * cViewProj;
	
#ifdef LOGDEPTH
	positionW = gl_Position.w;
#endif	
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
