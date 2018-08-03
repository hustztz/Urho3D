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

	class TranslucentFilter : public Filter
	{
		URHO3D_OBJECT(TranslucentFilter, Filter);
	public:
		TranslucentFilter(Context* context, Viewport* viewPort);
		~TranslucentFilter();
		virtual void Initial();
		void AddModel(Node* node);
		void ClearModel(Node* node);
	private:
		void InternalProcess(Node* node);
		void InternalRemove(Node* node);
		HashSet<SharedPtr<Node> > models_;
	};
}