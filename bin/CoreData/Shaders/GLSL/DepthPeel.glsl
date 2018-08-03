#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#ifdef LOGDEPTH
varying float positionW;
#endif
void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
#ifdef LOGDEPTH
	positionW = gl_Position.w;
#endif	
    //vTexCoord = GetTexCoord(iTexCoord);
}

void PS()
{
    // Get material diffuse albedo
    //#ifdef DIFFMAP
    //    vec4 diffInput = texture2D(sDiffMap, vTexCoord.xy);
    //    if (diffInput.a < 0.5)
    //    discard;
    //#endif
#ifdef LOGDEPTH
	if(!cCameraOrthoPS)
		gl_FragDepth = log2(1. + positionW)/log2(1. + cFarClipPS);
#endif
	float frontDepth = 0.;
	#ifdef HASPEELDEPTH
		frontDepth = texture2D(sNormalMap, gl_FragCoord.xy*cGBufferInvSize).r;
	#endif
#ifdef LOGDEPTH
	if(gl_FragDepth <= frontDepth)
		discard;

    gl_FragColor = vec4(gl_FragDepth);
#else	
	if(gl_FragCoord.z <= frontDepth)
		discard;

    gl_FragColor = vec4(gl_FragCoord.z);
#endif
}
