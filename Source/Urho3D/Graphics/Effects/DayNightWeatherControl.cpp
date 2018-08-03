#include "Graphics/Effects/DayNightWeatherControl.h"
#include "IO/Log.h"
#include "Graphics/Light.h"
#include "Scene/Scene.h"
#include "Graphics/Skybox.h"
#include "Resource/ResourceCache.h"
#include "Graphics/Material.h"
#include "Graphics/Model.h"
#include "Core/Context.h"
#include "Graphics/Renderer.h"
#include "Graphics/Zone.h"
#include <chrono>
#include <ctime>
#include <iomanip>

namespace Urho3D
{
	extern const char* GEOMETRY_CATEGORY;

	DayNightWeatherControl::DayNightWeatherControl(Context* context) :
		LogicComponent(context),
		hour_(9.f),
		yRotation_(0.5f),
		lastSunDir_(0,1,0),
		atmosphereRadius(6420e3),
		earthRadius(6360e3),
		Hr(7994),
		Hm(1200),
		///betaR(3.8e-6f, 13.5e-6f, 33.1e-6f),
		betaR(3.8e-6f*3.1f, 13.5e-6f*1.7f, 33.1e-6f),
		betaM(21e-6f, 21e-6f, 21e-6f),
		dayValue_(30.f),
		nightValue_(3.f),
		indirectColor_(1., 1., 1.),
		isDynamic_(true),
		isSystemTiming_(true),
		speed_(1.f),
		weatherRatio_(0.5),
		isHDR_(false)
	{
		// Only the scene update event is needed: unsubscribe from the rest for optimization
		SetUpdateEventMask(USE_UPDATE);
	}
	void DayNightWeatherControl::Update(float timeStep)
	{
		UpdateTime(timeStep);
		GetSunDirection(hour_, sunDir_, sunSpeedDir_);
		UpdateSun();
		UpdateSkyBox();
		UpdateZone();
	}
	void DayNightWeatherControl::RegisterObject(Context* context)
	{
		context->RegisterFactory<DayNightWeatherControl>(GEOMETRY_CATEGORY);
		URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
		URHO3D_ATTRIBUTE("Hour", float, hour_, 9.f, AM_DEFAULT);
		URHO3D_ATTRIBUTE("DayBrightness", float, dayValue_, 30.f, AM_DEFAULT);
		URHO3D_ATTRIBUTE("NightBrightness", float, nightValue_, 3.f, AM_DEFAULT);
		
		URHO3D_ATTRIBUTE("Dynamic", bool, isDynamic_, true, AM_DEFAULT);
		URHO3D_ATTRIBUTE("UseSystemTime", bool, isSystemTiming_, true, AM_DEFAULT);
		URHO3D_ATTRIBUTE("Speed", float, speed_, 1.f, AM_DEFAULT);
		URHO3D_ATTRIBUTE("WeatherRatio", float, weatherRatio_, 0.5f, AM_DEFAULT);
		URHO3D_ATTRIBUTE("HDR", bool, isHDR_, false, AM_DEFAULT);
	}
	void DayNightWeatherControl::EnableHDR(bool enable)
	{
		isHDR_ = enable;
	}
	Zone * DayNightWeatherControl::GetZone()
	{
		if(zoneNode_)
		{
			auto* zone = zoneNode_->GetComponent<Zone>();
			return zone;
		}
		return nullptr;
	}
	void DayNightWeatherControl::Start()
	{
		auto* scene = GetScene();
		if (!scene)
		{
			URHO3D_LOGERROR("DayNightWeatherControl获取不到scene！！！！！！");
			return;
		}
		zoneNode_ = scene->CreateChild("Zone");
		auto* zone = zoneNode_->CreateComponent<Zone>();
		zone->SetAmbientColor(Color(1, 1, 1, 1));
		//zone->SetAmbientColor(Color(0, 0, 0, 0.2));
		//	zone->SetZoneTexture(cache->GetResource<TextureCube>("Textures/Skybox.xml"));
		zone->SetBoundingBox(BoundingBox(Vector3(-1000, -1000, -1000), Vector3(1000, 1000, 1000)));
		//	zone->SetFogStart(100);
		//	//zone->SetFogColor(Color(0.4f, 0.5f, 0.8f));
		//	zone->SetFogEnd(3000);
		//	zone->SetFogColor(Color(4,4,4));
		//	zone->EnableFog(false);
		//	WeatherEffectUtil::SetFogEffect(zone, 100, 3000, Color(4, 4, 4));
		//	WeatherEffectUtil::CancelFogEffect(zone);
	}
	void DayNightWeatherControl::DelayedStart()
	{
		auto* scene = GetScene();
		if(!scene)
		{
			URHO3D_LOGERROR("DayNightWeatherControl获取不到scene！！！！！！");
			return;
		}

		sunNode_ = scene->CreateChild("DirectionalLight");
		sunNode_->SetDirection(Vector3(1.f, -0.5f, 0.f)); // The direction vector does not need to be normalized
		auto* light = sunNode_->CreateComponent<Light>();
		light->SetLightType(LIGHT_DIRECTIONAL);
		light->SetCastShadows(true);
		light->SetShadowResolution(1.);
		//light->SetBrightness(1000);
		//light->SetUsePhysicalValues(true);
		light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
		light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 500000.0f, 0.0f, 0.8f));
		light->SetShadowIntensity(0.0);

		skyBoxNode_ = scene->CreateChild("Sky");
		skyBoxNode_->SetScale(50.f); // The scale actually does not matter
		auto* skybox = skyBoxNode_->CreateComponent<Skybox>();
		auto* cache = GetSubsystem<ResourceCache>();
		skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
		if(isHDR_)
			skybox->SetMaterial(cache->GetResource<Material>("Materials/DynamicHDRSkybox.xml"));
		else
			skybox->SetMaterial(cache->GetResource<Material>("Materials/DynamicSkybox.xml"));
	}
	void DayNightWeatherControl::UpdateSun()
	{
		if(sunNode_)
		{
			
			auto* light = sunNode_->GetComponent<Light>();
			indirectColor_ = ComputeIncidentLight(Vector3(0, earthRadius, 0),
				Vector3(-sunDir_.x_, -sunDir_.y_, -sunDir_.z_));
//			URHO3D_LOGERROR("indirectColor:" + indirectColor_.ToString());
			light->SetColor(Color(indirectColor_.x_, indirectColor_.y_, indirectColor_.z_, 1.));
			bool isUpdate = false;
			if(Abs(lastSunDir_.DotProduct(sunDir_)) < 0.99995)
			{
				isUpdate = true;
			}
			bool hasStaticShadow = false;
			for (int i = 0; i < GetSubsystem<Renderer>()->GetNumViewports(); i++)
			{
				if(GetSubsystem<Renderer>()->GetViewport(i)&&GetSubsystem<Renderer>()->GetViewport(i)->GetEnableStaticShadow())
				{
//					GetSubsystem<Renderer>()->GetViewport(i)->UpdateStaticShadow();
					hasStaticShadow = true;
				}
			}
			if(hasStaticShadow)
			{
				if(isUpdate)
				{
					for (int i = 0; i < GetSubsystem<Renderer>()->GetNumViewports(); i++)
					{
						if (GetSubsystem<Renderer>()->GetViewport(i))
						{
							GetSubsystem<Renderer>()->GetViewport(i)->UpdateStaticShadow();
							lastSunDir_ = sunDir_;
							sunNode_->SetDirection(lastSunDir_);
						}
					}
				}
			}else
			{
				sunNode_->SetDirection(sunDir_);
			}
		}
	}
	void DayNightWeatherControl::UpdateSkyBox()
	{
		if(skyBoxNode_)
		{
			auto* skybox = skyBoxNode_->GetComponent<Skybox>();
			skybox->GetMaterial()->SetShaderParameter("SunDir", Vector3(-sunDir_.x_, -sunDir_.y_, -sunDir_.z_));
			skybox->GetMaterial()->SetShaderParameter("SunSpeedDir", sunSpeedDir_);
			skybox->GetMaterial()->SetShaderParameter("Hour", hour_);
			skybox->GetMaterial()->SetShaderParameter("IndirectColor", indirectColor_);
			skybox->GetMaterial()->SetShaderParameter("Ratio",weatherRatio_);
		}
	}
	void DayNightWeatherControl::UpdateZone()
	{
		if(zoneNode_)
		{
			auto* zone = zoneNode_->GetComponent<Zone>();
			float ambientColor = Max(Max(indirectColor_.x_, indirectColor_.y_), indirectColor_.z_) * 0.1f;
			if (!isHDR_)
			{
				if (ambientColor < 0.3)
					ambientColor = 0.3;
			}else
			{
				if (ambientColor < 0.8)
					ambientColor = 0.8;
			}
			//URHO3D_LOGERROR(String("Ambient:") + ambientColor);
			zone->SetAmbientColor(Color(ambientColor, ambientColor, ambientColor, 1.f));
		}
	}
	void DayNightWeatherControl::UpdateTime( float timeStep)
	{
		if(isDynamic_)
		{
			if(isSystemTiming_)
			{
				std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
				auto t = std::chrono::system_clock::to_time_t(now);
				auto* localTime = std::localtime(&t);
				float hour = localTime->tm_hour;
				float min = localTime->tm_min;
				float sec = localTime->tm_sec;
				hour_ = hour + min / 60.f + sec / 3600.f;
			}else
			{
				hour_ += timeStep / 3600.f * speed_;
				if (hour_ > 24.f)
				{
					hour_ = 0.f;
				}
			}
		}
		
	}
	void DayNightWeatherControl::GetSunDirection(float hour, Vector3& sunDir, Vector3& sunSpeedDir)
	{
		float ratio = M_PI * M_RADTODEG;
		if(hour >= 6.f && hour <= 18.)
		{
			ratio *= (hour - 6) / 12;
		}else if(hour < 6.f)
		{
			ratio *= (hour + 6) / 12;
		}else if(hour > 18)
		{
			ratio *= (hour - 18) / 12;
		}
		sunDir = Vector3(-Cos(ratio) * Cos(yRotation_*M_RADTODEG), -Sin(ratio), -Cos(ratio) * Sin(yRotation_*M_RADTODEG));
		sunSpeedDir = Vector3(Sin(ratio) * Cos(yRotation_*M_RADTODEG), -Cos(ratio), Sin(ratio) * Sin(yRotation_*M_RADTODEG));
	}
	bool DayNightWeatherControl::SolveQuadratic(float a, float b, float c, float & x1, float & x2)
	{
		if (b == 0)
		{
			// Handle special case where the the two vector ray.dir and V are perpendicular
			// with V = ray.orig - sphere.centre
			if (a == 0) return false;
			x1 = 0.f;
			x2 = Sqrt(-c / a);
			return true;
		}
		float discr = b * b - 4 * a * c;

		if (discr < 0) return false;

		float q = (float)((b < 0.f) ? -0.5f * (b - Sqrt(discr)) : -0.5f * (b + Sqrt(discr)));
		x1 = q / a;
		x2 = c / q;

		return true;
	}
	void DayNightWeatherControl::Swap(float & t0, float & t1)
	{
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}
	bool DayNightWeatherControl::RaySphereIntersect(Vector3 orig, Vector3 dir, float radius, float & t0, float & t1)
	{
		float A = dir.x_ * dir.x_ + dir.y_ * dir.y_ + dir.z_ * dir.z_;
		float B = 2 * (dir.x_ * orig.x_ + dir.y_ * orig.y_ + dir.z_ * orig.z_);
		float C = orig.x_ * orig.x_ + orig.y_ * orig.y_ + orig.z_ * orig.z_ - radius * radius;

		if (!SolveQuadratic(A, B, C, t0, t1)) return false;

		if (t0 > t1) Swap(t0, t1);
		return true;
	}
	Vector3 DayNightWeatherControl::ComputeIncidentLight(Vector3 orig, Vector3 sunDirection)
	{
		float t0, t1;
		unsigned int numSamplesLight = 8;
		float t0Light = 0, t1Light = 0;
		bool b = RaySphereIntersect(orig, sunDirection, atmosphereRadius, t0Light, t1Light);
		float segmentLengthLight = t1Light / numSamplesLight, tCurrentLight = 0;
		float opticalDepthLightR = 0, opticalDepthLightM = 0;
		Vector3 samplePositionLight;
		
		for (unsigned int j = 0; j < numSamplesLight; ++j)
		{
			samplePositionLight = orig+(sunDirection*(tCurrentLight + segmentLengthLight * 0.5f));			
			float heightLight = samplePositionLight.Length() - earthRadius;
			if (heightLight < 0) break;
			opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
			opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
			tCurrentLight += segmentLengthLight;

		}

		float Tx = exp(-(betaR*opticalDepthLightR).x_ - (betaM*opticalDepthLightM).x_);
		float Ty = exp(-(betaR*opticalDepthLightR).y_ - (betaM*opticalDepthLightM).y_);
		float Tz = exp(-(betaR*opticalDepthLightR).z_ - (betaM*opticalDepthLightM).z_);

		float value = dayValue_;
		float nightvalue = nightValue_;
		if (!isHDR_)
		{
			value = dayValue_/5.;
			nightvalue = nightValue_ / 5.;
		}

		//		System.out.println(T.mult(3.f));	
		if (hour_ < 6. || hour_ > 18.)
		{
			//float factor = Clamp((hour_ - 18.) / 1., 0., 1.) + Clamp((6. - hour_) / 1., 0., 1.);
			//value = value * (1. - factor) + nightvalue * factor;
			value = nightvalue;
			Tx *= value;
			//Tx = (Tx < 1.4) ? 1.4 : Tx;
			//			URHO3D_LOGERROR("directcolor:" + (Vector3(Tx, Ty, Tz)).ToString());
			return Vector3(Tx, Tx, Tx);
		}
		Tx *= value;
		Ty *= value;
		Tz *= value;
		//Tx = (Tx < 3.0) ? 3. : Tx;
		//Ty = (Ty < 1.0) ? 1. : Ty;
		//Tz = (Tz < 0.3) ? 0.3 : Tz;
		//		URHO3D_LOGERROR("directcolor:" + (Vector3(Tx, Ty, Tz)).ToString());
		return Vector3(Tx, Ty, Tz);
		
	}
}
