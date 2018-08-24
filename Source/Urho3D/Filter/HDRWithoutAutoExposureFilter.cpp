
#include "../Filter/HDRWithoutAutoExposureFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"

namespace Urho3D
{
	HDRWithoutAutoExposureFilter::HDRWithoutAutoExposureFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{

	}

	void HDRWithoutAutoExposureFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/HDRWithoutAutoExposure.xml"));
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
}