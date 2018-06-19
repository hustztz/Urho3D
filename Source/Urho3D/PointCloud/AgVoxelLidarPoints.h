#pragma once

#include "AgVoxelPoints.h"
#include "AgLidarPoint.h"

#include <vector>

namespace Urho3D
{
	class Geometry;
	class Graphics;
	class VertexBuffer;
}

namespace ambergris {
	namespace PointCloudEngine {

		/// 3D model resource.
		class URHO3D_API AgVoxelLidarPoints : public AgVoxelPoints
		{
			URHO3D_OBJECT(AgVoxelLidarPoints, AgVoxelPoints);
		public:
			/// Construct.
			explicit AgVoxelLidarPoints(Urho3D::Context* context);
			/// Destruct.
			~AgVoxelLidarPoints() override;
			/// Register object factory.
			static void RegisterObject(Urho3D::Context* context);

			/// Load resource from stream. May be called from a worker thread. Return true if successful.
			bool BeginLoad(Urho3D::Deserializer& source, unsigned offset, unsigned size) override;
			/// Finish resource loading. Always called from the main thread. Return true if successful.
			bool EndLoad() override;

			bool isEmpty() const;
			std::uint64_t							clear() override;
			std::uint64_t	getAllocatedMemory() const override;

			bool hasTimestamp() const { return !m_timeStampList.empty(); }
		private:
			std::vector<AgLidarPoint>          m_lidarPointList;
			std::vector<double>                m_timeStampList;

			/// Vertex buffers.
			Urho3D::Vector<Urho3D::SharedPtr<Urho3D::VertexBuffer> > vertexBuffers_;
			/// Geometries.
			Urho3D::SharedPtr<Urho3D::Geometry> geometry_;
		};
	}
}