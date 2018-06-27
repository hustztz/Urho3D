#pragma once

#include "../Core/Object.h"
#include "../Filter/Filter.h"

namespace Urho3D
{
	class Viewport;
	class Node;

	class EDLFilter : public Filter
	{
		URHO3D_OBJECT(EDLFilter, Filter);
	public:
		EDLFilter(Context* context, Viewport* viewPort);
		~EDLFilter();
		virtual void Initial();

		void SetPixScale(float value);
		void SetLightDir(Vector3 value);
	};
}