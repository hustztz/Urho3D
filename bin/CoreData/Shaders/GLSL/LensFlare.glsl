#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec2 vScreenPos;

#ifdef COMPILEPS
#ifdef SCALEFLARE1
uniform vec4 cLensFlareColor1;
uniform float cLensFlareScale1;
#endif
#ifdef SCALEFLARE2
uniform vec4 cLensFlareColor2;
uniform float cLensFlareScale2;
#endif
#ifdef SCALEFLARE3
uniform vec4 cLensFlareColor3;
uniform float cLensFlareScale3;
#endif
#ifdef SCALEFLARE4
uniform vec4 cLensFlareColor4;
uniform float cLensFlareScale4;
#endif
#ifdef SCALEFLARE5
uniform vec4 cLensFlareColor5;
uniform float cLensFlareScale5;
#endif
#ifdef SCALEFLARE6
uniform vec4 cLensFlareColor6;
uniform float cLensFlareScale6;
#endif
#ifdef SCALEFLARE7
uniform vec4 cLensFlareColor7;
uniform float cLensFlareScale7;
#endif
#ifdef SCALEFLARE8
uniform vec4 cLensFlareColor8;
uniform float cLensFlareScale8;
#endif
#endif

#ifdef COMPILEPS
vec4 computeLensFlareScale(vec2 texCoord, vec4 color, float scale)
{
	vec4 outColor;
	vec2 uv = vec2(0.5) + (texCoord - vec2(0.5)) / scale;
	if(uv.x < 0. || uv.x > 1.|| uv.y < 0. || uv.y > 1.)
		return vec4(0.);
	vec4 scaled = texture(sDiffMap, uv) * color;
	float screenBorderMask = DiscMask(texCoord*1.5);
	//screenBorderMask *= DiscMask(texCoord);
	outColor.rgb = scaled.rgb ;
	outColor.a =  0.;
	return outColor;
}
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vTexCoord = GetQuadTexCoord(gl_Position);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
#ifdef SCALEFLARE1
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor1, cLensFlareScale1);
#endif
#ifdef SCALEFLARE2
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor2, cLensFlareScale2);
#endif
#ifdef SCALEFLARE3
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor3, cLensFlareScale3);
#endif
#ifdef SCALEFLARE4
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor4, cLensFlareScale4);
#endif
#ifdef SCALEFLARE5
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor5, cLensFlareScale5);
#endif
#ifdef SCALEFLARE6
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor6, cLensFlareScale6);
#endif
#ifdef SCALEFLARE7
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor7, cLensFlareScale7);
#endif
#ifdef SCALEFLARE8
	gl_FragColor = computeLensFlareScale(vScreenPos, cLensFlareColor8, cLensFlareScale8);
#endif
}
