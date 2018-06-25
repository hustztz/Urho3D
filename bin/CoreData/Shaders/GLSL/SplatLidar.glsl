#include "Uniforms.glsl"
#include "Transform.glsl"

#ifdef COMPILEVS
attribute uint                      misc_in;    //lidar data, normals
#endif

#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
	
	gl_PointSize  = 10.0;
	
    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif

}

void PS()
{
    #ifdef VERTEXCOLOR
        gl_FragColor = vColor;
	#else
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif
}
