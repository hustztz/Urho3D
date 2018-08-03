#include "Uniforms.glsl"
#include "Samplers.glsl"
#include "Transform.glsl"
#include "PostProcess.glsl"

varying vec3 vTexCoord;

#ifdef COMPILEPS

uniform float cRatio;
uniform float cCloudsMoveSpeed;
uniform float cCloudsChangeSpeed;
uniform vec3 cSunDir;
uniform vec3 cSunSpeedDir;
uniform float cHour;
uniform vec3 cIndirectColor;
	
//stamophere
const float pi = 3.14159265359;
const float zenithOffset = 0.0;
const float multiScatterPhase = 0.1;
const float density = 0.7;
//const vec3 skyColor = vec3(0.39, 0.57, 1.0); //Make sure one of the conponents is never 0.0
const vec3 skyColor = vec3(0.20, 0.37, 1.0); //Make sure one of the conponents is never 0.0

//cloud
const float timeScale = 20.0;
const float cloudScale = 0.03;
const float skyCover = 0.6; //overwritten by mouse x drag
const float softness = 0.2;
const float brightness = 1.0;
const int noiseOctaves = 8;
const float curlStrain = 3.0;

float greatCircleDist(vec2 p, vec2 lp)
{
    float phi_1 = p.y;
    float phi_2 = lp.y;
    float delta_lambda = p.x-lp.x;
	//float delta = 0.001;
	//if( dot(vec2(phi_1-phi_2, delta_lambda), vec2(phi_1-phi_2, delta_lambda)) <= delta*delta)
	//	return delta;
	return acos(sin(phi_1)*sin(phi_2) + cos(phi_1)*cos(phi_2)*cos(delta_lambda));
}

float  zenithDensity(float x)
{
    
    return density / pow(max(x - zenithOffset, 0.35e-2), 0.75);
}

vec3 getSkyAbsorption(vec3 x, float y){
	
	vec3 absorption = x * -y;
	absorption = exp2(absorption) * 2.0;
	
	return absorption;
}

float getSunPoint(vec2 p, vec2 lp){
    float dist = greatCircleDist(p, lp)/pi*2.;
	return (1.-smoothstep(0.026, 0.03, dist)) * 50.0;
}
const float moonRadius = 0.05;
vec2 rotate_direction(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y, 
                  Dir.x*CosSin.y + Dir.y*CosSin.x);
}
vec3 getMoonPoint(vec2 lp, vec3 p3, vec3 lp3)
{
	if(cHour >= 6. && cHour <= 18.)
		return vec3(0.);
	if(p3.y < 0)
		return vec3(0.);
	float dist = length(p3-lp3);
	if(dist < moonRadius)
	{
		vec3 p2lp = (p3-lp3);
		vec3 pTangent = normalize(cSunSpeedDir);
		//if(cross(pTangent, lp3).y < 0)
		//	pTangent *= -1.;
		vec3 p2lpVTangent = normalize(p2lp - pTangent*dot(p2lp,pTangent));
		if(dot(cross(lp3, pTangent),p2lpVTangent) < 0)
			p2lpVTangent *= -1.;
		float sin = dot(p2lp,pTangent)/ dist;
		float cos = dot(p2lp,p2lpVTangent)/dist;
		vec2 cossin = vec2(cos, sin);
		vec2 texCoord = cossin*dist/moonRadius * 0.5 + 0.5;
		return texture2D(sNormalMap, texCoord).rgb * 0.5;
	}
	return vec3(0.);
}

vec3 getStarts(vec2 uv, vec3 lp)
{
	if(cHour >= 6. && cHour <= 18.)
		return vec3(0.);
	if(lp.y < 0)
		return vec3(0.);
	return texture2D(sSpecMap, uv).rgb*1.4;
}


float getRayleigMultiplier(vec2 p, vec2 lp)
{
    float dist = greatCircleDist(p, lp)/pi*5.;
	return 1.0 + pow(1.0 - clamp(dist, 0.0, 1.0), 2.0) * pi * 0.5;
}

