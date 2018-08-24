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

	class DepthOfFieldFilter : public Filter
	{
		URHO3D_OBJECT(DepthOfFieldFilter, Filter);
	public:
		DepthOfFieldFilter(Context* context, Viewport* viewPort);
		void Initial() override;
		void Update(float timeStep) override;
		void SetMaxCoc(float maxCoc);
		void SetFocusDistance(float focusDistance);
		void SetFocalLength(float focalLength);
		void SetFNumber(float fNumber);
	private:
		float CalculateFocusDistance();
		float CalculateFocalLength();
		
		float focusDistance_; //�������
		float focalLength_; //����
		float fNumber_; //��Ȧ
		const float kFilmHeight_;

	};
}