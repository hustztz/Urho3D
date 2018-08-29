#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

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
	gl_FragColor.rgb = texture2D(sDiffMap, vScreenPos).rgb * texture2D(sSpecMap, vScreenPos).rgb;
	gl_FragColor.a = 1.;
}
