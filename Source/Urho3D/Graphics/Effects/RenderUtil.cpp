#include "../../Graphics/Effects/RenderUtil.h"
#include "../../Core/Context.h"
#include "../../Graphics/Graphics.h"
#include "../../Resource/Image.h"
#include "../../Graphics/Texture2D.h"
#include "../../Graphics/Camera.h"

#include "../../Engine/Engine.h"
#include "../../Core/AsynCallBack.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/Technique.h"
#include "../../Graphics/Drawable.h"

#include "../../Graphics/RenderPath.h"
#include "../../Resource/XMLFile.h"

#include "../../IO/Log.h"

namespace Urho3D
{
	void RenderUtil::CaptureScreen(Context* context, Image* screenshot)
	{
		Graphics* graphics = context->GetSubsystem<Graphics>();
		graphics->TakeScreenShot(*screenshot);
	}
	
	SharedPtr<Texture2D> RenderUtil::CreateRenderTexture(Context* context, int width, int height)
	{
		SharedPtr<Texture2D> renderTexture(new Texture2D(context));
		renderTexture->SetSize(width, height, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
		renderTexture->SetFilterMode(FILTER_BILINEAR);
		return renderTexture;
	}

	SharedPtr<Viewport> RenderUtil::CreateViewport(Context* context, Texture2D* renderTexture, Scene* scene, Node* cameraNode)
	{
		if (!renderTexture)
		{
			URHO3D_LOGERROR("RenderUtil#CreateViewport中renderTexture为空！！！！！");
			return nullptr;
		}
		RenderSurface* surface = renderTexture->GetRenderSurface();
		auto* cache = context->GetSubsystem<ResourceCache>();
		auto* defaultRenderPath_ = new RenderPath();
		defaultRenderPath_->Load(cache->GetResource<XMLFile>("RenderPaths/ForwardHWDepth.xml"));
		SharedPtr<Viewport> rttViewport(new Viewport(context, scene, cameraNode->GetComponent<Camera>(), defaultRenderPath_));
		surface->SetViewport(0, rttViewport);
		surface->SetUpdateMode(RenderSurfaceUpdateMode::SURFACE_MANUALUPDATE);
		return rttViewport;
	}


	void RenderUtil::CaptureCamera(Context* context, Texture2D* renderTexture, std::function<void(void)> caller, const String& teniqueName)
	{
		if (!renderTexture)
		{
			URHO3D_LOGERROR("RenderUtil#CreateViewport中renderTexture为空！！！！！");
			return;
		}
		
		if (!teniqueName.Empty())
		{
			SetOverrideTechnique(renderTexture->GetRenderSurface()->GetViewport(0)->GetScene(), teniqueName);
		}
		
		renderTexture->GetRenderSurface()->QueueUpdate();
		context->GetSubsystem<Engine>()->PerformFunctionMainThread(URHO3D_ASYNCALLBACK_SIMPLE([=] {
			context->GetSubsystem<Engine>()->PerformFunctionMainThread(URHO3D_ASYNCALLBACK_SIMPLE(caller));
		}));
	}

	void RenderUtil::SetOverrideTechnique(Node * node, const String& techniqueName)
	{
		if (!node)
			return;
		if (node->GetNumComponents() > 0)
		{
			auto* cache = node->GetSubsystem<ResourceCache>();
			Technique* technique = cache->GetResource<Technique>(techniqueName);
			const Vector<SharedPtr<Component> >& components = node->GetComponents();
			for (int i = 0; i < components.Size(); i++)
			{
				if (components[i]->GetTypeInfo()->IsTypeOf(Drawable::GetTypeStatic()))
				{
					Drawable* drawable = dynamic_cast<Drawable*>(components[i].Get());
					if (!drawable)
						continue;
					drawable->SetOverrideTechnique(technique);
				}
			}
		}
		if (node->GetNumChildren() > 0)
		{
			for (int i = 0; i < node->GetNumChildren(); i++)
			{
				SetOverrideTechnique(node->GetChild(i), techniqueName);
			}
		}
	}

}
