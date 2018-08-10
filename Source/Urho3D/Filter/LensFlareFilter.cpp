
#include "../Filter/LensFlareFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../IO/Log.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/View.h"
#include "../Graphics/Texture2D.h"
#include "../Filter/BloomHDRFilter.h"

namespace Urho3D
{
	LensFlareFilter::LensFlareFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort),
		bokehSize_(8.f), 
		threshold_(0.04f)
	{
		SubscribeToEvent(E_RENDERPATHEVENT, URHO3D_HANDLER(LensFlareFilter, LensFlareBlur));
		bokehTex_ = GetSubsystem<ResourceCache>()->GetResource<Texture2D>("Textures/LensFlare/bokehshape.png");
	}
	
	void LensFlareFilter::SetEnable(bool enable)
	{
		if(viewPort_->GetFilter<BloomHDRFilter>() && viewPort_->GetFilter<BloomHDRFilter>()->IsEnable())
		{
			Filter::SetEnable(enable);
		} else
		{
			URHO3D_LOGERROR("BloomHDRFilter没有开启，所以LensFlareFilter不能打开！！！！");
			Filter::SetEnable(false);
		}
			
	}

	void LensFlareFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/LensFlare.xml"));
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

	void LensFlareFilter::LensFlareBlur(StringHash eventType, VariantMap& eventData)
	{
		String eventName = eventData[RenderPathEvent::P_NAME].GetString();
		auto* graphics = GetSubsystem<Graphics>();
		auto* renderer = GetSubsystem<Renderer>();
		auto* cache = GetSubsystem<ResourceCache>();
		auto* view = viewPort_->GetView();
//		view->GetViewSize();
		
		Texture* blurTexture = view->FindNamedTexture("lensFlareBlur", true, false);
		Texture* flareTexture = view->FindNamedTexture("lensFlare", true, false);
		if (!blurTexture || !flareTexture)
			return;
		flareTexture->SetFilterMode(TextureFilterMode::FILTER_TRILINEAR);
		String vertexDefines = "";
		String pixelDefines = "";

		ShaderVariation* vs = graphics->GetShader(VS, "LensFlareBlur", vertexDefines);
		if (!vs)
			return;
		ShaderVariation* ps = graphics->GetShader(PS, "LensFlareBlur", pixelDefines);
		if (!ps)
			return;
		
		// Set shaders & shader parameters and textures
		graphics->SetShaders(vs, ps);

		Vector2 viewSize(blurTexture->GetWidth()/4, blurTexture->GetHeight()/4);
		unsigned int tileSize = 1;
		Vector2 tileCount = viewSize / tileSize;
		float pixelKernelSize = bokehSize_ / 100.f * viewSize.x_;
		Vector4 tileCountAndSize(tileCount.x_, tileCount.y_, tileSize, tileSize);
		Vector4 colorScaleValue(1.f / Max(1.f, pixelKernelSize*pixelKernelSize), threshold_, 0.f, 0.f);
		Vector4 kernelSizeValue(pixelKernelSize, pixelKernelSize, 0.f, 0.f);
		graphics->SetShaderParameter("TileCountAndSize", tileCountAndSize);
		graphics->SetShaderParameter("ColorScaleValue", colorScaleValue);
		graphics->SetShaderParameter("KernelSizeValue", kernelSizeValue);

		graphics->SetTexture(TU_DIFFUSE, flareTexture);
		graphics->SetTexture(TU_SPECULAR, bokehTex_);

		graphics->SetRenderTarget(0, static_cast<Texture2D*>(blurTexture)->GetRenderSurface());

		graphics->SetBlendMode(BLEND_ADD);
		
		// needs to be the same on shader side
		unsigned QuadsPerInstance = 4;

		graphics->DrawArraysInstanced(TRIANGLE_LIST, 0, 6, Round(tileCount.x_*tileCount.y_ / QuadsPerInstance));
	}
}
