#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "PostProcess.glsl"

varying vec2 vScreenPos;
varying vec2 vScreenSize;

#ifdef COMPILEPS
uniform float		cPix_scale;		    //	(relative) pixel scale in image
uniform vec2		cNeigh_pos_2D[8] = vec2[8](vec2(0., 1.), vec2(1., 0.), vec2(0., -1.), vec2(-1., 0.), vec2(1., 1.),
 vec2(1., -1.), vec2(-1., 1.), vec2(-1., -1.));	//	array of neighbors (2D positions)
uniform float		cExp_scale;			//	exponential scale factor (for computed AO)

//uniform float		Zm;					//	minimal depth in image
//uniform float		ZM;					//	maximal depth in image

//uniform float		Sx;
//uniform float		Sy;

//uniform float		Zoom;				// image display zoom (so as to always use - approximately - the same pixels)
//uniform int			PerspectiveMode;	// whether perspective mode is enabled (1) or not (0) - for z-Buffer compensation

uniform vec3		cLight_dir;

/**************************************************/


//  Obscurance (pseudo angle version)
//	z		neighbour relative elevation
//	dist	distance to the neighbourx
float obscurance(float z, float dist)
{
	return max(0.0, z) / dist;
}

float ztransform(vec2 uv)
{
	float z_b = texture2D(sDepthBuffer, uv).r;
	if (cDepthReconstruct.w == 1)
	{
		//'1/z' depth-buffer transformation correction
		float z_n = 2.0 * z_b - 1.0;
		z_b = 2.0 * cNearClipPS / (cFarClipPS + cNearClipPS - z_n * (cFarClipPS - cNearClipPS));
		z_b = z_b * cFarClipPS / (cFarClipPS - cNearClipPS);
	}

	return clamp(1.0 - z_b, 0.0, 1.0);
}

float fetch_depth(vec2 uv)
{
    vec4 depthInput = texture2D(sDepthBuffer, uv);
    #ifdef HWDEPTH
        float depth = ReconstructDepth(depthInput.r);
		//float depth = 2. * cNearClipPS / (cFarClipPS + cNearClipPS - depthInput.r * ( cFarClipPS - cNearClipPS ));
	#else
       float depth = DecodeDepth(depthInput.rgb);
    #endif
    return depth;
}

float computeObscurance(float depth, float scale)
{
	// Light-plane point
	vec4 P = vec4( cLight_dir.xyz , -dot(cLight_dir.xyz,vec3(0.0,0.0,depth)) );
	float		Zoom = (cDepthReconstruct.w == 1) ? 3.0 : 1.7;
	float sum = 0.0;

	// contribution of each neighbor
	for(int c=0; c<8; c++)
	{
		vec2 N_rel_pos = scale * Zoom / vec2(vScreenSize.x,vScreenSize.y) * cNeigh_pos_2D[c];	//neighbor relative position
		vec2 N_abs_pos = vScreenSize + N_rel_pos;					//neighbor absolute position
		
		//version with background shading
		float Zn = ztransform(N_abs_pos);		//depth of the real neighbor
		float Znp = dot( vec4(N_rel_pos, Zn, 1.0) , P );				//depth of the in-plane neighbor

		sum += obscurance( Znp, scale );
	}

	return	sum;
}
#endif //COMPILEPS

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
	vScreenSize = cGBufferOffsets.zw;
}

void PS()
{
    //ambient occlusion
    vec3 rgb = texture2D(sDiffMap, vScreenPos).rgb;
	float depth = ztransform(vScreenPos);

	if( depth > 0.01 )
	{
		float f = computeObscurance(depth, cPix_scale);
		f = exp(-cExp_scale*f);

		gl_FragData[0]	=	vec4(f*rgb, 1.0);
	}
	else
	{
		gl_FragData[0]	=	vec4(rgb, 1.0);
	}
}

