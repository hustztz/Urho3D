#pragma once

#include "../Scene/Node.h"
#include "AgVoxelContainer.h"

#include <common/RCCode.h>
#include <common/RCTransform.h>

namespace ambergris {
	namespace PointCloudEngine {

		struct AgVoxelTreeNode;

		class AgVoxelTreeRunTime : public Urho3D::Node
		{
			URHO3D_OBJECT(AgVoxelTreeRunTime, Urho3D::Node);
		public:
			/// Construct.
			explicit AgVoxelTreeRunTime(Urho3D::Context* context);
			/// Destruct.
			~AgVoxelTreeRunTime() override;

			/// Register object factory. Node must be registered first.
			static void RegisterObject(Urho3D::Context* context);

			/// Load from JSON data. Return true if successful.
			bool LoadJSON(const Urho3D::JSONValue& source, Urho3D::SceneResolver& resolver, bool loadChildren = true, bool rewriteIDs = false,
				Urho3D::CreateMode mode = Urho3D::REPLICATED);

			const AgVoxelContainer*                   getVoxelContainerAt(int index) const;
			AgVoxelContainer*                   getVoxelContainerAt(int index);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Returns the SVO bounding box
			//////////////////////////////////////////////////////////////////////////
			RealityComputing::Common::RCBox			getSvoBounds() const;

			//////////////////////////////////////////////////////////////////////////
			//\brief: Returns if this scan is lidar data, and thus has a different
			//        vertex format
			//////////////////////////////////////////////////////////////////////////
			bool                                    isLidarData() const;

			//EngineSpatialFilter::FilterResult       getClipEngineFilterResult() const;
			//SDK::Interface::ARCSpatialFilter::FilterResult          getClipARCFilterResult() const;

			// \brief: set the region flag as invalid
			void                                    setRegionFlagInvalid();
			// \brief: set the region flag as valid
			void                                    setRegionFlagValid();
			// \brief: if a region flag is valid
			bool                                    isRegionFlagValid() const;
			// \brief: set the region index
			void                                    setRegionIndex(std::uint8_t index);
			// \brief: only valid when isRegionFlagValid returns true
			std::uint8_t                            getRegionIndex() const;

			// \brief: set the temp selection flag as invalid
			void                                    setInTempSelectionFlagInvalid();
			// \brief: set the temp selection flag as valid
			void                                    setInTempSelectionFlagValid();
			// \brief: if the temp selection flag is valid
			bool                                    isInTempSelectionFlagValid() const;
			// \brief: set whether inside/out side the temp selection
			void                                    setIsInTempSelection(bool flag);
			// \brief: only valid when
			bool                                    getIsInTempSelection() const;

			//////////////////////////////////////////////////////////////////////////
			//\brief: Returns the transformed bounding box
			//////////////////////////////////////////////////////////////////////////
			RealityComputing::Common::RCBox                       getTransformedBounds() const { return m_transformedBounds; }

			void                     updateAll();

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the allocated memory of this scan,
			//////////////////////////////////////////////////////////////////////////
			std::uint64_t                           getAllocatedMemory() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns if this scan has time stamp information 
			//////////////////////////////////////////////////////////////////////////
			bool                                    hasTimeStamp() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns if this scan has intensity
			//////////////////////////////////////////////////////////////////////////
			bool                                    hasIntensity() const { return mHasIntensity; }
		private:
			int                                     m_octreeFlags;
			unsigned int                            m_totalAmountOfPoints;

			RealityComputing::Common::RCVector3d             m_scannerOrigin;
			RealityComputing::Common::RCBox			         m_svoBounds;
			RealityComputing::Common::RCBox			         m_scanBounds;
			RealityComputing::Common::RCBox                  m_transformedBounds;

			bool                                    mHasRGB;
			bool                                    mHasNormals;
			bool                                    mHasIntensity;
			bool                                    mIsLidarData;

			//RealityComputing::Common::RCBox                      mScanOriginalWorldBounds;


			// \brief: unsigned short m_regionFlag
			/*
			*[0] whether in_temp_selection_flag is valid
			*[1] whether the voxel is inside temp selection or
			*    outside temp selection if in_temp_selection is valid
			*[2] whether in_region_flag is valid
			*[3...9] the region index if in region flag is valid
			*/
			unsigned short                          m_regionFlag;

			//RealityComputing::Common::RCVector4f              m_nodeColor;
		};
	}
}