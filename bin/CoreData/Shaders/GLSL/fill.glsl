#include "Uniforms.glsl"
#include "Transform.glsl"
#include "MathUtil.glsl"

#ifdef COMPILEPS
//uniform float cOutlineWidth;
uniform vec4 cOutlineColor;
#endif

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

	vec4 color;
	color.r = DecodeByteRG(cOutlineColor.rg);
	color.g = DecodeByteRG(cOutlineColor.ba);
	float depth = gl_FragCoord.z;
#ifdef LOGDEPTH
	if(!cCameraOrthoPS)
	{
		gl_FragDepth = log2(1. + positionW)/log2(1. + cFarClipPS);
		depth = gl_FragDepth;
	}
#endif
	color.ba = EncodeFloatRG(depth);
    gl_FragColor = color;
}
