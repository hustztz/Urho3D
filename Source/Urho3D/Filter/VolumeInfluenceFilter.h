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

	class VolumeInfluenceFilter : public Filter
	{
		URHO3D_OBJECT(VolumeInfluenceFilter, Filter);
	public:
		VolumeInfluenceFilter(Context* context, Viewport* viewPort);
		~VolumeInfluenceFilter();
		virtual void Initial() override;
		virtual void Update(float timeStep) override;
		void SetVolume(Node* node);
		void SetMarkColor(Color color) { markColor_ = color; }
		Color GetMarkColor(Color color) { return markColor_; }
	private:
		void InternalProcess(Node* node);
		void InternalRemove(Node* node);
		SharedPtr<Node> volume_;
		Color markColor_;
	};
}