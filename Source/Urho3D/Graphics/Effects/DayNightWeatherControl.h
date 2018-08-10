#pragma once
#include "../../scene/LogicComponent.h"

namespace Urho3D
{
	class Zone;
	class DayNightWeatherControl : public LogicComponent
	{
		URHO3D_OBJECT(DayNightWeatherControl, LogicComponent);
	public:
		explicit DayNightWeatherControl(Context* context);
		/// Handle scene update. Called by LogicComponent base class.
		void Update(float timeStep) override;
		static void RegisterObject(Context* context);
		void SetDynamic(bool enable) { isDynamic_ = enable; }
		bool IsDynamic() const { return isDynamic_; }
		void SetSystemTiming(bool enable) { isSystemTiming_ = enable; }
		bool IsSystemTiming() const { return isSystemTiming_; }
		void SetSpeed(float speed) { speed_ = speed; }
		void SetHour(float hour) { hour_ = hour; }
		float GetHour() const { return hour_; }
		void SetWeatherRatio(float ratio) { weatherRatio_ = ratio; }
		void EnableHDR(bool enable);
		bool GetEnableHDR() const { return isHDR_;}
		Zone* GetZone();
		Vector3 GetIndrectColor(){ return indirectColor_; }
	private:
		virtual void Start() override;
		virtual void DelayedStart() override;
		void UpdateSun();
		void UpdateSkyBox();
		void UpdateZone();
		void UpdateTime(float timeStep);
		void GetSunDirection(float hour, Vector3& sunDir, Vector3& sunSpeedDir);
		bool SolveQuadratic(float a, float b, float c, float& x1, float& x2);
		void Swap(float& t0, float& t1);
		bool RaySphereIntersect(Vector3 orig, Vector3 dir, float radius, float& t0, float& t1);
		Vector3 ComputeIncidentLight(Vector3 orig, Vector3 sunDirection/* , float tmin, float tmax */);
		Vector3 ComputeAmbientLight(Vector3 orig, Vector3 dir, Vector3 sunDirection/* , float tmin, float tmax */);
		WeakPtr<Node> sunNode_{};
		WeakPtr<Node> skyBoxNode_{};
		WeakPtr<Node> zoneNode_{};
		float hour_;
		float sunSpeed_;
		Vector3 sunDir_;
		Vector3 lastSunDir_;
		Vector3 sunSpeedDir_;
		float yRotation_;
		float atmosphereRadius; // In the paper this is usually R or Ra (radius atmosphere)
		float earthRadius;
		float Hr;
		float Hm;
		Vector3 betaR;
		Vector3 betaM;
		float dayValue_;
		float nightValue_;
		Vector3 indirectColor_;
		bool isSystemTiming_;
		bool isDynamic_;
		float speed_;
		float weatherRatio_;
		bool isHDR_;
	};
}
