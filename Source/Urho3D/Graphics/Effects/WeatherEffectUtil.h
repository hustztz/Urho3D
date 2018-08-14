#pragma once

namespace Urho3D
{
	class Camera;
	class Zone;
	class Color;
	class WeatherEffectUtil
	{
	public:
		/// ����Ч��
		static void SetRainEffect(Camera* cam);
		static void CancelRainEffect(Camera* cam);
		/// ��ѩЧ��
		static void SetSnowEffect(Camera* cam);
		static void CancelSnowEffect(Camera* cam);
		/// ����Ч��
		static void SetFogEffect(Zone* zone, float startFog, float endFog, Color fogColor);
		static void CancelFogEffect(Zone* zone);
	};

}