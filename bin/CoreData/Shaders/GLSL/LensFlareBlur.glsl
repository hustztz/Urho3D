#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vTexCoord;
varying vec3 vOutColor;

#ifdef COMPILEVS
	uniform vec4 cTileCountAndSize;
	uniform vec4 cColorScaleValue;
	uniform vec4 cKernelSizeValue;
#endif

#ifdef COMPILEPS

#endif
#line 10000
void VS()
{
	ivec2 tileCount = ivec2(cTileCountAndSize.xy);
	ivec2 tileSize = ivec2(cTileCountAndSize.zw);
	
	// needs to be the same on shader side
	int QuadsPerInstance = 4;
	
	//uint iid = vec2(mod(gl_VertexID, gl_InstanceID
	int iid = gl_InstanceID*QuadsPerInstance + (gl_VertexID / 6);
	int vid = int(mod(gl_VertexID, 6));
	float localPosY;
	if(vid > 1 && vid < 5)
		localPosY = 1.;
	else
		localPosY = 0.;
	
	vec2 localPos = vec2(mod(vid, 2), localPosY);
	vec2 tilePos = vec2(mod(iid, float(tileCount.x)), float(iid / tileCount.x));
	
	gl_Position = vec4(0., 0., 0., 1.);
	vTexCoord = localPos.xy;
	
	gl_Position.xy = tilePos/vec2(tileCount);
	
	vOutColor = texture2D(sDiffMap, gl_Position.xy).rgb;
	
	//vOutColor *= DiscMask(gl_Position.xy);
		
	gl_Position.xy = gl_Position.xy * 2. - 1.;
	
	float luminaceVal = dot(vOutColor, vec3(1.));
	
	float threshold = cColorScaleValue.y;
	
	vec2 thisKernelSize = cKernelSizeValue.xy;
	if(luminaceVal < threshold)
	{
		thisKernelSize = vec2(0.);
	}
	
	vOutColor *= cColorScaleValue.x * 9.6;
	
	gl_Position.xy += 2. * (localPos - 0.5) * thisKernelSize / vec2(tileCount);
}

void PS()
{
	float kernel = texture2D(sSpecMap, vTexCoord).a;
	gl_FragColor = vec4(vOutColor*kernel, 1.);
}
