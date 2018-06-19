#pragma once

#include "../Graphics/Drawable.h"
#include "../Math/BoundingBox.h"

#include "AgLidarPoint.h"
#include "AgTerrestialPoint.h"
#include "AgOctreeDefinitions.h"

#include <utility/RCMutex.h>

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

			/// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
			void UpdateBatches(const Urho3D::FrameInfo& frame) override;

			std::uint32_t							getLODPointCount(std::uint8_t lod) const;

			bool									isGeometryEmpty() const;
			std::uint64_t							clearGeometry();
			std::uint64_t							getAllocatedMemory() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: load a LIDAR LOD from disk this is a
			//////////////////////////////////////////////////////////////////////////
			void                                    loadLODInternal(bool isLidarData, int lodLevel, bool lock = false);

			RealityComputing::Common::RCBox                       getSVOBound() const;

			bool                                    isComplete(int LOD) const;

			// \brief:  clip status function
			bool                                    isClipFlag(ClipFlag rhs) const;
			ClipFlag                                getClipFlag() const;
			void                                    updateClipFlag();
			int                                     clipIndex() const { return m_nClipIndex; }
			void                                    setClipIndex(int nClipIndex) { m_nClipIndex = nClipIndex; }

			/// Set geometry.
			void SetVoxelPoints(AgVoxelPoints* geometry);
			/// Return geometry.
			AgVoxelPoints* GetVoxelPoints() const { return voxelPoints_; }
			/// Return geometry attribute.
			Urho3D::ResourceRef GetVoxelPointsAttr() const;
			/// Set geometry attribute.
			void SetVoxelPointsAttr(const Urho3D::ResourceRef& value);

			/// Get wheel data attribute for serialization.
			Urho3D::VariantVector GetLODInfoAttr() const;
			/// Set wheel data attribute during loading.
			void SetLODInfoAttr(const Urho3D::VariantVector& value);

			int GetAmountOfOctreeNodes(std::uint8_t lod) const;
			int GetAmountOfLODPoints(std::uint8_t lod) const;

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
			/// Handle a background loaded resource completing.
			void HandleResourceBackgroundLoaded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
			/// Recalculate the world-space bounding box.
			void OnWorldBoundingBoxUpdate() override;
		public:
			int                                m_currentLODLoaded;             //LOD level loaded in cache
			int                                m_currentDrawLOD;               //LOD currently used for drawing

			int                                m_maximumLOD;                   //maximum LOD stored in file

			//int                              m_originalIndex;                //original index into VoxelTreeRunTime::m_originalScanContainerList;

			RealityComputing::Common::RCBox                       m_nodeBounds;                   //node bounds
			RealityComputing::Common::RCBox                       m_svoBounds;                    //voxel bounds

			struct VoxelLODInfo {
				int                                m_amountOfOctreeNodes;
				int                                m_amountOfLODPoints;
			};
			double							   m_lastTimeModified;             //last time this node was modified/seen
																			//TODO: we shall check if we are allowed to change the stack sequence of class members
		private:
			Urho3D::SharedPtr<AgVoxelPoints>			voxelPoints_;

			Urho3D::PODVector<VoxelLODInfo>				LODInfos_;
			int                                amountOfPoints_;               //total amount of points
												//TODO: we shall check if we are allowed to change the stack sequence of class members
												//      and stack public/private member together.
		
			// \brief: clip status
			ClipFlag                                m_clipFlag;
			ClipFlag                                mExternalClipFlag;
			ClipFlag                                mInternalClipFlag;
			// \brief: the index which indicates the point number of valid clip flag
			int                                     m_nClipIndex;

			int                                     m_numOldPoints;
		};
	}
}
