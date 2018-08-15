#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "ScreenPos.glsl"
#include "Lighting.glsl"
#include "MathUtil.glsl"
#line 40007

varying vec2 vScreenPos;
varying vec3 frustumSize;

#ifdef COMPILEPS
uniform mat4 cLastVPMatrix;
uniform float cTemporlAAFeedbackMin;
uniform float cTemporlAAFeedbackMax;
uniform vec2 cTemporlAAJitterUV;
#endif

#ifdef COMPILEPS
//----------------------------------------------------------------------------------
#ifdef GL_ES
	const float FLT_EPS = 0.0001;
#else
	const float FLT_EPS = 0.00000001;
#endif

#define ZCMP_GT(a, b) (a > b)

vec3 find_closest_fragment_3x3(vec2 uv)
{
	vec2 dd = abs(cGBufferInvSize.xy);
	vec2 du = vec2(dd.x, 0.0);
	vec2 dv = vec2(0.0, dd.y);

	vec3 dtl = vec3(-1., -1., texture2D(sDepthBuffer, uv - dv - du).x);
	vec3 dtc = vec3( 0., -1., texture2D(sDepthBuffer, uv - dv).x);
	vec3 dtr = vec3( 1., -1., texture2D(sDepthBuffer, uv - dv + du).x);

	vec3 dml = vec3(-1., 0., texture2D(sDepthBuffer, uv - du).x);
	vec3 dmc = vec3( 0., 0., texture2D(sDepthBuffer, uv).x);
	vec3 dmr = vec3( 1., 0., texture2D(sDepthBuffer, uv + du).x);

	vec3 dbl = vec3(-1., 1., texture2D(sDepthBuffer, uv + dv - du).x);
	vec3 dbc = vec3( 0., 1., texture2D(sDepthBuffer, uv + dv).x);
	vec3 dbr = vec3( 1., 1., texture2D(sDepthBuffer, uv + dv + du).x);

	vec3 dmin = dtl;
	if (ZCMP_GT(dmin.z, dtc.z)) dmin = dtc;
	if (ZCMP_GT(dmin.z, dtr.z)) dmin = dtr;

	if (ZCMP_GT(dmin.z, dml.z)) dmin = dml;
	if (ZCMP_GT(dmin.z, dmc.z)) dmin = dmc;
	if (ZCMP_GT(dmin.z, dmr.z)) dmin = dmr;

	if (ZCMP_GT(dmin.z, dbl.z)) dmin = dbl;
	if (ZCMP_GT(dmin.z, dbc.z)) dmin = dbc;
	if (ZCMP_GT(dmin.z, dbr.z)) dmin = dbr;

	return vec3(uv + dd.xy * dmin.xy, dmin.z);
}

// https://software.intel.com/en-us/node/503873
vec3 RGB_YCoCg(vec3 c)
{
	// Y = R/4 + G/2 + B/4
	// Co = R/2 - B/2
	// Cg = -R/4 + G/2 - B/4
	return vec3(
		 c.x/4.0 + c.y/2.0 + c.z/4.0,
		 c.x/2.0 - c.z/2.0,
		-c.x/4.0 + c.y/2.0 - c.z/4.0
	);
}

// https://software.intel.com/en-us/node/503873
vec3 YCoCg_RGB(vec3 c)
{
	// R = Y + Co - Cg
	// G = Y + Cg
	// B = Y - Co - Cg
	return clamp(vec3(
		c.x + c.y - c.z,
		c.x + c.z,
		c.x - c.y - c.z
	), 0., 1.);
}
	
vec4 sample_color(sampler2D tex, vec2 uv)
{
#ifdef USE_YCOCG
	vec4 c = texture2D(tex, uv);
	return vec4(RGB_YCoCg(c.rgb), c.a);
#else
	return texture2D(tex, uv);
#endif
}

vec4 resolve_color(vec4 c)
{
#ifdef USE_YCOCG
	return vec4(YCoCg_RGB(c.rgb).rgb, c.a);
#else
	return c;
#endif
}

