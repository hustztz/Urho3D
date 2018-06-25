#pragma once

#include "../Scene/Component.h"
#include "../Math/StringHash.h"

#include "AgVoxelTreeRunTime.h"

#include <common/RCTransform.h>

namespace Urho3D {
	class Viewport;
}

namespace ambergris {
	namespace PointCloudEngine {

		//////////////////////////////////////////////////////////////////////////
		//\brief:  LodRecord Tells which LOD a container has, and what its size is
		//////////////////////////////////////////////////////////////////////////
		struct LodRecord
		{
			std::uint8_t m_LOD;
			std::uint32_t m_pointCount;
			std::uint32_t m_renderPointCount;
			std::uint32_t m_desiredPointCount;
			LodRecord() : m_LOD(0), m_pointCount(0), m_renderPointCount(0), m_desiredPointCount(0) {}
		};

		//////////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////////
		struct ScanContainerID
		{
			int                                     m_scanId;
			int                                     m_containerId;
			std::vector<LodRecord>                  m_LODs;
			int                                     m_spatialFilterResult;
		};

		//////////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////////
		struct PointCloudInformation
		{
			int                                     m_scanId;                       //scan id into PointCloudProject::m_scanList
			int                                     m_spatialFilterResult;
		};

		struct PointCloudLoadOptions
		{
			bool useFileHeaderTransform;

			PointCloudLoadOptions() : useFileHeaderTransform(true) {}
		};

		namespace VoxelTreeRunTimeVars
		{
			static const Urho3D::StringHash VAR_SCANNERORIGIN("scannerOrigin");
			static const Urho3D::StringHash VAR_OCTREEFLAGS("OctreeFlags");
			static const Urho3D::StringHash VAR_TOTALPOINTS("TotalPoints");
			static const Urho3D::StringHash VAR_HASRGB("HasRGB");
			static const Urho3D::StringHash VAR_HASNORMALS("HasNormals");
			static const Urho3D::StringHash VAR_HASINTENSITY("HasIntensity");
			static const Urho3D::StringHash VAR_LIDARDATA("IsLidarData");
			static const Urho3D::StringHash VAR_MATERIAL("Material");
			static const Urho3D::StringHash VAR_POINTSIZE("PointSize");

			static const Urho3D::StringHash VAR_SVOBOUNDSMIN("svoBoundsMin");
			static const Urho3D::StringHash VAR_SVOBOUNDSMAX("svoBoundsMax");
			static const Urho3D::StringHash VAR_SCANBOUNDSMIN("scanBoundsMin");
			static const Urho3D::StringHash VAR_SCANBOUNDSMAX("scanBoundsMax");
		};

		class AgPointCloudEngine : public Urho3D::Component
		{
			URHO3D_OBJECT(AgPointCloudEngine, Urho3D::Component);

			/// <description>
			/// Represents whether a given voxel is inside, outside, or in the edge of a filter.
			/// </description>
			enum FilterResult { FILTER_INSIDE = 0, FILTER_OUTSIDE, FILTER_INTERSECTS };
		public:

			enum POINTCLOUD_CULL_METHOD
			{
				kCullMethodFrustumCullingOnly = 0,
				kCullMethodOccludersOnly = 1,
				kCullMethodHybrid = 2       // TODO
			};

			enum POINTCLOUD_SELECTION_LEVEL
			{
				kTestProject = 0,
				kTestScan = 1,
				kTestVoxel = 2,
				kTestPoint = 3
			};

		public:
			explicit AgPointCloudEngine(Urho3D::Context* context);
			~AgPointCloudEngine() override;

			/// Register object factory. Drawable must be registered first.
			static void RegisterObject(Urho3D::Context* context);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Update bounding box of this project
			//////////////////////////////////////////////////////////////////////////
			void                            updateProjectBounds();


			void updateVisibleNodes(const Urho3D::Viewport* viewport);

			//void evaluate(std::vector<AgDrawInfo>& pointList, AgCameraView::Handle viewId, const std::vector<ScanContainerID>& visibleLeafNodes);
			//////////////////////////////////////////////////////////////////////////
			//\ brief: Returns the total amount of points visible, for this view
			//////////////////////////////////////////////////////////////////////////
			std::uint32_t      getTotalPointCount(const Urho3D::Viewport* viewport, const std::vector<ScanContainerID>& voxelContainerListOut) const;

			//////////////////////////////////////////////////////////////////////////
			//\brief: Free's memory, returns the amount of memory freed
			//////////////////////////////////////////////////////////////////////////
			std::uint64_t                           freeLRUCache();

			//////////////////////////////////////////////////////////////////////////
			//\ brief: Streams in new voxels from disk, will return the proportion of
			//              voxels that are completely loaded.
			//////////////////////////////////////////////////////////////////////////
			float                                   refineVoxels(std::vector<ScanContainerID>& visibleLeafNodes,
				int timeOutInMS = -1,
				const bool& interupted = false,
				std::vector<bool>* updatedViewports = NULL);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the allocated memory of this project file
			//////////////////////////////////////////////////////////////////////////
			std::uint64_t                   getAllocatedMemory() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Adds a new scan to this project
			//////////////////////////////////////////////////////////////////////////
			//bool				addScan(const Urho3D::String& name);

