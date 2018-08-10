#pragma once

#include "Sample.h"
#include "Global.h"
#include "GeometryTile.h"
#include "TileNodeManager.h"

namespace GIS {

	class GISApp : public Sample {
	public:

		GISApp(Context *context);

		void Start() override final;

	private:

		void CreateScene();

		void SetupViewport();

		void MoveCamera(float timeStep);

		void HandleUpdate(StringHash eventType, VariantMap& eventData);

		void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);

		void SubscribeToEvents();

	private:

		Global global_;

		bool m_Debug = false;

		uint32_t m_Show = 0;

		std::unique_ptr<TileNodeManager> m_TileNodeManager;

	};

}