float getMie(vec2 p, vec2 lp){
    float dist = greatCircleDist(p, lp)/pi*2.;
	float disk = clamp(1.0 - pow(dist, 0.1), 0.0, 1.0);
	
	return disk*disk*(3.0 - 2.0 * disk) * 2.0 * pi;
}

vec3 getAtmosphericScattering(vec2 p, vec2 lp)
{
    
	float zenith = zenithDensity(p.y);
	float sunPointDistMult =  clamp(length(max(lp.y + multiScatterPhase - zenithOffset, 0.0)), 0.0, 1.0);
	
	float rayleighMult = getRayleigMultiplier(p, lp);
	vec3 skyColorCopy = skyColor;
	if(cHour < 6. || cHour > 18.)
	{
		float attention = mix(1., 1./4., clamp((6.-cHour)/0.1, 0., 1.)+clamp((cHour-18.)/0.1, 0., 1.));
		skyColorCopy *= attention;
	}
	vec3 absorption = getSkyAbsorption(skyColorCopy, zenith);
	vec3 sky = vec3(0.);
	
	vec3 totalSky = vec3(0.);
    if(cHour < 6. || cHour > 18.)
	{
		sky = skyColorCopy;
		totalSky = mix(sky * absorption, sky / (sky + 0.5), sunPointDistMult);
	}
	else{
		sky = skyColorCopy * zenith * rayleighMult;
		totalSky = mix(sky * absorption, sky / (sky + 0.5), sunPointDistMult);
		vec3 sunAbsorption = getSkyAbsorption(skyColorCopy, zenithDensity(lp.y + multiScatterPhase));
		vec3 sun = getSunPoint(p, lp) * absorption;
		vec3 mie = getMie(p, lp) * sunAbsorption;
    	totalSky += (sun + mie)*0.06;
		totalSky *= sunAbsorption * 0.5 + 0.5 * length(sunAbsorption);
	}
	totalSky *= 0.5;
	return totalSky;
}

vec3 jodieReinhardTonemap(vec3 c)
{
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    vec3 tc = c / (c + 1.0);

    return mix(c / (l + 1.0), tc, tc);
}

vec2 toSkyPosition(vec3 pos)
{

    //pos = (pos.xy / iResolution.xy - .5)*vec2(iResolution.x/iResolution.y, 1.);
    //return vec2(atan(pos.y, pos.x), (.5-length(pos))*pi);
	return vec2(atan(-pos.z, pos.x), atan(pos.y/sqrt(pos.z*pos.z+pos.x*pos.x)));
}

float saturate(float num) {
	return clamp(num, 0.0, 1.0);
}

float noise(vec2 uv) {
	return texture2D(sDiffMap, uv).r;
}

vec2 rotate(vec2 uv) {
	uv = uv + noise(uv * 0.2) * 0.005;
	float rot = curlStrain;
	float sinRot = sin(rot);
	float cosRot = cos(rot);
	mat2 rotMat = mat2(cosRot, -sinRot, sinRot, cosRot);
	return uv * rotMat;
}

float fbm(vec2 uv) {
	float rot = 1.57;
	float sinRot = sin(rot);
	float cosRot = cos(rot);
	float f = 0.0;
	float total = 0.0;
	float mul = 0.5;
	mat2 rotMat = mat2(cosRot, -sinRot, sinRot, cosRot);

	for (int i = 0; i < noiseOctaves; i++) {
		f += noise(uv + cElapsedTimePS * cCloudsChangeSpeed * 0.00015 * timeScale * (1.0 - mul)) * mul;
		total += mul;
		uv *= 3.0;
		uv = rotate(uv);
		mul *= 0.5;
	}
	return f / total;
}

