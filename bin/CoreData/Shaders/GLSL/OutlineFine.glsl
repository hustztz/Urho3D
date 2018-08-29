#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "MathUtil.glsl"

#ifdef COMPILEPS
uniform float cOutlineWidth;
//uniform vec4 cOutlineColor;
#endif

varying vec2 vScreenPos;

//const vec4 BlackNoAlpha = vec4(0.5);

const vec2 around0 = vec2(-1., 1.);
const vec2 around1 = vec2(0., 1.);
const vec2 around2 = vec2(1., 1.);
const vec2 around3 = vec2(-1., 0.);
const vec2 around4 = vec2(1., 0.);
const vec2 around5 = vec2(-1., -1.);
const vec2 around6 = vec2(0., -1.);
const vec2 around7 = vec2(1., -1.);

#ifdef COMPILEPS

vec4 decodeColor(vec4 color)
{
	vec2 rg = EncodeByteRG(color.r);
	vec2 ba = EncodeByteRG(color.g);
	vec4 depthInput = texture2D(sDepthBuffer, vScreenPos);
	float alpha = ba.y;
    #ifdef HWDEPTH
	float depth = ReconstructDepth(depthInput.r);
	float depthOutline = ReconstructDepth(DecodeFloatRG(color.ba));
        if(depth < depthOutline - 0.0001)
			alpha *= 0.4;//min(step(0.,(mod(length(vScreenPos.x), 0.002)-0.001)), step(0.,(mod(length(vScreenPos.y), 0.002)-0.001)));
    #endif
	
	return vec4(rg, ba.x, alpha);
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
    vec4 thisColor = texture2D(sNormalMap, vScreenPos);

	if (length(thisColor.rgb) <= 0.) {

		vec4 color = texture2D(sNormalMap, (gl_FragCoord.xy + around0 * cOutlineWidth)
				* cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around1 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around2 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around3 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around4 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around5 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around6 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}
		color = texture2D(sNormalMap, (gl_FragCoord.xy + around7 * cOutlineWidth) * cGBufferInvSize);
		if (length(color) > 0.) {
			gl_FragColor = decodeColor(color);
			return;
		}

	}
	discard;
}
