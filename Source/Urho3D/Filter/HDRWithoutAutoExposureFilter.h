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

	class HDRWithoutAutoExposureFilter : public Filter
	{
		URHO3D_OBJECT(HDRWithoutAutoExposureFilter, Filter);
	public:
		HDRWithoutAutoExposureFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
	};
}