
#include "../Filter/VolumeInfluenceFilter.h"
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
	VolumeInfluenceFilter::VolumeInfluenceFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort)
	{
	}

	VolumeInfluenceFilter::~VolumeInfluenceFilter()
	{
		if(!volume_)
		{
			InternalRemove(volume_);
		}
	}



	void VolumeInfluenceFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/VolumeInfluence.xml"));
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

	void VolumeInfluenceFilter::Update(float timeStep)
	{
		if(viewPort_)
			viewPort_->GetRenderPath()->SetShaderParameter("MarkColor", markColor_);
	}
	
	void VolumeInfluenceFilter::SetVolume(Node* node)
	{
		if(!node)
		{
			InternalRemove(volume_);
			volume_ = nullptr;
			return;
		}			

		volume_ = node;
		InternalProcess(volume_);
	}
	
	void VolumeInfluenceFilter::InternalProcess(Node * node)
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
					auto* dpethFeel1Pass = drawable->AddPass("depthFeell", "DepthPeel", "DepthPeel");
					auto* dpethFeel2Pass = drawable->AddPass("depthFeel2", "DepthPeel", "DepthPeel", "HasPeelDepth", "HasPeelDepth");
					dpethFeel1Pass->SetCullMode(CullMode::CULL_NONE);
					dpethFeel1Pass->SetDepthWrite(true);
					//	dpethFeel1Pass->SetDepthTestMode(CompareMode::CMP_ALWAYS);
					dpethFeel2Pass->SetCullMode(CullMode::CULL_NONE);
					dpethFeel2Pass->SetDepthWrite(true);
					//	dpethFeel2Pass->SetDepthTestMode(CompareMode::CMP_ALWAYS);
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
	void VolumeInfluenceFilter::InternalRemove(Node * node)
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
					drawable->RemovePass("depthFeell");
					drawable->RemovePass("depthFeel2");
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