vec4 clip_aabb(vec3 aabb_min, vec3 aabb_max, vec4 p, vec4 q)
{
#ifdef USE_OPTIMIZATIONS
	// note: only clips towards aabb center (but fast!)
	vec3 p_clip = 0.5 * (aabb_max + aabb_min);
	vec3 e_clip = 0.5 * (aabb_max - aabb_min) + FLT_EPS;

	vec4 v_clip = q - vec4(p_clip, p.w);
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

	if (ma_unit > 1.0)
		return vec4(p_clip, p.w) + v_clip / ma_unit;
	else
		return q;// point inside aabb
#else
	vec4 r = q - p;
	vec3 rmax = aabb_max - p.xyz;
	vec3 rmin = aabb_min - p.xyz;
 
	const float eps = FLT_EPS;
	if (r.x > rmax.x + eps)
		r *= (rmax.x / r.x);
	if (r.y > rmax.y + eps)
		r *= (rmax.y / r.y);
	if (r.z > rmax.z + eps)
		r *= (rmax.z / r.z);

	if (r.x < rmin.x - eps)
		r *= (rmin.x / r.x);
	if (r.y < rmin.y - eps)
		r *= (rmin.y / r.y);
	if (r.z < rmin.z - eps)
		r *= (rmin.z / r.z);

	return p + r;

#endif
}

vec4 temporal_reprojection(vec2 ss_txc, vec2 ss_vel, float vs_dist)
{
	// read texels
	vec4 texel0 = sample_color(sSpecMap, ss_txc);

	vec4 texel1 = sample_color(sDiffMap, ss_txc - ss_vel);

	// calc min-max of current neighbourhood
	vec2 uv = ss_txc;

#if defined(MINMAX_3X3) || defined(MINMAX_3X3_ROUNDED)

	vec2 du = vec2(cGBufferInvSize.x, 0.0);
	vec2 dv = vec2(0.0, cGBufferInvSize.y);

	vec4 ctl = sample_color(sSpecMap, uv - dv - du);
	vec4 ctc = sample_color(sSpecMap, uv - dv);
	vec4 ctr = sample_color(sSpecMap, uv - dv + du);
	vec4 cml = sample_color(sSpecMap, uv - du);
	vec4 cmc = sample_color(sSpecMap, uv);
	vec4 cmr = sample_color(sSpecMap, uv + du);
	vec4 cbl = sample_color(sSpecMap, uv + dv - du);
	vec4 cbc = sample_color(sSpecMap, uv + dv);
	vec4 cbr = sample_color(sSpecMap, uv + dv + du);

	vec4 cmin = min(ctl, min(ctc, min(ctr, min(cml, min(cmc, min(cmr, min(cbl, min(cbc, cbr))))))));
	vec4 cmax = max(ctl, max(ctc, max(ctr, max(cml, max(cmc, max(cmr, max(cbl, max(cbc, cbr))))))));

	#if defined(MINMAX_3X3_ROUNDED) || defined(USE_YCOCG) || defined(USE_CLIPPING)
		vec4 cavg = (ctl + ctc + ctr + cml + cmc + cmr + cbl + cbc + cbr) / 9.0;
	#endif

	#ifdef MINMAX_3X3_ROUNDED
		vec4 cmin5 = min(ctc, min(cml, min(cmc, min(cmr, cbc))));
		vec4 cmax5 = max(ctc, max(cml, max(cmc, max(cmr, cbc))));
		vec4 cavg5 = (ctc + cml + cmc + cmr + cbc) / 5.0;
		cmin = 0.5 * (cmin + cmin5);
		cmax = 0.5 * (cmax + cmax5);
		cavg = 0.5 * (cavg + cavg5);
	#endif

#elif MINMAX_4TAP_VARYING// this is the method used in v2 (PDTemporalReprojection2)

	const float _SubpixelThreshold = 0.5;
	const float _GatherBase = 0.5;
	const float _GatherSubpixelMotion = 0.1666;

	vec2 texel_vel = ss_vel / cGBufferInvSize.xy;
	float texel_vel_mag = length(texel_vel) * vs_dist;
	float k_subpixel_motion = clamp(_SubpixelThreshold / (FLT_EPS + texel_vel_mag), 0., 1.);
	float k_min_max_support = _GatherBase + _GatherSubpixelMotion * k_subpixel_motion;

	vec2 ss_offset01 = k_min_max_support * vec2(-cGBufferInvSize.x, cGBufferInvSize.y);
	vec2 ss_offset11 = k_min_max_support * vec2(cGBufferInvSize.x, cGBufferInvSize.y);
	vec4 c00 = sample_color(sSpecMap, uv - ss_offset11);
	vec4 c10 = sample_color(sSpecMap, uv - ss_offset01);
	vec4 c01 = sample_color(sSpecMap, uv + ss_offset01);
	vec4 c11 = sample_color(sSpecMap, uv + ss_offset11);

	vec4 cmin = min(c00, min(c10, min(c01, c11)));
	vec4 cmax = max(c00, max(c10, max(c01, c11)));

	#if defined(USE_YCOCG) || defined(USE_CLIPPING)
		vec4 cavg = (c00 + c10 + c01 + c11) / 4.0;
	#endif

#endif

	// shrink chroma min-max
#ifdef USE_YCOCG
	vec2 chroma_extent = vec2(0.25 * 0.5 * (cmax.r - cmin.r));
	vec2 chroma_center = texel0.gb;
	cmin.yz = chroma_center - chroma_extent;
	cmax.yz = chroma_center + chroma_extent;
	cavg.yz = chroma_center;
#endif

	// clamp to neighbourhood of current sample
#ifdef USE_CLIPPING
	texel1 = clip_aabb(cmin.xyz, cmax.xyz, clamp(cavg, cmin, cmax), texel1);
#else
	texel1 = clamp(texel1, cmin, cmax);
#endif

	// feedback weight from unbiased luminance diff (t.lottes)
#ifdef USE_YCOCG
	float lum0 = texel0.r;
	float lum1 = texel1.r;
#else
	float lum0 = GetIntensity(texel0.rgb);
	float lum1 = GetIntensity(texel1.rgb);
#endif
	float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.2));
	float unbiased_weight = 1.0 - unbiased_diff;
	float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
	float k_feedback = mix(cTemporlAAFeedbackMin, cTemporlAAFeedbackMax, unbiased_weight_sqr) * clamp( 1. - length(ss_vel)*100., 0., 1.);

	// output
	return mix(texel0, texel1, k_feedback);
}

