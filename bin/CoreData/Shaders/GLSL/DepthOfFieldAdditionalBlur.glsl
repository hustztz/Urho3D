#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#line 40007

varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform float cViewportDivisior;
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
	// 9-tap tent filter
    vec4 duv = (cGBufferInvSize*cViewportDivisior).xyxy * vec4(1., 1., -1., 0.);
    vec4 acc;

    acc  = texture2D(sDiffMap, vScreenPos - duv.xy);
    acc += texture2D(sDiffMap, vScreenPos - duv.wy) * 2.;
    acc += texture2D(sDiffMap, vScreenPos - duv.zy);

    acc += texture2D(sDiffMap, vScreenPos + duv.zw) * 2.;
    acc += texture2D(sDiffMap, vScreenPos         ) * 4.;
    acc += texture2D(sDiffMap, vScreenPos + duv.xw) * 2.;

    acc += texture2D(sDiffMap, vScreenPos + duv.zy);
    acc += texture2D(sDiffMap, vScreenPos + duv.wy) * 2.;
    acc += texture2D(sDiffMap, vScreenPos + duv.xy);

    gl_FragColor = acc / 16.;
}
