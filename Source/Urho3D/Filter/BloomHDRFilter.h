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

	class BloomHDRFilter : public Filter
	{
		URHO3D_OBJECT(BloomHDRFilter, Filter);
	public:
		BloomHDRFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
		void SetBloomHDRThreshold(float threshold);
	};
}