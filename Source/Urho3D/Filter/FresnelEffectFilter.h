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
	template <class T, class U> class HashMap;

	class UnshadedColorFilter : public Filter
	{
		URHO3D_OBJECT(UnshadedColorFilter, Filter);
	public:
		UnshadedColorFilter(Context* context, Viewport* viewPort);
		~UnshadedColorFilter();
		virtual void Initial();
		void AddUnshadedColorModel(Node* node, Color color);
		void ClearUnshadedColorModel(Node* node);
	private:
		void InternalProcess(Node* node, Color color);
		void InternalRemove(Node* node);
		HashMap<SharedPtr<Node>, Color> unshadedColorModels_;
	};
}