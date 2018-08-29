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

	enum HBAOQualityTemporal
	{
		LowTemporal,
		MiddleTemporal,
		HighTemporal
	};
	class DeferredHBAOTemporalFilter : public Filter
	{
		URHO3D_OBJECT(DeferredHBAOTemporalFilter, Filter);
	public:
		DeferredHBAOTemporalFilter(Context* context, Viewport* viewPort);
		virtual void Initial();
		void SetHBAOQuality(HBAOQualityTemporal quality);
		void SetHBAOIntensity(float intensity);
		void SetAORadius(float radius);
		void SetAONumDir(float num);
		void SetAONumSteps(float num);
		void SetAOAttenuation(float attenuation);
		void SetAOAngleBias(float angleBias);
		void SetBilateralBlurRadius(float radius);
		void Update(float timeStep);
		void PostUpdate();
	private:
		Matrix4 lastVPMatrix_;
	};
}