#pragma once

namespace Urho3D
{
	class Viewport;
	class Node;
	class Color;
	enum DynamicType;
	class ModelEffectUtil
	{
	public:
		///穿透描边效果
		static void SetOutlinePerspect(const Viewport* viewport, Node* node);
		static void SetOutlinePerspect(const Viewport* viewport, Node* node, Color color);
		static void CancelOutlinePerspect(const Viewport* viewport, Node* node);
		static void SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node);
		static void SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node, Color color);
		static void CancelOutlinePerspectTwinkle(const Viewport* viewport, Node* node);
		///电线外面的电流效果
		static void SetOuterElecEffect(const Viewport* viewport, Node* node);
		static void SetOuterElecEffect(const Viewport* viewport, Node* node, Color color);
		static void SetOuterElecEffect(const Viewport* viewport, Node* node, bool xDir, float entend, float speed, Color color, float sample);
		static void CancelOuterElecEffect(const Viewport* viewport, Node* node);
		///相交部分高亮闪烁
		static void SetCollisionEffect(const Viewport* viewport, Node* node);
		static void SetCollisionEffect(const Viewport* viewport, Node* node, Color color);
		static void CancelCollisionEffect(const Viewport* viewport);
		///通过改变模型diffuse颜色，是模型外观泛出这种颜色
		static void SetDiffuse(Node* node, Color diffColor);
		static void CancelDiffuse(Node* node);
		///模型unshaded着色
		static void SetUnshadedColor(const Viewport* viewport, Node* node, Color color);
		static void CancelUnshadedColor(const Viewport* viewport, Node* node);
		///模型透明
		static void SetModelTransparent(Node* node, float alpha);
		static void CancelModelTransparent(Node* node);
		///模型纯色透明
		static void SetModelUnshadedTransparent(Node* node, Color color);
		static void CancelModelUnshadedTransparent(Node* node);
		///线框（wire）显示,假定这里只有FILL_WIREFRAME和FILL_SOLID两种填充方式，不考虑FILL_POINTS填充方式
		static void SetWireframe(Node* node);
		static void CancelWireframe(Node* node);
		///穿透显示,这里假定batches的技术都是一样的
		static void SetTranslucent(const Viewport* viewport, Node* node);
		static void CancelTranslucent(const Viewport* viewport, Node* node);
		///发光效果,注意color会和场景光线作用的模型外观显示颜色已经叠加来起作用，并和hdr是否开启有关，开启后color值可以设置超过1.
		static void SetBloom(Node* node, Color color);
		static void CancelBloom(Node* node);
		///设置drawable的动态类型，主要用于静态阴影
		static void SetDynamicHint(Node* node, DynamicType dynamicHint);
		///设置drawable的可见性
		static void SetModelVisible(Node* node, bool visible);
		///设置闪烁效果
		static void SetTwinkle(Node* node, float period, Color color);
		static void CancelTwinkle(Node* node);
		///显示法线
		static void SetNormalShowEffect(Node* node);
		static void CancelNormalShowEffect(Node* node);
	};

}
