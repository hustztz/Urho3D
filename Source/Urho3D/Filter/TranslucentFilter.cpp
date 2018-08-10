
#include "../Filter/TranslucentFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../Scene/Node.h"
#include "../Graphics/Drawable.h"
#include "../Graphics/Material.h"
#include "../Graphics/Renderer.h"

namespace Urho3D
{
	TranslucentFilter::TranslucentFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{


	}

	TranslucentFilter::~TranslucentFilter()
	{
		HashSet<SharedPtr<Node> >::Iterator iter = models_.Begin();
		for (; iter != models_.End(); ++iter)
		{
			InternalRemove(iter->Get());
		}
		models_.Clear();
	}



	void TranslucentFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/TranslucentHWDepth.xml"));
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
	
	void TranslucentFilter::AddModel(Node* node)
	{
		if(!node)
			return;

		HashSet<SharedPtr<Node> >::Iterator iter = models_.Find(SharedPtr<Node>(node));
		if (iter == models_.End())
		{
			models_.Insert(SharedPtr<Node>(node));
			InternalProcess(node);
		}
	}
	
	void TranslucentFilter::ClearModel(Node* node)
	{
		if (!node)
			return;

		if (models_.Contains(SharedPtr<Node>(node)))
		{
			models_.Erase(SharedPtr<Node>(node));
			InternalRemove(node);
		}
	}
	void TranslucentFilter::InternalProcess(Node * node)
	{
		if (!node)
			return;
		if (node->GetNumComponents() > 0)
		{
			const Vector<SharedPtr<Component> >& components = node->GetComponents();
			for (int i = 0; i < components.Size(); i++)
			{
				if (components[i]->GetTypeInfo()->IsTypeOf(Drawable::GetTypeStatic()))
				{
					Drawable* drawable = dynamic_cast<Drawable*>(components[i].Get());
					if (!drawable)
						continue;
					if (drawable->GetBatches().Size() > 0)
					{
						Material* mat;
						if (drawable->GetBatches()[0].material_)
							mat = drawable->GetBatches()[0].material_;
						else
							mat = node->GetSubsystem<Renderer>()->GetDefaultMaterial();
						auto* tech = mat->GetTechnique(0);
						SharedPtr<Technique> overrideTech = tech->Clone();
						overrideTech->RemoveAllPasses();
						Pass* pass = nullptr;
						if(tech->GetPass(Technique::GetPassIndex("litbase")))
						{
						pass = tech->GetPass(Technique::GetPassIndex("litbase"));
						}else if(tech->GetPass(Technique::GetPassIndex("base")))
						{
						pass = tech->GetPass(Technique::GetPassIndex("base"));
						} else if (tech->GetPass(Technique::GetPassIndex("alpha")))
						{
						pass = tech->GetPass(Technique::GetPassIndex("alpha"));
						}
						if (!pass)
							return;
						auto* overridepass = overrideTech->CreatePass("translucent");
						overridepass->CopyAllState(pass);
						overridepass->SetDepthTestMode(CMP_LESS);
						overridepass->SetDepthWrite(true);
						overridepass->SetLightingMode(LIGHTING_PERPIXEL);
//						overridepass->SetPixelShaderDefines(overridepass->GetPixelShaderDefines() + " PERPIXEL");
						drawable->SetOverrideTechnique(overrideTech);
					}
				}
			}
		}
		if (node->GetNumChildren() > 0)
		{
			for (int i = 0; i < node->GetNumChildren(); i++)
			{
				InternalProcess(node->GetChild(i));
			}
		}
	}
	void TranslucentFilter::InternalRemove(Node * node)
	{
		if (!node)
			return;
		if (node->GetNumComponents() > 0)
		{
			const Vector<SharedPtr<Component> >& components = node->GetComponents();
			for (int i = 0; i < components.Size(); i++)
			{
				if (components[i]->GetTypeInfo()->IsTypeOf(Drawable::GetTypeStatic()))
				{
					Drawable* drawable = dynamic_cast<Drawable*>(components[i].Get());
					if (!drawable)
						continue;
					drawable->CancelOverrideTechnique();
				}
			}
		}
		if (node->GetNumChildren() > 0)
		{
			for (int i = 0; i < node->GetNumChildren(); i++)
			{
				InternalRemove(node->GetChild(i));
			}
		}
	}
}
