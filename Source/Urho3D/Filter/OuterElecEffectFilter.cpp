
#include "../Filter/OuterElecEffectFilter.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../Scene/Node.h"
#include "../Graphics/Drawable.h"
#include "../IO/Log.h"
#include "../Graphics/Material.h"
#include "../Math/MathDefs.h"

namespace Urho3D
{
	OuterElecEffectFilter::OuterElecEffectFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{


	}

	OuterElecEffectFilter::~OuterElecEffectFilter()
	{
		HashSet<SharedPtr<Node> >::Iterator iter = outerElecModels_.Begin();
		for (; iter != outerElecModels_.End(); ++iter)
		{
			InternalRemove(iter->Get());
		}
		outerElecModels_.Clear();
	}



	void OuterElecEffectFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/OuterElecEffect.xml"));
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
	
	void OuterElecEffectFilter::AddOuterElecModel(Node* node, bool xDir, float entend, float speed, Color color, float sample)
	{
		if(!node)
			return;

		HashSet<SharedPtr<Node> >::Iterator iter = outerElecModels_.Find(SharedPtr<Node>(node));
		if (iter == outerElecModels_.End())
		{
			outerElecModels_.Insert(SharedPtr<Node>(node));
			InternalProcess(node, xDir, entend, speed, color, sample);
		}
	}
	
	void OuterElecEffectFilter::ClearOuterElecModel(Node* node)
	{
		if (!node)
			return;

		if (outerElecModels_.Contains(SharedPtr<Node>(node)))
		{
			outerElecModels_.Erase(SharedPtr<Node>(node));
			InternalRemove(node);
		}
	}
	void OuterElecEffectFilter::InternalProcess(Node * node, bool xDir, float entend, float speed, Color color, float sample)
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
					auto* pass = drawable->AddPass("OuterElecEffect", "OuterElecEffect", "OuterElecEffect");
					if (pass)
					{
						pass->SetBlendMode(BlendMode::BLEND_ADD);
					}
					drawable->SetOverrideShaderParameter("XDirection", xDir);
					drawable->SetOverrideShaderParameter("Extend", entend);
					drawable->SetOverrideShaderParameter("Speed", speed);
					drawable->SetOverrideShaderParameter("Color", color);
					drawable->SetOverrideShaderParameter("Sample", sample);
				}
			}
		}
		if (node->GetNumChildren() > 0)
		{
			for (int i = 0; i < node->GetNumChildren(); i++)
			{
				InternalProcess(node->GetChild(i), xDir, entend, speed, color, sample);
			}
		}
	}
	void OuterElecEffectFilter::InternalRemove(Node * node)
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
					drawable->RemovePass("OuterElecEffect");
					drawable->RemoveOverrideShaderParameter("XDirection");
					drawable->RemoveOverrideShaderParameter("Extend");
					drawable->RemoveOverrideShaderParameter("Speed");
					drawable->RemoveOverrideShaderParameter("Color");
					drawable->RemoveOverrideShaderParameter("Sample");
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