#pragma once

#include "../Resource/Resource.h"

namespace Urho3D
{
	class Geometry;
}

namespace ambergris {
	namespace PointCloudEngine {

		/// 3D model resource.
		class URHO3D_API AgVoxelPoints : public Urho3D::Resource
		{
			URHO3D_OBJECT(AgVoxelPoints, Urho3D::Resource);
		public:
			/// Construct.
			explicit AgVoxelPoints(Urho3D::Context* context) : Urho3D::Resource(context) {}

			virtual bool isEmpty() const = 0;
			virtual std::uint64_t							clear() = 0;
			virtual std::uint32_t	getCount() const = 0;
			virtual std::uint64_t	getAllocatedMemory() const = 0;
			virtual Urho3D::Geometry* getGeometry() const = 0;
		};
	}
}