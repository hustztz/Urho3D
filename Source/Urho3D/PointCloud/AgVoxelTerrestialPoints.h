#pragma once

#include "AgVoxelPoints.h"

#include "AgTerrestialPoint.h"

#include <vector>

namespace ambergris {
	namespace PointCloudEngine {

		/// 3D model resource.
		class URHO3D_API AgVoxelTerrestialPoints : public AgVoxelPoints
		{
			URHO3D_OBJECT(AgVoxelTerrestialPoints, AgVoxelPoints);
		public:
			/// Construct.
			explicit AgVoxelTerrestialPoints(Urho3D::Context* context);
			/// Destruct.
			~AgVoxelTerrestialPoints() override;
			/// Register object factory.
			static void RegisterObject(Urho3D::Context* context);

			/// Load resource from stream. May be called from a worker thread. Return true if successful.
			bool BeginLoad(Urho3D::Deserializer& source, unsigned offset, unsigned size) override;
			/// Finish resource loading. Always called from the main thread. Return true if successful.
			bool EndLoad() override;

			bool isEmpty() const;
			std::uint64_t							clear() override;
			std::uint64_t	getAllocatedMemory() const override;
		private:
			std::vector<AgTerrestialPoint>     m_terrestialPointList;
			std::vector<std::uint16_t>         m_terrestialSegIdList;
		};
	}
}