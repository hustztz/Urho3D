#pragma once
#include "../Graphics/Drawable.h"

namespace Urho3D
{
	class Texture2D;
	class URHO3D_API BillboardDrawable : public Drawable
	{
		URHO3D_OBJECT(BillboardDrawable, Drawable)

	public:
		/// ���캯��
		explicit BillboardDrawable(Context* context);
		/// ��������
		~BillboardDrawable() override;
		/// Register object factory. Drawable must be registered first.
		static void RegisterObject(Context* context);
		/// ����Ĭ����ͼ
		void SetDefaultTexture(Texture2D* tex);
		/// ����Hover��ͼ
		void SetHoverTexture(Texture2D* tex);
		/// ���õ����Ľ�����
		void SetFadeNearDistance(float distance);
		/// ���õ�����Զ����
		void SetFadeFarDistance(float distance);
		/// ����billboard��С
		void SetSize(Vector2 size);
		/// �����Ƿ�ǰ��
		void SetOnTop(bool enable);
		/// ͬ��billboard��node��worldPosition�����λ��
		void BindModel(Node* node, Vector3 bindedOffset);
		/// Handle node transform being dirtied.
		void OnMarkedDirty(Node* node) override;
		/// �Ƿ�ɼ�
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
		/// ��������Ļ�ϵ�ƫ��
		void SetScreenOffset(Vector2 screenOffset);
		/// ����ͶӰ����Z�����ƫ��
		void SetZOffset(float zOffset);
	protected:
		Vector2 screenOffset_;
		float zOffset_;
		bool hoverEnlarge_;
	};
}