/*
*
* zhangzhiwei
*
*/

#pragma once

#include "../Core/Object.h"
#include "../Filter/Filter.h"
//#include "../../Container/Ptr.h"

namespace Urho3D
{
	class Viewport;
	class Node;
	class Camera;
	class Texture2D;

	template <class T, class U> class HashMap;

	class ProjectiveTextureMappingFilter : public Filter
	{
		URHO3D_OBJECT(ProjectiveTextureMappingFilter, Filter);
	public:
		ProjectiveTextureMappingFilter(Context* context, Viewport* viewPort);
		~ProjectiveTextureMappingFilter();
		virtual void Initial();
		void SetProjectionCamera(Camera* cam);
		void SetProjectionTexture(Texture2D* tex);
		void Update(float timeStep) override;

	private:
		SharedPtr<Camera> cam_;
		SharedPtr<Texture2D> projectiveTexture_;
	};
}