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
	class Texture2D;

	class LensFlareFilter : public Filter
	{
		URHO3D_OBJECT(LensFlareFilter, Filter);
	public:
		LensFlareFilter(Context* context, Viewport* viewPort);
		void SetEnable(bool enable) override;
		virtual void Initial();
		void LensFlareBlur(StringHash eventType, VariantMap& eventData);
	private:
		float bokehSize_;
		float threshold_;
		SharedPtr<Texture2D> bokehTex_;
	};
}