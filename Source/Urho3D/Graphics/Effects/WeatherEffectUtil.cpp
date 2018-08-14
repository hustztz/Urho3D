#include "../../Graphics/Effects/WeatherEffectUtil.h"
#include "../../Graphics/Camera.h"
#include "../../Scene/Node.h"
#include "../../IO/Log.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/ParticleEffect.h"
#include "../../Graphics/ParticleEmitter.h"
#include "../../Graphics/Zone.h"

namespace Urho3D
{
	void WeatherEffectUtil::SetRainEffect(Camera* cam)
	{
		Node* cameraNode = cam->GetNode();
		if(!cameraNode)
		{
			URHO3D_LOGERROR("调用WeatherEffectUtil::SetRainEffect时，传人的Camera组件没有加入节点！！！！");
			return;
		}
		if(cameraNode->GetChild("RainNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::SetRainEffect时，传人的Camera节点已加入下雨粒子节点！！！！");
			return;
		}
		if (cameraNode->GetChild("SnowNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::SetSnowEffect时，传人的Camera节点已加入下雪粒子节点！！！！");
			return;
		}
		Node* rainNode = cameraNode->CreateChild("RainNode");
		rainNode->SetPosition(Vector3(0.0f, 10.f, 0.0f));
		//	emitter->SetRotation(Quaternion(30, Vector3(0, 0, 1)));
		auto* particleEmitter = rainNode->CreateComponent<ParticleEmitter>();
		auto* cache = rainNode->GetSubsystem<ResourceCache>();
		auto* effect = cache->GetResource<ParticleEffect>("Particle/rain.xml");
		particleEmitter->SetEffect(effect);
		particleEmitter->SetEmitting(true);
		cam->SetIsRaining(true);
	}
	void WeatherEffectUtil::CancelRainEffect(Camera* cam)
	{
		Node* cameraNode = cam->GetNode();
		if (!cameraNode)
		{
			URHO3D_LOGERROR("调用WeatherEffectUtil::CancelRainEffect时，传人的Camera组件没有加入节点！！！！");
			return;
		}
		if (!cameraNode->GetChild("RainNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::CancelRainEffect时，传人的Camera节点下并没有下雨粒子节点！！！！");
			return;
		}
		cameraNode->RemoveChild(cameraNode->GetChild("RainNode"));
		auto* cache = cameraNode->GetSubsystem<ResourceCache>();
		cache->ReleaseResource<ParticleEffect>("Particle/rain.xml");
		cam->SetIsRaining(false);
	}
	void WeatherEffectUtil::SetSnowEffect(Camera * cam)
	{
		Node* cameraNode = cam->GetNode();
		if (!cameraNode)
		{
			URHO3D_LOGERROR("调用WeatherEffectUtil::SetSnowEffect时，传人的Camera组件没有加入节点！！！！");
			return;
		}
		if (cameraNode->GetChild("SnowNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::SetSnowEffect时，传人的Camera节点已加入下雪粒子节点！！！！");
			return;
		}
		if (cameraNode->GetChild("RainNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::SetSnowEffect时，传人的Camera节点已加入下雨粒子节点！！！！");
			return;
		}
		Node* snowNode = cameraNode->CreateChild("SnowNode");
		snowNode->SetPosition(Vector3(0.0f, 10.f, 0.0f));
		//	emitter->SetRotation(Quaternion(30, Vector3(0, 0, 1)));
		auto* particleEmitter = snowNode->CreateComponent<ParticleEmitter>();
		auto* cache = snowNode->GetSubsystem<ResourceCache>();
		auto* effect = cache->GetResource<ParticleEffect>("Particle/snow.xml");
		particleEmitter->SetEffect(effect);
		particleEmitter->SetEmitting(true);
		cam->SetIsSnowing(true);
	}
	void WeatherEffectUtil::CancelSnowEffect(Camera * cam)
	{
		Node* cameraNode = cam->GetNode();
		if (!cameraNode)
		{
			URHO3D_LOGERROR("调用WeatherEffectUtil::CancelSnowEffect时，传人的Camera组件没有加入节点！！！！");
			return;
		}
		if (!cameraNode->GetChild("SnowNode"))
		{
			URHO3D_LOGWARNING("调用WeatherEffectUtil::CancelSnowEffect时，传人的Camera节点下并没有下雪粒子节点！！！！");
			return;
		}
		cameraNode->RemoveChild(cameraNode->GetChild("SnowNode"));
		auto* cache = cameraNode->GetSubsystem<ResourceCache>();
		cache->ReleaseResource<ParticleEffect>("Particle/snow.xml");
		cam->SetIsSnowing(false);
	}
	void WeatherEffectUtil::SetFogEffect(Zone * zone, float startFog, float endFog, Color fogColor)
	{
		zone->SetFogStart(startFog);
		zone->SetFogEnd(endFog);
		zone->SetFogColor(fogColor);
		zone->EnableFog(true);
	}
	void WeatherEffectUtil::CancelFogEffect(Zone * zone)
	{
		zone->EnableFog(false);
	}
}
