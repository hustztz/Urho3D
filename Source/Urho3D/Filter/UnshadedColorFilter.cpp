
#include "../Filter/UnshadedColorFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../Scene/Node.h"
#include "../Graphics/Drawable.h"
#include "../IO/Log.h"
#include "../Graphics/Material.h"

namespace Urho3D
{
	UnshadedColorFilter::UnshadedColorFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{


	}

	UnshadedColorFilter::~UnshadedColorFilter()
	{
		HashMap<SharedPtr<Node>, Color>::Iterator iter = unshadedColorModels_.Begin();
		for (; iter != unshadedColorModels_.End(); ++iter)
		{
			InternalRemove(iter->first_);
		}
		unshadedColorModels_.Clear();
	}



	void UnshadedColorFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/UnshadedColor.xml"));
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
	
	void UnshadedColorFilter::AddUnshadedColorModel(Node* node, Color color)
	{
		if(!node)
			return;

		unshadedColorModels_.Insert(MakePair(SharedPtr<Node>(node), color));
		InternalProcess(node, color);
	}
	
	void UnshadedColorFilter::ClearUnshadedColorModel(Node* node)
	{
		if (!node)
			return;

		if (unshadedColorModels_.Contains(SharedPtr<Node>(node)))
		{
			unshadedColorModels_.Erase(SharedPtr<Node>(node));
			InternalRemove(node);
		}
	}
	void UnshadedColorFilter::InternalProcess(Node * node,Color color)
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
					auto* pass = drawable->AddPass("UnshadedColor", "Unlit", "Unlit");
					if(pass)
					{
						//pass->SetDepthBias(0.01, 0.1);
						pass->SetBlendMode(BlendMode::BLEND_REPLACE);
						drawable->SetOverrideShaderParameter("MatDiffColor", color);
					}
				}
			}
		}
		if (node->GetNumChildren() > 0)
		{
			for (int i = 0; i < node->GetNumChildren(); i++)
			{
				InternalProcess(node->GetChild(i), color);
			}
		}
	}
	void UnshadedColorFilter::InternalRemove(Node * node)
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
					drawable->RemovePass("UnshadedColor");
					drawable->RemoveOverrideShaderParameter("MatDiffColor");
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