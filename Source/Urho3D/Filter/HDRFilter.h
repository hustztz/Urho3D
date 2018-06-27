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

	class HDRFilter : public Filter
	{
		URHO3D_OBJECT(HDRFilter, Filter);
	public:
		HDRFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
	};
}