#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "DiskKernel.glsl"
#include "Constants.glsl"
#line 40007

varying vec2 vScreenPos;

#ifdef COMPILEPS
uniform float cViewportDivisior;
uniform float cMaxCoC;
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
#line 1110000
    vec4 samp0 = texture2D(sDiffMap, vScreenPos);

    vec4 bgAcc = vec4(0.); // Background: far field bokeh
    vec4 fgAcc = vec4(0.); // Foreground: near field bokeh

    for (int si = 0; si < kSampleCount; si++)
    {
        vec2 disp = kDiskKernel[si] * cMaxCoC;
        float dist = length(disp);

        vec2 duv = vec2(disp.x * cGBufferInvSize.x / cGBufferInvSize.y, disp.y);
        vec4 samp = texture2D(sDiffMap, vScreenPos + duv);

        // BG: Compare CoC of the current sample and the center sample
        // and select smaller one.
        float bgCoC = max(min(samp0.a, samp.a), 0.);

        // Compare the CoC to the sample distance.
        // Add a small margin to smooth out.
        float margin = (cGBufferInvSize*cViewportDivisior).y * 2.;
        float bgWeight = clamp((bgCoC   - dist + margin) / margin, 0., 1.);
        float fgWeight = clamp((-samp.a - dist + margin) / margin, 0., 1.);

        // Cut influence from focused areas because they're darkened by CoC
        // premultiplying. This is only needed for near field.
        fgWeight *= step((cGBufferInvSize*cViewportDivisior).y, -samp.a);

        // Accumulation
        bgAcc += vec4(samp.rgb, 1.) * bgWeight;
        fgAcc += vec4(samp.rgb, 1.) * fgWeight;
    }

    // Get the weighted average.
	if(bgAcc.a == 0.)
		bgAcc.rgb /= bgAcc.a + 1.; // zero-div guard
	else
		bgAcc.rgb /= bgAcc.a; // zero-div guard
	
	if(fgAcc.a == 0.)
		fgAcc.rgb /= fgAcc.a + 1.; // zero-div guard
	else
		fgAcc.rgb /= fgAcc.a; // zero-div guard

    // BG: Calculate the alpha value only based on the center CoC.
    // This is a rather aggressive approximation but provides stable results.
    bgAcc.a = smoothstep((cGBufferInvSize*cViewportDivisior).y, (cGBufferInvSize*cViewportDivisior).y * 2, samp0.a);

    // FG: Normalize the total of the weights.
    fgAcc.a *= 6. * M_PI / float(kSampleCount);

    // Alpha premultiplying
    vec3 rgb = vec3(0.);
    rgb = mix(rgb, bgAcc.rgb, clamp(bgAcc.a, 0., 1.));
    rgb = mix(rgb, fgAcc.rgb, clamp(fgAcc.a, 0., 1.));

    // Combined alpha value
    float alpha = (1. - clamp(bgAcc.a, 0., 1.)) * (1. - clamp(fgAcc.a, 0., 1.));

    gl_FragColor = vec4(rgb, alpha);
}
