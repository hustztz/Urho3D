
#include "../Filter/TemporalAAFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../IO/Log.h"
#include "../Graphics/Camera.h"
#include "../Graphics/View.h"
#include "../Graphics/Graphics.h"

namespace Urho3D
{
	TemporalAAFilter::TemporalAAFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{

	}

	void TemporalAAFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/TemporalAA.xml"));
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

	void TemporalAAFilter::Update(float timeStep)
	{
		if (!IsEnable())
		{
			viewPort_->GetCamera()->SetProjectionOffset(Vector2(0, 0));
			return;
		}
		unsigned height = 0, width = 0;
		width = viewPort_->GetRect().Width();
		height = viewPort_->GetRect().Height();
		if (width == 0 || height == 0)
		{
			width = GetSubsystem<Graphics>()->GetWidth();
			height = GetSubsystem<Graphics>()->GetHeight();
		}

		float offsetX = HaltonRandom(GetSubsystem<Time>()->GetFrameNumber() % 1023, 2);
		float offsetY = HaltonRandom(GetSubsystem<Time>()->GetFrameNumber() % 1023, 3);
	//	offsetX = offsetX*2.f - 1.f;
	//	offsetY = offsetY*2.f - 1.f;
	//	offsetX *= 1;
	//	offsetY *= 1;
		offsetX /= width;
		offsetY /= height;

		viewPort_->GetRenderPath()->SetShaderParameter("TemporlAAJitterUV", Vector2(offsetX, offsetY));
		viewPort_->GetCamera()->SetProjectionOffset(Vector2(offsetX , offsetY));
		if(GetSubsystem<Time>()->GetFrameNumber() == 0)
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
		}else
			viewPort_->GetRenderPath()->SetShaderParameter("LastVPMatrix", lastVPMatrix_);
//		{
//			Matrix4 projection = viewPort_->GetCamera()->GetGPUProjection();
//			auto* graphics = GetSubsystem<Graphics>();
//#ifdef URHO3D_OPENGL
//			// Add constant depth bias manually to the projection matrix due to glPolygonOffset() inconsistency
//			float constantBias = 2.0f * graphics->GetDepthConstantBias();
//			projection.m22_ += projection.m32_ * constantBias;
//			projection.m23_ += projection.m33_ * constantBias;
//#endif
//			URHO3D_LOGERROR(String("VPMDemp:") + (projection * viewPort_->GetCamera()->GetView()).ToString());
//		URHO3D_LOGERROR("TemporalAA Update");
//		}
	}

	void TemporalAAFilter::PostUpdate()
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
//		URHO3D_LOGERROR("TemporalAA PostUpdate");
	}
}
