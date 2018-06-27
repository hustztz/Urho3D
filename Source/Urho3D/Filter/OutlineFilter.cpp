
#include "../Filter/OutlineFilter.h"
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
	OutlineFilter::OutlineFilter(Context* context, Viewport* viewPort) :
		Filter(context, viewPort),
		width_(2),
		outlineColor_(1., 1., 1., 1.),
		alpha_(0),
		time_(0),
		maxWith_(20)
	{


	}

	OutlineFilter::~OutlineFilter()
	{
		HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Begin();
		for (; iter != outlineModels_.End(); ++iter)
		{
			InternalRemove(iter->first_);
		}
		outlineModels_.Clear();
	}



	void OutlineFilter::Initial()
	{
		SharedPtr<RenderPath> renderPath(new RenderPath());
		auto* cache = GetSubsystem<ResourceCache>();
		renderPath->Load(cache->GetResource<XMLFile>("PostProcess/Outline.xml"));
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
	void OutlineFilter::Update(float timeStep)
	{
		alpha_ = Sin(time_ * M_PI * 2 / M_DEGTORAD) * 0.5 + 0.5;
		time_ += timeStep;
		for (HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Begin(); iter != outlineModels_.End(); ++iter)
		{
			InternalProcess(iter->first_, iter->second_);
		}
	}
	void OutlineFilter::AddOutLineModel(Node* node)
	{
		AddOutLineModel(node, outlineColor_, width_);
	}
	void OutlineFilter::AddOutLineModel(Node* node, const Color& color)
	{
		AddOutLineModel(node, color, width_);
	}
	void OutlineFilter::AddOutLineModel(Node* node, const Color& color, float width)
	{
		if (!node)
			return;
		if (width > maxWith_)
		{
			width = maxWith_;
		}
		HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Find(SharedPtr<Node>(node));
		if (iter == outlineModels_.End())
		{
			if (color.a_ == 0.0)
			{
				Color colorTemp(color.r_, color.g_, color.b_, 0.00001);
				outlineModels_.Insert(MakePair(SharedPtr<Node>(node), colorTemp));
			} else
			{
				outlineModels_.Insert(MakePair(SharedPtr<Node>(node), color));
			}
			viewPort_->GetRenderPath()->SetShaderParameter("OutlineWidth", width_);
		} else
		{
			URHO3D_LOGINFO("This node has setting outline!!");
		}
	}

	void OutlineFilter::AddOutLineModelTwinkle(Node* node)
	{
		AddOutLineModelTwinkle(node, outlineColor_, width_);
	}
	void OutlineFilter::AddOutLineModelTwinkle(Node* node, const Color& color)
	{
		AddOutLineModelTwinkle(node, color, width_);
	}
	void OutlineFilter::AddOutLineModelTwinkle(Node* node, const Color& color, float width)
	{
		if (!node)
			return;
		if (width > maxWith_)
		{
			width = maxWith_;
		}
		HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Find(SharedPtr<Node>(node));
		if (iter == outlineModels_.End())
		{
			Color colorTemp(color.r_, color.g_, color.b_, 0);
			outlineModels_.Insert(MakePair(SharedPtr<Node>(node), colorTemp));
			viewPort_->GetRenderPath()->SetShaderParameter("OutlineWidth", width_);
		} else
		{
			URHO3D_LOGINFO("This node has setting outline!!");
		}
	}
	void OutlineFilter::ClearOutlineModel(Node* node)
	{
		if (!node)
			return;

		HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Find(SharedPtr<Node>(node));
		if (iter != outlineModels_.End())
		{
			if (iter->second_.a_ > 0.)
			{
				outlineModels_.Erase(SharedPtr<Node>(node));
			}
			InternalRemove(node);
		}
	}
	void OutlineFilter::ClearOutlineModelTwinkle(Node* node)
	{
		if (!node)
			return;

		HashMap<SharedPtr<Node>, Color>::Iterator iter = outlineModels_.Find(SharedPtr<Node>(node));
		if (iter != outlineModels_.End())
		{
			if (iter->second_.a_ == 0.)
			{
				outlineModels_.Erase(SharedPtr<Node>(node));
			}
		}
		InternalRemove(node);
	}
	void OutlineFilter::InternalProcess(Node* node, const Color& color)
	{
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
					for (int j = 0; j < drawable->GetBatches().Size(); ++j)
					{
						auto* pass = drawable->AddPass(j, "outlineFill", "fill", "fill");
						if(pass)
						{
							pass->SetDepthTestMode(CompareMode::CMP_ALWAYS);
						}
					}
					if (color.a_ == 0.)
					{
						Color colorTemp(color.r_, color.g_, color.b_, alpha_);
						for (int j = 0; j < drawable->GetBatches().Size(); ++j)
						{
							drawable->GetBatches().At(j).material_->SetShaderParameter("OutlineColor", colorTemp);
						}

					} else
						for (int j = 0; j < drawable->GetBatches().Size(); ++j)
						{
							drawable->GetBatches().At(j).material_->SetShaderParameter("OutlineColor", color);
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
	void OutlineFilter::InternalRemove(Node * node)
	{
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
					for (int j = 0; j < drawable->GetBatches().Size(); ++j)
						drawable->RemovePass(j, "outlineFill");
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