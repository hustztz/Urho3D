
#include "../../Filter/Deferred/DeferredHBAOTemporalFilter.h"
#include "../../Graphics/Viewport.h"
#include "../../Graphics/RenderPath.h"
#include "../../Resource/ResourceCache.h"
#include "../../Resource/XMLFile.h"
#include "../../Graphics/Viewport.h"
#include "../../Graphics/Camera.h"
#include "../../Graphics/Graphics.h"

namespace Urho3D
{
	DeferredHBAOTemporalFilter::DeferredHBAOTemporalFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{

	}

	void DeferredHBAOTemporalFilter::Initial()
	{
		bool hasNormal = false;
		for (int i = 0; i < viewPort_->GetRenderPath()->GetNumRenderTargets(); i++)
		{
			if (viewPort_->GetRenderPath()->renderTargets_[i].name_.Compare("normal") == 0)
			{
				hasNormal = true;
				break;
			}
		}

		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		if(hasNormal)
			renderPath->Load(cache->GetResource<XMLFile>("PostProcess/Deferred/DeferredHBAOTemporalUsingNormal.xml"));
		else
			renderPath->Load(cache->GetResource<XMLFile>("PostProcess/Deferred/DeferredHBAOTemporal.xml"));
		
		String tag = GetTypeName();
		for (int i = 0; i < renderPath->GetNumCommands(); ++i)
		{
			if (renderPath->GetCommand(i)->enabled_)
			{
				renderPath->GetCommand(i)->tag_ = tag;
			}
			viewPort_->GetRenderPath()->AddCommand(*(renderPath->GetCommand(i)));
		}
		for (int i = 0; i < renderPath->GetNumRenderTargets(); ++i)
		{
			if (renderPath->renderTargets_[i].enabled_)
			{
				renderPath->renderTargets_[i].tag_ = tag;
			}
			viewPort_->GetRenderPath()->AddRenderTarget(renderPath->renderTargets_[i]);
		}
	}
	void DeferredHBAOTemporalFilter::SetHBAOQuality(HBAOQualityTemporal quality)
	{
		switch(quality)
		{
		case HBAOQualityTemporal::HighTemporal:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 6.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 3.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 8.f);
			break;
		case HBAOQualityTemporal::MiddleTemporal:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 4.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 2.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 4.f);
			break;
		case HBAOQualityTemporal::LowTemporal:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 2.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 1.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 2.f);
			break;
		}
	}
	void DeferredHBAOTemporalFilter::SetHBAOIntensity(float intensity)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AOContrast", intensity);
	}

	void DeferredHBAOTemporalFilter::SetAORadius(float radius)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AORadius", radius);
		viewPort_->GetRenderPath()->SetShaderParameter("AORadiusInv", 1.f/radius);
		viewPort_->GetRenderPath()->SetShaderParameter("AORadiusSqr", radius*radius);
	}
	void DeferredHBAOTemporalFilter::SetAONumDir(float num)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", num);
	}
	void DeferredHBAOTemporalFilter::SetAONumSteps(float num)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", num);
	}
	void DeferredHBAOTemporalFilter::SetAOAttenuation(float attenuation)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AOAttenuation", attenuation);
	}
	void DeferredHBAOTemporalFilter::SetAOAngleBias(float angleBias)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AOAngleBias", angleBias * M_DEGTORAD);
		viewPort_->GetRenderPath()->SetShaderParameter("AOTanAngleBias", Tan(angleBias));
	}
	void DeferredHBAOTemporalFilter::SetBilateralBlurRadius(float radius)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", radius);
	}

	void DeferredHBAOTemporalFilter::Update(float timeStep)
	{
		if (!IsEnable())
		{
			viewPort_->GetCamera()->SetProjectionOffset(Vector2(0, 0));
			return;
		}


		float randomX = HaltonRandom(GetSubsystem<Time>()->GetFrameNumber() % 10023, 2);
		float randomY = HaltonRandom(GetSubsystem<Time>()->GetFrameNumber() % 761023, 3);
		viewPort_->GetRenderPath()->SetShaderParameter("AORandom", Vector2(randomX, randomY));

		if (GetSubsystem<Time>()->GetFrameNumber() == 0)
		{
			Matrix4 projection = viewPort_->GetCamera()->GetGPUProjection();
			auto* graphics = GetSubsystem<Graphics>();
#ifdef URHO3D_OPENGL
			// Add constant depth bias manually to the projection matrix due to glPolygonOffset() inconsistency
			float constantBias = 2.0f * graphics->GetDepthConstantBias();
			projection.m22_ += projection.m32_ * constantBias;
			projection.m23_ += projection.m33_ * constantBias;
#endif
			viewPort_->GetRenderPath()->SetShaderParameter("LastVPMatrix", projection * viewPort_->GetCamera()->GetView());
		}
		else
			viewPort_->GetRenderPath()->SetShaderParameter("LastVPMatrix", lastVPMatrix_);
	}

	void DeferredHBAOTemporalFilter::PostUpdate()
	{
		Matrix4 projection = viewPort_->GetCamera()->GetGPUProjection();
		auto* graphics = GetSubsystem<Graphics>();
#ifdef URHO3D_OPENGL
		// Add constant depth bias manually to the projection matrix due to glPolygonOffset() inconsistency
		float constantBias = 2.0f * graphics->GetDepthConstantBias();
		projection.m22_ += projection.m32_ * constantBias;
		projection.m23_ += projection.m33_ * constantBias;
#endif
		projection.m02_ = 0.f;
		projection.m12_ = 0.f;
		lastVPMatrix_ = projection * viewPort_->GetCamera()->GetView();
		//	URHO3D_LOGERROR("TemporalAA PostUpdate");
	}
}