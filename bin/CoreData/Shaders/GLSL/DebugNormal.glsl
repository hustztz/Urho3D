#include "Uniforms.glsl"
#include "Transform.glsl"

varying vec3 vNormal;
#ifdef LOGDEPTH
varying float positionW;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vNormal = iNormal;
#ifdef LOGDEPTH
	positionW = gl_Position.w;
#endif	
}

void PS()
{
	if(isnan(vNormal.x)||isnan(vNormal.y)||isnan(vNormal.z))
		discard;
#ifdef LOGDEPTH
	if(!cCameraOrthoPS)
		gl_FragDepth = log2(1. + positionW)/log2(1. + cFarClipPS);
#endif
    gl_FragColor = vec4(normalize(vNormal)*0.5+0.5, 1.);
}
