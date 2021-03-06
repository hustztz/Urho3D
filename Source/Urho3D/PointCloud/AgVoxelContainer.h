#pragma once

#include "../Graphics/Drawable.h"
#include "../Math/BoundingBox.h"

#include <vector>
#include <memory>

namespace ambergris {
	namespace PointCloudEngine {

		class AgVoxelPoints;

		enum ClipFlag
		{
			ALL_CLIPPED = 0,
			NON_CLIPPED,
			PARTIAL_CLIPPED
		};

		//////////////////////////////////////////////////////////////////////////
		//\brief:  VoxelContainer 'Container' for storing pointcloud/voxel cache
		//////////////////////////////////////////////////////////////////////////
		class AgVoxelContainer : public Urho3D::Drawable
		{
			URHO3D_OBJECT(AgVoxelContainer, Urho3D::Drawable);
		public:
			/// Construct.
			explicit AgVoxelContainer(Urho3D::Context* context);
			/// Destruct.
			~AgVoxelContainer() override;
			/// Register object factory. Drawable must be registered first.
			static void RegisterObject(Urho3D::Context* context);

			/// Process octree raycast. May be called from a worker thread.
			void ProcessRayQuery(const Urho3D::RayOctreeQuery& query, Urho3D::PODVector<Urho3D::RayQueryResult>& results) override;

			/// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
			void UpdateBatches(const Urho3D::FrameInfo& frame) override;

			/// Prepare geometry for rendering.
			void UpdateGeometry(const Urho3D::FrameInfo& frame) override;

			/// Return whether a geometry update is necessary, and if it can happen in a worker thread.
			Urho3D::UpdateGeometryType GetUpdateGeometryType() override { return Urho3D::UPDATE_WORKER_THREAD; }

			std::uint32_t							getLODPointCount(std::uint8_t lod) const;

			bool									isGeometryEmpty() const {	return batches_.Empty();	}
			std::uint64_t							clearGeometry();
			std::uint64_t							getAllocatedMemory() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: load a LIDAR LOD from disk this is a
			//////////////////////////////////////////////////////////////////////////
			void                                    loadLODInternal(bool isLidarData, std::uint8_t lodLevel);

			bool                                    isComplete(unsigned LOD) const;

			// \brief:  clip status function
			bool                                    isClipFlag(ClipFlag rhs) const;
			ClipFlag                                getClipFlag() const;
			void                                    updateClipFlag();
			int                                     clipIndex() const { return m_nClipIndex; }
			void                                    setClipIndex(int nClipIndex) { m_nClipIndex = nClipIndex; }

			/// Set geometry.
			void SetVoxelPoints(AgVoxelPoints* points);
			/// Return geometry attribute.
			const Urho3D::String& GetResourceName() const { return resourceName_; }
			/// Set geometry attribute.
			void SetResourceName(const Urho3D::String& name) { resourceName_ = name; }

			/// Return layout spacing.
			char GetMaxLOD() const { return maximumLOD_; }
			/// Set layout spacing.
			void SetMaxLOD(char num) { maximumLOD_ = num; }

			/// Return layout spacing.
			int GetAmountOfPoints() const { return amountOfPoints_; }
			/// Set layout spacing.
			void SetAmountOfPoints(int num) { amountOfPoints_ = num; }

			/// Set TransformBounds Min.
			void SetSVOBoundsMin(const Urho3D::Vector3& value) {
				boundingBox_.min_ = value; OnMarkedDirty(node_);
			}
			/// Return position.
			const Urho3D::Vector3& GetSVOBoundsMin() const { return boundingBox_.min_; }

			/// Set TransformBounds Min.
			void SetSVOBoundsMax(const Urho3D::Vector3& value) {
				boundingBox_.max_ = value; OnMarkedDirty(node_);
			}
			/// Return position.
			const Urho3D::Vector3& GetSVOBoundsMax() const { return boundingBox_.max_; }

		protected:
			std::uint8_t  CalcLOD(const Urho3D::FrameInfo& frame, float pointSize);
			/// Handle a background loaded resource completing.
			void HandleResourceBackgroundLoaded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
			/// Recalculate the world-space bounding box.
			void OnWorldBoundingBoxUpdate() override;

			void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
			void HandlePostRenderUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
		private:
			Urho3D::String								resourceName_;
			std::uint8_t                                currentDrawLOD_;
			std::uint8_t                                maximumLOD_;                   //maximum LOD stored in file
			std::uint32_t                               amountOfPoints_;               //total amount of points
												//TODO: we shall check if we are allowed to change the stack sequence of class members
												//      and stack public/private member together.
		
			// \brief: clip status
			ClipFlag                                m_clipFlag;
			ClipFlag                                mExternalClipFlag;
			ClipFlag                                mInternalClipFlag;
			// \brief: the index which indicates the point number of valid clip flag
			int                                     m_nClipIndex;
		};
	}
}
