
#include "../Filter/Filter.h"
#include "../Core/CoreEvents.h"
#include "../Math/StringHash.h"
#include "../Graphics/Viewport.h"
#include "../Graphics/RenderPath.h"

namespace Urho3D
{
	Filter::Filter(Context* context, Viewport* viewPort) :
		Object(context),
		isInitialed_(false),
		isEnable_(false),
		viewPort_(viewPort)
	{

	}

	void Filter::SetEnable(bool enable)
	{
		if (isEnable_ == enable)
			return;
		if (!IsInitial())
		{
			Initial();
			isInitialed_ = true;
		}
		isEnable_ = enable;
		if (isEnable_)
		{
			SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Filter, HandleUpdate));
			SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Filter, HandlePostUpdate));
		}
		else
		{
			UnsubscribeFromEvent(E_UPDATE);
			UnsubscribeFromEvent(E_POSTUPDATE);
		}

		String tag = GetTypeName();
		viewPort_->GetRenderPath()->SetEnabled(tag, enable);
	}

	void Filter::HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;

		// Take the frame time step, which is stored as a float
		float timeStep = eventData[P_TIMESTEP].GetFloat();

		Update(timeStep);
	}

	void Filter::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace PostUpdate;

		// Take the frame time step, which is stored as a float
		float timeStep = eventData[P_TIMESTEP].GetFloat();

		PostUpdate(timeStep);
	}
}