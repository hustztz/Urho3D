
#include "../Filter/DepthOfFieldFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../IO/Log.h"

namespace Urho3D
{
	DepthOfFieldFilter::DepthOfFieldFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort),
		focusDistance_(30.f),
		focalLength_(0.1f),
		fNumber_(1.4f),
		kFilmHeight_(0.024f)
	{

	}

	void DepthOfFieldFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/DepthOfField.xml"));
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
	void DepthOfFieldFilter::Update(float timeStep)
	{
		if (focusDistance_ < 0.01f) focusDistance_ = 0.01f;
		if (fNumber_ < 0.1f) fNumber_ = 0.1f;

		auto s1 = CalculateFocusDistance();
		auto f = CalculateFocalLength();
		s1 = Max(s1, f);
		viewPort_->GetRenderPath()->SetShaderParameter("DofDistance", s1);
		
		auto coeff = f * f / (fNumber_ * (10.f - f) * kFilmHeight_ * 2.f);
		viewPort_->GetRenderPath()->SetShaderParameter("DofLensCoeff", coeff);
		//URHO3D_LOGWARNING(String("DofLensCoeff:") + coeff);

	}
	void DepthOfFieldFilter::SetMaxCoc(float maxCoc)
	{
		viewPort_->GetRenderPath()->SetShaderParameter("MaxCoC", maxCoc);
		viewPort_->GetRenderPath()->SetShaderParameter("RcpMaxCoC", 1.f / maxCoc);
	}
	void DepthOfFieldFilter::SetFocusDistance(float focusDistance)
	{
		focusDistance_ = focusDistance;
	}
	void DepthOfFieldFilter::SetFocalLength(float focalLength)
	{
		focalLength_ = focalLength;
	}
	void DepthOfFieldFilter::SetFNumber(float fNumber)
	{
		fNumber_ = fNumber;
	}
	float DepthOfFieldFilter::CalculateFocusDistance()
	{
		return focusDistance_;
	}
	float DepthOfFieldFilter::CalculateFocalLength()
	{
		return focalLength_;
	}
}
