#include "Uniforms.glsl"
#include "Transform.glsl"
#include "Samplers.glsl"

#define MAX_POINT_SIZE              16

#ifdef COMPILEVS
	uniform float cPointSize;
	uniform float cPointScalePersp;
	uniform float cPointScaleOrtho;

	#ifdef FALSECOLOR
		uniform float cHeightMax;
		uniform float cHeightMin;
	#endif
	
	#ifdef HASNORMAL
		uniform sampler2D       normalLookUpTable;
	#endif

#endif

    varying vec4 vColor;
#ifdef HASNORMAL
	varying vec3 vNormal;
#endif
	
void VS()
{
    vec3 worldPos = (vec4(iPos.xyz, 1.0) * iModelMatrix).xyz;
    gl_Position = GetClipPos(worldPos);
	
	#ifdef ADAPTIVESIZE
		vec4 camPos = vec4(worldPos, 1.0) * cView;
		float camDist = length( camPos.xyz / camPos.w );
		float pointSizeLocal = cPointScalePersp / camDist + cPointScaleOrtho;
		if( pointSizeLocal > MAX_POINT_SIZE )
			pointSizeLocal = MAX_POINT_SIZE;
		gl_PointSize  = pointSizeLocal * cPointSize;
	#else
		gl_PointSize  = cPointSize;
	#endif
	
    #ifdef VERTEXCOLOR
        vColor = iColor;
	#elif defined(DEPTHTARGET)
		vColor.x = GetDepth(gl_Position);
	#elif defined(FALSECOLOR)
		float a = (iPos.z - cHeightMin) / (cHeightMax - cHeightMin);
		if(a < 0.333)
			vColor = vec4(1.0 * a * 3, 1.0, 1.0, 1.0);
		else if(a < 0.666)
			vColor = vec4(1.0, 1.0 * (a-0.333) * 3, 1.0, 1.0);
		else
			vColor = vec4(1.0, 1.0, 1.0 * (a-0.666) * 3, 1.0);
	#else
		vColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif
	
	#ifdef HASNORMAL
		uint misc_in = uint(iObjectIndex);
		float sampleIndex = float( ( misc_in & 0x3FFFu  ));
		float xCoord =  mod( float(sampleIndex), 128.0) /128.0 ;
		float yCoord =  floor(sampleIndex / 128.0 ) / 128.0;
		vNormal  = texture2D( normalLookUpTable,  vec2( xCoord, yCoord)).xyz;
	#endif
}

void PS()
{
	#ifdef DEPTHTARGET
		gl_FragColor = vec4(EncodeDepth(vColor.x), 1.0);
	#else
		gl_FragColor = vColor;
	#endif
}
