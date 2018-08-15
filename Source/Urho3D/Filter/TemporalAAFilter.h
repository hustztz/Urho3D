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

	class TemporalAAFilter : public Filter
	{
		URHO3D_OBJECT(TemporalAAFilter, Filter);
	public:
		TemporalAAFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
		void Update(float timeStep) override;
		void PostUpdate() override;
	private:
		Matrix4 lastVPMatrix_;
	};
}