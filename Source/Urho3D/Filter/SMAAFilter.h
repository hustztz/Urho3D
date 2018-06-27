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

	class SMAAFilter : public Filter
	{
		URHO3D_OBJECT(SMAAFilter, Filter);
	public:
		SMAAFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
	};
}