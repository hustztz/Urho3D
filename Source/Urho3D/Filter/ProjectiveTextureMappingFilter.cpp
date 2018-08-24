
#include "../Filter/ProjectiveTextureMappingFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../Scene/Node.h"
#include "../Graphics/Drawable.h"
#include "../IO/Log.h"
#include "../Graphics/Material.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Texture2D.h"
#include "../Graphics/Renderer.h"

namespace Urho3D
{
	ProjectiveTextureMappingFilter::ProjectiveTextureMappingFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{
	}

	ProjectiveTextureMappingFilter::~ProjectiveTextureMappingFilter()
	{
	}



	void ProjectiveTextureMappingFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/ProjectiveTextureMapping.xml"));
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
	void ProjectiveTextureMappingFilter::SetProjectionCamera(Camera * cam)
	{
		cam_ = cam;
		Matrix4 projection = cam_->GetGPUProjection();
		auto* graphics = GetSubsystem<Graphics>();
#ifdef URHO3D_OPENGL
		// Add constant depth bias manually to the projection matrix due to glPolygonOffset() inconsistency
		float constantBias = 2.0f * graphics->GetDepthConstantBias();
		projection.m22_ += projection.m32_ * constantBias;
		projection.m23_ += projection.m33_ * constantBias;
#endif
		viewPort_->GetRenderPath()->SetShaderParameter("PTMViewProjectionMatrix", projection * cam_->GetView());
	}
	void ProjectiveTextureMappingFilter::SetProjectionTexture(Texture2D * tex)
	{
		projectiveTexture_ = tex;
		projectiveTexture_->SetName("projectiveTexture.manual");
		auto* cache = GetSubsystem<ResourceCache>();
		cache->AddManualResource(projectiveTexture_);
	}
	void ProjectiveTextureMappingFilter::Update(float timeStep)
	{
	}
}