vec4 getCloudColor(vec2 texCoord, float y, vec3 sunCol)
{
    vec2 uv = texCoord / cGBufferInvSize / (250000.0 * cloudScale);

	float cover = -cRatio*0.6+0.6;

	float bright = brightness * (0.1 + cRatio);

	float color1 = fbm(uv - 0.5 + cElapsedTimePS * cCloudsMoveSpeed * 0.00004 * timeScale);
	float color2 = fbm(uv - 10.5 + cElapsedTimePS * cCloudsMoveSpeed * 0.00002 * timeScale);

	float clouds1 = smoothstep( cover, min(( cover ) + softness * 5.0, 1.0), color1);
	float clouds2 = smoothstep( cover, min(( cover ) + softness * 2.0 , 1.0), color2);

	float cloudsFormComb = clamp((clouds1+clouds2)/2.*bright, 0., 1.) * smoothstep(0., 0.1, y);
	
	float bAttenuation = 0.4 * (1.1 - cRatio) * ( 1.4 - cloudsFormComb);
	vec3 cloudColor = clamp( vec3(sunCol.rgb * bAttenuation), 0., 10.);
	
	vec3 monochromeCloudColor = vec3(max(max(cloudColor.r,cloudColor.b),cloudColor.g));
	cloudColor = mix( cloudColor, monochromeCloudColor, vec3(cRatio));
	
	return vec4(cloudColor, cloudsFormComb + cRatio * 0.1);
}
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);
    gl_Position.z = gl_Position.w;
    vTexCoord = iPos.xyz;
}

void PS()
{
   // vec4 sky = cMatDiffColor * textureCube(sDiffCubeMap, vTexCoord);
   // #ifdef HDRSCALE
   //     sky = vec4(20.,20.,20.,1.);
   // #endif
  //  gl_FragColor = sky;
	vec2 position = toSkyPosition(vTexCoord);
	vec2 lightPosition = toSkyPosition(cSunDir);
	vec3 atmosphereColor = getAtmosphericScattering(position, lightPosition) * pi;
	vec3 moonColor= getMoonPoint(lightPosition, normalize(vTexCoord), normalize(cSunDir));
	atmosphereColor += moonColor;
	vec3 dir = normalize(vTexCoord);
	vec2 dirSkyPos = toSkyPosition(dir);
	vec2 dirProj = dir.xz/pow(0.2 + (dot(vec3(0.,1.,0.),dir)), 0.4);
	dirProj = dirProj*0.5 + 0.5;
	//vec2 dirProj;
	//float cos1 = cos(dirSkyPos.x);
	//float sin1 = sin(dirSkyPos.x);
	//float cos2 = cos(dirSkyPos.y);
	//float sin2 = sin(dirSkyPos.y);
	
	//dirProj.x = acos(cos2*cos1/sqrt(1.-cos2*cos2*sin1*sin1))*sqrt(1.-cos2*cos2*sin1*sin1);
	//dirProj.y = acos(-cos2*sin1/sqrt(1.-cos2*cos2*cos1*cos1))*sqrt(1.-cos2*cos2*cos1*cos1);
	//dirProj.x = acos(cos2*cos1);
	//dirProj.y = asin(-cos2*sin1);
	if(moonColor.r <= 0.)
		atmosphereColor += getStarts(dirProj, dir);
		
#ifndef HDRSCALE
	atmosphereColor = jodieReinhardTonemap(atmosphereColor);
    atmosphereColor = pow(atmosphereColor, vec3(2.2)); //Back to linear
	//float lum = 0.6;
    //gl_FragColor.rgb = Uncharted2Tonemap(gl_FragColor.rgb) / Uncharted2Tonemap(vec3(lum*5.));
#endif
	vec3 sunColor = cIndirectColor;
	vec4 cloudColor = getCloudColor(dirProj, dir.y, sunColor);
#ifndef HDRSCALE
	cloudColor.rgb = jodieReinhardTonemap(cloudColor.rgb * 10.);
    cloudColor.rgb = pow(cloudColor.rgb, vec3(2.2)); //Back to linear
	//float lum = 0.6;
    //gl_FragColor.rgb = Uncharted2Tonemap(gl_FragColor.rgb) / Uncharted2Tonemap(vec3(lum*5.));
#endif	
	gl_FragColor.rgb = mix(atmosphereColor, cloudColor.rgb, clamp(cloudColor.a, 0., 1.));	

}
