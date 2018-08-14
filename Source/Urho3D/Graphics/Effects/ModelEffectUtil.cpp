#include "../../Graphics/Effects/ModelEffectUtil.h"
#include "../../Graphics/Viewport.h"
#include "../../Scene/Node.h"
#include "../../Math/Color.h"
#include "../../IO/Log.h"
#include "../../Filter/OutlineFilter.h"
#include "../../Filter/OuterElecEffectFilter.h"
#include "../../Filter/VolumeInfluenceFilter.h"
#include "../../Filter/UnshadedColorFilter.h"
#include "../../Scene/Component.h"
#include "../../Graphics/Drawable.h"
#include "../../Graphics/Material.h"
#include "../../Resource/ResourceCache.h"
#include "../../Graphics/Renderer.h"
#include "../../Filter/TranslucentFilter.h"
#include "../..//Scene/ValueAnimation.h"

namespace Urho3D
{

void ModelEffectUtil::SetOutlinePerspect(const Viewport* viewport, Node* node)
{
	if(!viewport)
		return;
	if(!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if(!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetOutlinePerspect调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->AddOutLineModel(node);
}
void ModelEffectUtil::SetOutlinePerspect(const Viewport* viewport, Node* node, Color color)
{
	if (!viewport)
		return;
	if (!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if (!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetOutlinePerspect调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->AddOutLineModel(node, color);
}
void ModelEffectUtil::CancelOutlinePerspect(const Viewport* viewport, Node* node)
{
	if (!viewport)
		return;
	if (!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if (!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#CancelOutlinePerspect调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->ClearOutlineModel(node);
}

void ModelEffectUtil::SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node)
{
	if (!viewport)
		return;
	if (!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if (!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetOutlinePerspectTwinkle调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->AddOutLineModelTwinkle(node);
}

void ModelEffectUtil::SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node, Color color)
{
	if (!viewport)
		return;
	if (!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if (!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetOutlinePerspectTwinkle调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->AddOutLineModelTwinkle(node, color);
}

void ModelEffectUtil::CancelOutlinePerspectTwinkle(const Viewport* viewport, Node* node)
{
	if (!viewport)
		return;
	if (!node)
		return;
	auto* outlineFilter = viewport->GetFilter<OutlineFilter>();
	if (!outlineFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#CancelOutlinePerspectTwinkle调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	outlineFilter->ClearOutlineModelTwinkle(node);
}

void ModelEffectUtil::SetOuterElecEffect(const Viewport* viewport, Node* node)
{
	SetOuterElecEffect(viewport, node, false, 0.01f, 1, Color::BLUE, 2.0f);
}

void ModelEffectUtil::SetOuterElecEffect(const Viewport* viewport, Node* node, Color color)
{
	SetOuterElecEffect(viewport, node, false, 0.01f, 1, color, 2.0f);
}

void ModelEffectUtil::SetOuterElecEffect(const Viewport* viewport, Node* node, bool xDir, float entend, float speed, Color color, float sample)
{
	if (!node)
		return;
	auto* outerElecFilter = viewport->GetFilter<OuterElecEffectFilter>();
	if (!outerElecFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetOuterElecEffect调用过程时，OuterElecEffectFilter还没有添加到viewport中");
		return;
	}
	outerElecFilter->AddOuterElecModel(node, xDir, entend, speed, color, sample);
}

void ModelEffectUtil::CancelOuterElecEffect(const Viewport* viewport, Node* node)
{
	if (!node)
		return;
	auto* outerElecFilter = viewport->GetFilter<OuterElecEffectFilter>();
	if (!outerElecFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#CancelOuterElecEffect调用过程时，OuterElecEffectFilter还没有添加到viewport中");
		return;
	}
	outerElecFilter->ClearOuterElecModel(node);
}

void ModelEffectUtil::SetCollisionEffect(const Viewport * viewport, Node * node)
{
	SetCollisionEffect(viewport, node, Color::RED);
}

void ModelEffectUtil::SetCollisionEffect(const Viewport * viewport, Node * node, Color color)
{
	if (!node)
		return;
	auto* volumeFilter = viewport->GetFilter<VolumeInfluenceFilter>();
	if (!volumeFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetCollisionEffect调用过程时，VolumeInfluenceFilter还没有添加到viewport中");
		return;
	}
	volumeFilter->SetVolume(node);
	volumeFilter->SetMarkColor(color);
}

void ModelEffectUtil::CancelCollisionEffect(const Viewport * viewport)
{
	auto* volumeFilter = viewport->GetFilter<VolumeInfluenceFilter>();
	if (!volumeFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#CancelCollisionEffect调用过程时，VolumeInfluenceFilter还没有添加到viewport中");
		return;
	}
	volumeFilter->SetVolume(nullptr);
}

void ModelEffectUtil::SetDiffuse(Node* node, Color diffColor)
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

				drawable->SetOverrideShaderParameter("MatDiffColor", diffColor);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetDiffuse(node->GetChild(i), diffColor);
		}
	}
}
void ModelEffectUtil::CancelDiffuse(Node * node)
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
				drawable->RemoveOverrideShaderParameter("MatDiffColor");
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelDiffuse(node->GetChild(i));
		}
	}
}

void ModelEffectUtil::SetUnshadedColor(const Viewport* viewport, Node* node, Color color)
{
	if (!node)
		return;
	auto* unshadedFilter = viewport->GetFilter<UnshadedColorFilter>();
	if (!unshadedFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetUnshadedColor调用过程时，UnshadedColorFilter还没有添加到viewport中");
		return;
	}
	unshadedFilter->AddUnshadedColorModel(node, color);
}
void ModelEffectUtil::CancelUnshadedColor(const Viewport* viewport, Node * node)
{
	if (!node)
		return;
	auto* unshadedFilter = viewport->GetFilter<UnshadedColorFilter>();
	if (!unshadedFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetUnshadedColor调用过程时，OutlineFilter还没有添加到viewport中");
		return;
	}
	unshadedFilter->ClearUnshadedColorModel(node);
}
void ModelEffectUtil::SetModelTransparent(Node * node, float alpha)
{
	if (!node)
		return;
	if (node->GetNumComponents() > 0)
	{
		auto* cache = node->GetSubsystem<ResourceCache>();
		Technique* technique = cache->GetResource<Technique>("Techniques/PBR/ModelTransparent.xml");
		const Vector<SharedPtr<Component> >& components = node->GetComponents();
		for (int i = 0; i < components.Size(); i++)
		{
			if (components[i]->GetTypeInfo()->IsTypeOf(Drawable::GetTypeStatic()))
			{
				Drawable* drawable = dynamic_cast<Drawable*>(components[i].Get());
				if (!drawable)
					continue;
				drawable->SetOverrideTechnique(technique);
				drawable->SetOverrideShaderParameter("MatDiffColor", Color(1.f, 1.f, 1.f, alpha));
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetModelTransparent(node->GetChild(i), alpha);
		}
	}
}
void ModelEffectUtil::CancelModelTransparent(Node * node)
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
				drawable->RemoveOverrideShaderParameter("MatDiffColor");
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelModelTransparent(node->GetChild(i));
		}
	}
}
void ModelEffectUtil::SetModelUnshadedTransparent(Node * node, Color color)
{
	if (!node)
		return;
	if (node->GetNumComponents() > 0)
	{
		auto* cache = node->GetSubsystem<ResourceCache>();
		Technique* technique = cache->GetResource<Technique>("Techniques/PBR/ModelUnshadedTransparent.xml");
		const Vector<SharedPtr<Component> >& components = node->GetComponents();
		for (int i = 0; i < components.Size(); i++)
		{
			if (components[i]->GetTypeInfo()->IsTypeOf(Drawable::GetTypeStatic()))
			{
				Drawable* drawable = dynamic_cast<Drawable*>(components[i].Get());
				if (!drawable)
					continue;
				drawable->SetOverrideTechnique(technique);
				drawable->SetOverrideShaderParameter("MatDiffColor", color);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetModelUnshadedTransparent(node->GetChild(i), color);
		}
	}
}
void ModelEffectUtil::CancelModelUnshadedTransparent(Node * node)
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
				drawable->RemoveOverrideShaderParameter("MatDiffColor");
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelModelUnshadedTransparent(node->GetChild(i));
		}
	}
}
void ModelEffectUtil::SetWireframe(Node * node)
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
				drawable->SetOverrideFillMode(FILL_WIREFRAME);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetWireframe(node->GetChild(i));
		}
	}
}
void ModelEffectUtil::CancelWireframe(Node * node)
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
				drawable->SetOverrideFillMode(FILL_SOLID);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelWireframe(node->GetChild(i));
		}
	}
}
void ModelEffectUtil::SetTranslucent(const Viewport* viewport, Node * node)
{
	if (!node)
		return;
	auto* translucentFilter = viewport->GetFilter<TranslucentFilter>();
	if (!translucentFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#SetTranslucent调用过程时，TranslucentFilter还没有添加到viewport中");
		return;
	}
	translucentFilter->AddModel(node);
}
void ModelEffectUtil::CancelTranslucent(const Viewport* viewport, Node * node)
{
	if (!node)
		return;
	auto* translucentFilter = viewport->GetFilter<TranslucentFilter>();
	if (!translucentFilter)
	{
		URHO3D_LOGERROR("ModelEffectUtil#CancelTranslucent调用过程时，TranslucentFilter还没有添加到viewport中");
		return;
	}
	translucentFilter->ClearModel(node);
}
void ModelEffectUtil::SetBloom(Node * node, Color color)
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
				drawable->SetOverrideShaderParameter("MatEmissiveColor", color);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetBloom(node->GetChild(i), color);
		}
	}
}
void ModelEffectUtil::CancelBloom(Node * node)
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
				drawable->RemoveOverrideShaderParameter("MatEmissiveColor");
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelBloom(node->GetChild(i));
		}
	}
}
void ModelEffectUtil::SetDynamicHint(Node * node, DynamicType dynamicHint)
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
				drawable->SetDynamicType(dynamicHint);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetDynamicHint(node->GetChild(i), dynamicHint);
		}
	}
}
void ModelEffectUtil::SetModelVisible(Node * node, bool visible)
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
				if (visible)
					drawable->CancelOverrideViewMask();
				else
					drawable->SetOverrideViewMask(0);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetModelVisible(node->GetChild(i), visible);
		}
	}
}
void ModelEffectUtil::SetTwinkle(Node * node, float period, Color color)
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
				/*for (int i = 0; i < drawable->GetBatches().Size(); ++i)
				{
					if(drawable->GetBatches()[i].material_)
					{
						if(drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor") != Variant::EMPTY)
						{
							SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(drawable->GetContext()));
							colorAnimation->SetKeyFrame(0.0f, drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor").GetColor());
							colorAnimation->SetKeyFrame(period/2.f, color);
							colorAnimation->SetKeyFrame(period, drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor").GetColor());
							drawable->GetBatches()[i].material_->SetShaderParameterAnimation("MatDiffColor", colorAnimation);
						}else
						{
							SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(drawable->GetContext()));
							colorAnimation->SetKeyFrame(0.0f, Color(1.,1.,1.,1.));
							colorAnimation->SetKeyFrame(period/2.f, color);
							colorAnimation->SetKeyFrame(period, Color(1., 1., 1., 1.));
							drawable->GetBatches()[i].material_->SetShaderParameterAnimation("MatDiffColor", colorAnimation);
						}
					}
				}*/
				if (drawable->GetBatches().Size() > 0 && drawable->GetBatches()[i].material_)
				{
					if (drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor") != Variant::EMPTY)
					{
						SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(drawable->GetContext()));
						colorAnimation->SetKeyFrame(0.0f, drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor").GetColor());
						colorAnimation->SetKeyFrame(period / 2.f, color);
						colorAnimation->SetKeyFrame(period, drawable->GetBatches()[i].material_->GetShaderParameter("MatDiffColor").GetColor());
						drawable->SetOverrideShaderParameterAnimation("MatDiffColor", colorAnimation);
					}else
					{
						SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(drawable->GetContext()));
						colorAnimation->SetKeyFrame(0.0f, Color::WHITE);
						colorAnimation->SetKeyFrame(period / 2.f, color);
						colorAnimation->SetKeyFrame(period, Color::WHITE);
						drawable->SetOverrideShaderParameterAnimation("MatDiffColor", colorAnimation);
					}
				}
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			SetTwinkle(node->GetChild(i), period, color);
		}
	}
}
void ModelEffectUtil::CancelTwinkle(Node * node)
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
//				for (int i = 0; i < drawable->GetBatches().Size(); ++i)
//				{
//					if (drawable->GetBatches()[i].material_)
//					{
//						drawable->GetBatches()[i].material_->SetShaderParameterAnimation("MatDiffColor", nullptr);
//					}
//				}
				drawable->SetOverrideShaderParameterAnimation("MatDiffColor", nullptr);
			}
		}
	}
	if (node->GetNumChildren() > 0)
	{
		for (int i = 0; i < node->GetNumChildren(); i++)
		{
			CancelTwinkle(node->GetChild(i));
		}
	}
}

void ModelEffectUtil::SetNormalShowEffect(Node* node)
{
	if (!node)
		return;
	if (node->GetNumComponents() > 0)
	{
		auto* cache = node->GetSubsystem<ResourceCache>();
		Technique* technique = cache->GetResource<Technique>("Techniques/Debug/DebugNormal.xml");
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
			SetNormalShowEffect(node->GetChild(i));
		}
	}
}
	
void ModelEffectUtil::CancelNormalShowEffect(Node * node)
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
			CancelNormalShowEffect(node->GetChild(i));
		}
	}
}
}
