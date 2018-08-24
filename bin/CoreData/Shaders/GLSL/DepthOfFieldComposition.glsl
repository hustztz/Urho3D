#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;

#ifdef COMPILEPS
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
	vec4 cs = texture2D(sDiffMap, vScreenPos);
    vec4 cb = texture2D(sNormalMap, vScreenPos);

    vec3 rgb = cs.rgb * cb.a + cb.rgb;

    gl_FragColor = vec4(rgb, cs.a);
}
