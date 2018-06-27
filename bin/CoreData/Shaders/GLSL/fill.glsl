#include "Uniforms.glsl"
#include "Transform.glsl"

#ifdef COMPILEPS
//uniform float cOutlineWidth;
uniform vec4 cOutlineColor;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);

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

    gl_FragColor = cOutlineColor;
}
