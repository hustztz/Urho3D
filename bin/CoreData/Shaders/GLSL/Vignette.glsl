#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"

#ifdef COMPILEPS
uniform float cVignetteFalloff;
#endif

varying vec2 vScreenPos;

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}

void PS()
{
	vec2 coord = (vScreenPos - 0.5) * vec2(cGBufferInvSize.x/cGBufferInvSize.y,1.) * 2.;
	float rf = sqrt(dot(coord, coord)) * cVignetteFalloff;
	float rf2_1 = rf * rf + 1.0;
	float e = 1.0 / (rf2_1 * rf2_1);
	
	vec4 viewportColor = texture2D(sDiffMap, vScreenPos);
	gl_FragColor = vec4(viewportColor.rgb*e, viewportColor.a);
}
