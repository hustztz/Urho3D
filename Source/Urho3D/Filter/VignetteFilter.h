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

	class VignetteFilter : public Filter
	{
		URHO3D_OBJECT(VignetteFilter, Filter);
	public:
		VignetteFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
		void SetFalloff(float falloff);//Ä¬ÈÏ0.3£¬ È¡Öµ0~1 
	};
}