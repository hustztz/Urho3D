#pragma once
#include "../Graphics/Drawable.h"

namespace Urho3D
{
	class Texture2D;
	class URHO3D_API BillboardDrawable : public Drawable
	{
		URHO3D_OBJECT(BillboardDrawable, Drawable)

	public:
		/// 构造函数
		explicit BillboardDrawable(Context* context);
		/// 析构函数
		~BillboardDrawable() override;
		/// Register object factory. Drawable must be registered first.
		static void RegisterObject(Context* context);
		/// 设置默认贴图
		void SetDefaultTexture(Texture2D* tex);
		/// 设置Hover贴图
		void SetHoverTexture(Texture2D* tex);
		/// 设置淡化的近距离
		void SetFadeNearDistance(float distance);
		/// 设置淡化的远距离
		void SetFadeFarDistance(float distance);
		/// 设置billboard大小
		void SetSize(Vector2 size);
		/// 设置是否前置
		void SetOnTop(bool enable);
		/// 同步billboard到node的worldPosition的相对位置
		void BindModel(Node* node, Vector3 bindedOffset);
		/// Handle node transform being dirtied.
		void OnMarkedDirty(Node* node) override;
		/// 是否可见
		void SetVisible(bool enable);
		bool IsVisible() const { return visible_;}
	protected:
		explicit BillboardDrawable(Context* context, unsigned char);
		void OnWorldBoundingBoxUpdate() override;
		void SetMaterial(Material* mat);
		void CreateQuad();

		float fadeNearDistance_;
		float fadeFarDistance_;
		Vector2 size_;
		bool visible_;
		Vector3 bindedOffset_;
		Node* bindedModel_;
		SharedPtr<Texture2D> defaultTexture_;
		SharedPtr<Texture2D> hoverTexture_;
		bool onTop_;
	};
	class URHO3D_API BillboardGUIDrawable : public BillboardDrawable
	{
		URHO3D_OBJECT(BillboardGUIDrawable, BillboardDrawable)
	public:
		explicit BillboardGUIDrawable(Context* context);
		~BillboardGUIDrawable();
		/// Register object factory. Drawable must be registered first.
		static void RegisterObject(Context* context);
		/// 设置在屏幕上的偏移
		void SetScreenOffset(Vector2 screenOffset);
		/// 设置投影后在Z方向的偏移
		void SetZOffset(float zOffset);
	protected:
		Vector2 screenOffset_;
		float zOffset_;
		bool hoverEnlarge_;
	};
}