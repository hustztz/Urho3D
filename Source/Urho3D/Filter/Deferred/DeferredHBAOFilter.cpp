
#include "../../Filter/Deferred/DeferredHBAOFilter.h"
#include "../../Graphics/Viewport.h"
#include "../../Graphics/RenderPath.h"
#include "../../Resource/ResourceCache.h"
#include "../../Resource/XMLFile.h"

namespace Urho3D
{
	DeferredHBAOFilter::DeferredHBAOFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{

	}

	void DeferredHBAOFilter::Initial()
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
			renderPath->Load(cache->GetResource<XMLFile>("PostProcess/Deferred/DeferredHBAOUsingNormal.xml"));
		else
			renderPath->Load(cache->GetResource<XMLFile>("PostProcess/Deferred/DeferredHBAO.xml"));
		
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
	void DeferredHBAOFilter::SetHBAOQuality(HBAOQuality quality)
	{
		switch(quality)
		{
		case HBAOQuality::High:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 16.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 8.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 8.f);
			break;
		case HBAOQuality::Middle:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 4.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 2.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 4.f);
			break;
		case HBAOQuality::Low:
			viewPort_->GetRenderPath()->SetShaderParameter("AONumDir", 2.f);
			viewPort_->GetRenderPath()->SetShaderParameter("NumSteps", 1.f);
			viewPort_->GetRenderPath()->SetShaderParameter("BilateralBlurRadius", 2.f);
			break;
		}
	}
	void DeferredHBAOFilter::SetHBAOIntensity(float intensity)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("AOContrast", intensity);
	}
}