#pragma once

namespace Urho3D
{
	class Camera;
	class Zone;
	class Color;
	class WeatherEffectUtil
	{
	public:
		/// 下雨效果
		static void SetRainEffect(Camera* cam);
		static void CancelRainEffect(Camera* cam);
		/// 下雪效果
		static void SetSnowEffect(Camera* cam);
		static void CancelSnowEffect(Camera* cam);
		/// 起雾效果
		static void SetFogEffect(Zone* zone, float startFog, float endFog, Color fogColor);
		static void CancelFogEffect(Zone* zone);
	};

}