/*
*
* zhangzhiwei
*
*/

#pragma once

#include "../Core/Object.h"
#include "../Filter/Filter.h"
#include "../Container/HashSet.h"

namespace Urho3D
{
	class Viewport;
	class Node;

	class OuterElecEffectFilter : public Filter
	{
		URHO3D_OBJECT(OuterElecEffectFilter, Filter);
	public:
		OuterElecEffectFilter(Context* context, Viewport* viewPort);
		~OuterElecEffectFilter();
		virtual void Initial();
		void AddOuterElecModel(Node* node, bool xDir, float entend, float speed, Color color, float sample);
		void ClearOuterElecModel(Node* node);
	private:
		void InternalProcess(Node* node, bool xDir, float entend, float speed, Color color, float sample);
		void InternalRemove(Node* node);
		HashSet<SharedPtr<Node> > outerElecModels_;
	};
}