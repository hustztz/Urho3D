/*
*
* zhangzhiwei
*
*/

#pragma once

#include "../Core/Object.h"
#include "../Filter/Filter.h"

namespace Urho3D
{
	class Viewport;
	class Node;

	class OutlineFilter : public Filter
	{
		URHO3D_OBJECT(OutlineFilter, Filter);
	public:
		OutlineFilter(Context* context, Viewport* viewPort);
		~OutlineFilter();
		virtual void Initial();
		virtual void Update(float timeStep);
		void AddOutLineModel(Node* node);
		void AddOutLineModel(Node* node, const Color& color);
		void AddOutLineModel(Node* node, const Color& color, float width);
		void AddOutLineModelTwinkle(Node* node);
		void AddOutLineModelTwinkle(Node* node, const Color& color);
		void AddOutLineModelTwinkle(Node* node, const Color& color, float width);
		void ClearOutlineModel(Node* node);
		void ClearOutlineModelTwinkle(Node* node);
	private:
		void InternalProcess(Node* node, const Color& color);
		void InternalRemove(Node* node);
		Color outlineColor_;
		float width_;
		HashMap<SharedPtr<Node>, Color> outlineModels_;
		float alpha_;
		float time_;
		float maxWith_;
	};
}