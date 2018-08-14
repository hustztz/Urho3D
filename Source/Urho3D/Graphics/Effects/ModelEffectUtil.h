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
		///��͸���Ч��
		static void SetOutlinePerspect(const Viewport* viewport, Node* node);
		static void SetOutlinePerspect(const Viewport* viewport, Node* node, Color color);
		static void CancelOutlinePerspect(const Viewport* viewport, Node* node);
		static void SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node);
		static void SetOutlinePerspectTwinkle(const Viewport* viewport, Node* node, Color color);
		static void CancelOutlinePerspectTwinkle(const Viewport* viewport, Node* node);
		///��������ĵ���Ч��
		static void SetOuterElecEffect(const Viewport* viewport, Node* node);
		static void SetOuterElecEffect(const Viewport* viewport, Node* node, Color color);
		static void SetOuterElecEffect(const Viewport* viewport, Node* node, bool xDir, float entend, float speed, Color color, float sample);
		static void CancelOuterElecEffect(const Viewport* viewport, Node* node);
		///�ཻ���ָ�����˸
		static void SetCollisionEffect(const Viewport* viewport, Node* node);
		static void SetCollisionEffect(const Viewport* viewport, Node* node, Color color);
		static void CancelCollisionEffect(const Viewport* viewport);
		///ͨ���ı�ģ��diffuse��ɫ����ģ����۷���������ɫ
		static void SetDiffuse(Node* node, Color diffColor);
		static void CancelDiffuse(Node* node);
		///ģ��unshaded��ɫ
		static void SetUnshadedColor(const Viewport* viewport, Node* node, Color color);
		static void CancelUnshadedColor(const Viewport* viewport, Node* node);
		///ģ��͸��
		static void SetModelTransparent(Node* node, float alpha);
		static void CancelModelTransparent(Node* node);
		///ģ�ʹ�ɫ͸��
		static void SetModelUnshadedTransparent(Node* node, Color color);
		static void CancelModelUnshadedTransparent(Node* node);
		///�߿�wire����ʾ,�ٶ�����ֻ��FILL_WIREFRAME��FILL_SOLID������䷽ʽ��������FILL_POINTS��䷽ʽ
		static void SetWireframe(Node* node);
		static void CancelWireframe(Node* node);
		///��͸��ʾ,����ٶ�batches�ļ�������һ����
		static void SetTranslucent(const Viewport* viewport, Node* node);
		static void CancelTranslucent(const Viewport* viewport, Node* node);
		///����Ч��,ע��color��ͳ����������õ�ģ�������ʾ��ɫ�Ѿ������������ã�����hdr�Ƿ����йأ�������colorֵ�������ó���1.
		static void SetBloom(Node* node, Color color);
		static void CancelBloom(Node* node);
		///����drawable�Ķ�̬���ͣ���Ҫ���ھ�̬��Ӱ
		static void SetDynamicHint(Node* node, DynamicType dynamicHint);
		///����drawable�Ŀɼ���
		static void SetModelVisible(Node* node, bool visible);
		///������˸Ч��
		static void SetTwinkle(Node* node, float period, Color color);
		static void CancelTwinkle(Node* node);
		///��ʾ����
		static void SetNormalShowEffect(Node* node);
		static void CancelNormalShowEffect(Node* node);
	};

}
