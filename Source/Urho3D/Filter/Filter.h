/*
*
* zhangzhiwei
*
*/

#pragma once

#include "../Core/Object.h"

namespace Urho3D
{
	class Viewport;

	class Filter : public Object
	{
		URHO3D_OBJECT(Filter, Object);
	public:
		Filter(Context* context, Viewport* viewPort);
		virtual ~Filter(){}
		virtual void Initial() = 0;
		bool IsInitial() { return isInitialed_; }
		virtual void SetEnable(bool enable);
		bool IsEnable() { return isEnable_; };
		virtual void Update(float timeStep){};
		virtual void PostUpdate() {};
	private:
		void HandleUpdate(StringHash eventType, VariantMap& eventData);
		void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

	protected:
		/// Rendertarget names
		//Vector<String> renderTargetNames_;
		/// Rendering command  names
		//Vector<String> commandsNames_;
		///
		//Vector<Drawable> drawables_;

		Viewport* viewPort_;
	private:
		bool isInitialed_;
		bool isEnable_;
		
	};
}