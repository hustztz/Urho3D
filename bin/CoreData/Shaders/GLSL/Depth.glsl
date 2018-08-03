#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"

varying vec3 vTexCoord;
#ifdef LOGDEPTH
varying float positionW;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = vec3(GetTexCoord(iTexCoord), GetDepth(gl_Position));
#ifdef LOGDEPTH
	positionW = gl_Position.w;
#endif	
}

void PS()
{
    #ifdef ALPHAMASK
        float alpha = texture2D(sDiffMap, vTexCoord.xy).a;
        if (alpha < 0.5)
            discard;
    #endif
#ifdef LOGDEPTH
	if(!cCameraOrthoPS)
		gl_FragDepth = log2(1. + positionW)/log2(1. + cFarClipPS);
#endif

    gl_FragColor = vec4(EncodeDepth(vTexCoord.z), 1.0);
}