vec4 sample_color_motion(sampler2D tex, vec2 uv, vec2 ss_vel)
{
	vec2 v = 0.5 * ss_vel;
	const int taps = 3;// on either side!

	float srand = PDsrand(uv + vec2(sin(cElapsedTimePS/8.)));

	vec2 vtap = v / taps;
	vec2 pos0 = uv + vtap * (0.5 * srand);
	vec4 accu = vec4(0.0);
	float wsum = 0.0;
	
	for (int i = -taps; i <= taps; i++)
	{
	
		float w = 1.0;// box
		//float w = taps - abs(i) + 1;// triangle
		//float w = 1.0 / (1.0 + float(abs(i)));// pointy triangle
		accu += w * sample_color(tex, pos0 + i * vtap);
		wsum += w;
	}
	return accu / wsum;
}
	
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    frustumSize = cFrustumSize;
	vScreenPos = GetScreenPosPreDiv(gl_Position);
}


void PS()
{
	vec2 uv = vScreenPos;
	
#ifdef USE_DILATION
	vec3 c_frag = find_closest_fragment_3x3(uv);
	vec2 ss_vel = texture2D(sNormalMap, c_frag.xy).xy;
	vec4 depthInput = texture2D(sDepthBuffer, uv);
	#ifdef HWDEPTH
        float vs_dist = ReconstructDepth(c_frag.z);
	#else
       float vs_dist = DecodeDepth(depthInput.rgb);
    #endif
#else
	vec2 ss_vel = texture2D(sNormalMap, uv).xy;
	vec4 depthInput = texture2D(sDepthBuffer, uv);
	#ifdef HWDEPTH
        float vs_dist = ReconstructDepth(depthInput.r);
	#else
       float vs_dist = DecodeDepth(depthInput.rgb);
    #endif
#endif

	// temporal resolve
	vec4 color_temporal = temporal_reprojection(uv, ss_vel, vs_dist);
	
#ifdef USE_MOTION_BLUR
	const float _MotionScale = 1.;
	ss_vel = _MotionScale * ss_vel;
	
	float vel_mag = length(ss_vel / cGBufferInvSize);
	const float vel_trust_full = 2.0;
	const float vel_trust_none = 15.0;
	const float vel_trust_span = vel_trust_none - vel_trust_full;
	float trust = 1.0 - clamp(vel_mag - vel_trust_full, 0.0, vel_trust_span) / vel_trust_span;
	
	vec4 color_motion = sample_color_motion(sSpecMap, vScreenPos, ss_vel);
	
	vec4 to_buffer = resolve_color(mix(color_motion, color_temporal, trust));
#else
	// prepare outputs
	vec4 to_buffer = resolve_color(color_temporal);
	
#endif
	
	// add noise
	vec4 noise4 = PDsrand4(vScreenPos + sin(cElapsedTimePS/8.) + 0.6959174) / 510.0;
	gl_FragColor = clamp(to_buffer + noise4, 0., 1.);
}
