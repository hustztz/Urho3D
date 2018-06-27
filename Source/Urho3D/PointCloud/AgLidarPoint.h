#pragma once
#include "../Math/Vector3.h"
#include "AgCompactColor.h"

//////////////////////////////////////////////////////////////////////////
// \brief: VoxelLidarPoint Point format for LIDAR based data
//////////////////////////////////////////////////////////////////////////
/*
VoxelLidarPoint ( 32*3 bits total )
//float #1          xyz Coordinates

//4 * UByte #2      RGBA

//UInt #3
[0....13]           Normal index
[14...21]           Lidar Classification
[22...28]           Layer info
[29]                Is in temp selection
[30]                Filtered : 1 filtered, 0 not filtered
[31]                Crop: 1 clipped, 0 not clipped
*/

namespace ambergris {
	namespace PointCloudEngine {

		struct AgLidarPoint
		{
			AgLidarPoint();

			Urho3D::Vector3				getRawCoord() const;
			void                                    setRawCoord(const Urho3D::Vector3& val);

			void                                    setNormal(const Urho3D::Vector3& normal);
			Urho3D::Vector3                      getNormal() const;

			void                                    setNormalIndex(uint32_t normalIndex);
			int                                     getNormalIndex() const;

			void                                setRGBA(const AgCompactColor& rgba);
			AgCompactColor                     getRGBA() const;

			void                                    setLidarClassification(uint8_t lidarData);
			uint8_t                            getLidarClassification() const;

			void                                    setLayerInfo(uint8_t layerData);
			uint8_t                            getLayerInfo() const;

			//uint8_t                            getInheritRegionIndex(const VoxelContainer* parentVoxelContainer) const;

			bool                                    getInTempSelection() const;
			void                                    setInTempSelection(bool val);

			void                               setFiltered(bool val);
			bool                               isFiltered() const;

			void                               setClipped(bool val);
			bool                               isClipped() const;

			//bool                               isDeleted(const VoxelContainer* parentVoxelContainer) const;

			float                              m_pos[3];
			AgCompactColor                       m_rgba;
			uint32_t                           m_misc;         //normals, lidar classification
		};
	}
}