			//////////////////////////////////////////////////////////////////////////
			// \brief: remove scan file by id
			//////////////////////////////////////////////////////////////////////////
			//bool                                    removeScan(unsigned id);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Return the scan with the given index
			//////////////////////////////////////////////////////////////////////////
			const AgVoxelTreeRunTime*         getScanAt(int index) const;
			AgVoxelTreeRunTime*         getScanAt(int index);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Unloads a project
			//////////////////////////////////////////////////////////////////////////
			bool                 unloadProject();

			//////////////////////////////////////////////////////////////////////////
			// \brief: set whether the project is dirty or not, any operation changes
			//         rcp file need to call this method
			//////////////////////////////////////////////////////////////////////////
			void                                   setIsProjectDirty(bool dirty) { mIsProjectDirty = dirty; }

			//////////////////////////////////////////////////////////////////////////
			// \brief: get whether the project is dirty or not
			//////////////////////////////////////////////////////////////////////////
			bool                                   getIsProjectDirty() { return mIsProjectDirty; }

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns bounding box around all scans (visible & hidden) in the project
			//////////////////////////////////////////////////////////////////////////
			const RealityComputing::Common::RCBox& getFullBoundingBox() const { return m_projectBounds; }

			//////////////////////////////////////////////////////////////////////////
			// \brief: set to global from world transform
			//////////////////////////////////////////////////////////////////////////
			void                                    setToGlobalFromWorldTransform(const RealityComputing::Common::RCTransform& toGlobalFromWorld);

			//////////////////////////////////////////////////////////////////////////
			// \brief: get to global from world transform
			//////////////////////////////////////////////////////////////////////////
			RealityComputing::Common::RCTransform        getToGlobalFromWorldTransform() const;

			void setOffset(const Urho3D::Vector3& value) { offset_ = value; }


		protected:
			void		_UpdateLODs();
			//////////////////////////////////////////////////////////////////////////
			//\brief: Do first a frustum cull with the scans's bounding box,
			//        after that determine the LOD's for each of the voxel containers
			//////////////////////////////////////////////////////////////////////////
			void		_DoFrustumCullingAndLODDetermination(std::vector<ScanContainerID>& idsOut);
			//////////////////////////////////////////////////////////////////////////
			// \brief: Perform frustum culling to determine the visible tree's,
			//  returns amount of visible scans
			//////////////////////////////////////////////////////////////////////////
			int			_DoPerformFrustumCulling(std::vector<PointCloudInformation> &visibleListOut) const;
			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the visible voxel containers from an individual scan
			//////////////////////////////////////////////////////////////////////////
			void        _GetVisibleNodesFromScan(int scan, std::vector<ScanContainerID>& voxelContainerListOut);
			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the visible voxel containers from the scan list
			//////////////////////////////////////////////////////////////////////////
			int         _GetVisibleNodesFromScans(const std::vector<PointCloudInformation> &visibleScanList,
				std::vector<ScanContainerID>& voxelContainerListOut);
			//////////////////////////////////////////////////////////////////////////
			// \brief: Reduce the amount of points that are streamed in, until it
			// falls below 'mMaxPointsLoad' returns the new number of
			// points that will be streamed in
			//////////////////////////////////////////////////////////////////////////
			int        _ReducePointCloudLoad(std::uint32_t amountOfPointsVisible, const Urho3D::Viewport* viewport, std::vector<ScanContainerID>& voxelContainerListOut);

			int			_GetViewIndex(const Urho3D::Viewport* viewport) const;
			bool		_MustVisibleNodesBeFullyUpdated();
			void		_SetPointRequestsForVisibleNodes();
		private:
			std::vector<unsigned>				m_scanList;
			std::vector<ScanContainerID>		mVisibleNodes;

			float mPointSize;
			POINTCLOUD_CULL_METHOD              mCullMethod;
			bool								mIgnoreClip;

			Urho3D::Vector3						offset_;
			RealityComputing::Common::RCBox		m_worldBounds;
			RealityComputing::Common::RCBox      mTransformedWorldBounds;

			std::wstring                m_projectFile;
			bool                                mIsProjectDirty;

			Urho3D::Vector<Urho3D::WeakPtr<Urho3D::Viewport> >			mViewports;

			bool                                mCoordinateSystemHasChanged;

			//Maximum amount of points to be streamed in/ display at any given time, in millions
			std::uint32_t                       mMaxPointsLoad;

			std::uint64_t                       m_maxAllocatedMemory;       //default is 1024MB
			std::uint64_t                       m_lruFreeMemory;            //default is 256MB

			RealityComputing::Common::RCTransform       mToGlobalFromWorld;

			RealityComputing::Common::RCBox                   m_visibleProjectBounds;
			RealityComputing::Common::RCBox                   m_projectBounds;

		};
	}
}
