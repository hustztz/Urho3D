/*
*
* zhangzhiwei
*
*/

#pragma once

#include "../../Core/Object.h"
#include "../../Filter/Filter.h"

namespace Urho3D
{
	class Viewport;

	enum HBAOQuality
	{
		Low,
		Middle,
		High
	};
	class DeferredHBAOFilter : public Filter
	{
		URHO3D_OBJECT(DeferredHBAOFilter, Filter);
	public:
		DeferredHBAOFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
		void SetHBAOQuality(HBAOQuality quality);
		void SetHBAOIntensity(float intensity);
	};
}