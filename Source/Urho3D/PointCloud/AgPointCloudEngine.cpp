#include "AgPointCloudEngine.h"
#include "AgVoxelContainer.h"

#include "../Core/Context.h"
#include "../Scene/Scene.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/Camera.h"
#include "../Graphics/View.h"

#include <common/RCMath.h>
#include <common/RCMemoryHelper.h>
#include <common/RCMemoryMapFile.h>
#include <utility/RCFilesystem.h>
#include <utility/RCTimer.h>
#include <utility/RCStringUtils.h>
#include <algorithm>
#include <assert.h>

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Utility::String;
using namespace ambergris::RealityComputing::Utility::Threading;

static int calcLOD(const RCBox& voxelBounds, const Viewport* viewport, float pointSize)
{
	if (!viewport)
		return 0;

	const Camera* camera = viewport->GetCamera();
	if (!camera)
		return 0;

	const View* view = viewport->GetView();
	if (!view)
		return 0;

	RCVector3d voxCorners[8];
	getCorners(voxelBounds, voxCorners);
	const Frustum& frustum = camera->GetFrustum();
	/*RCVector3d min = voxelBounds.getMin();
	RCVector3d max = voxelBounds.getMax();

	BoundingBox voxBounds(Vector3((float)min.x, (float)min.y, (float)min.z), Vector3((float)max.x, (float)max.y, (float)max.z));

	if (OUTSIDE == frustum.IsInside(voxBounds))
		return 0;*/

	bool anyBehind = false;
	BoundingBox ndcBounds;
	Plane nearPlane = frustum.planes_[PLANE_NEAR];
	for (int i = 0; i != 8; ++i)
	{
		Vector3 cornerWS = Vector3((float)voxCorners[i].x, (float)voxCorners[i].y, (float)voxCorners[i].z);
		if (nearPlane.Distance(cornerWS) < 0)
		{
			Vector3 cornerWS = nearPlane.Project(cornerWS);
		}
		else
		{
			anyBehind = true;
		}

		Vector3 cornerSS = camera->GetProjection() * camera->GetView() * cornerWS;
		ndcBounds.Merge(cornerSS);
	}
	if (!anyBehind)
		return 0;

	// make sure that part of the voxel is visible w.r.t the other clipping planes
	bool outside = false;
	outside |= ndcBounds.max_.x_ < -1;
	outside |= ndcBounds.max_.y_ < -1;
	outside |= ndcBounds.max_.z_ < -1;
	outside |= ndcBounds.min_.x_ > +1;
	outside |= ndcBounds.min_.y_ > +1;
	outside |= ndcBounds.min_.z_ > +1;
	if (outside)
	{
		return 0;
	}

	IntVector2 viewportSize = view->GetViewSize();
	// Convert NDC space to pixel space.
	Vector3 ndcSpan = ndcBounds.max_ - ndcBounds.min_;
	Vector2 pixBounds(ndcSpan.x_ * viewportSize.x_  * 0.5,
		ndcSpan.y_ * viewportSize.y_ * 0.5);
	double pixSize = std::max(pixBounds.x_, pixBounds.y_);
	if (pixSize <= 0)
	{
		return 0;
	}

	// Check for infinity
	if (pixSize > std::numeric_limits<double>::max())
	{
		return std::numeric_limits<int>::max();
	}

	// Calc the actual LOD
	int lodLevel = 1;
	while (pixSize > pointSize)
	{
		pixSize *= 0.5f;
		lodLevel++;
	}

	return lodLevel;
}

static bool calcLODs(const Renderer* renderer, const RCBox& voxelBounds, std::vector< LodRecord >& lodsOut, float pointSize)
{
	if (!renderer)
		return false;

	const unsigned viewports_size = renderer->GetNumViewports();
	lodsOut.resize(viewports_size);
	bool someGood = false;
	for (unsigned i = 0; i != viewports_size; ++i)
	{
		const Viewport* viewport = renderer->GetViewport(i);
		if(!viewport)
			continue;
		const int lod = calcLOD(voxelBounds, viewport, pointSize);
		someGood |= (lod > 0);
		lodsOut[i].m_LOD = (std::uint8_t) lod;
	}
	return someGood;
}

static bool calcVoxelLODs(const Renderer* renderer, const Vector<WeakPtr<Viewport> >& viewports, const AgVoxelContainer& voxel, std::vector< LodRecord >& lodsOut, float pointSize)
{
	const double svoBoundExtent = voxel.m_svoBounds.getMax().distance(voxel.m_svoBounds.getMin());
	const double nodeBoundExtent = voxel.m_nodeBounds.getMax().distance(voxel.m_nodeBounds.getMin());
	if (svoBoundExtent < Math::Constants::epsilon)
		return false;

	const double ratio = nodeBoundExtent / svoBoundExtent;
	RCBox bound = voxel.getSVOBound();
	if (ratio > 0.5)
	{
		//use the original svo bound

		if (calcLODs(renderer, bound, lodsOut, pointSize) == false)
			return false;

		for (auto& lod : lodsOut)
		{
			lod.m_LOD = std::min(lod.m_LOD, (uint8_t)(32 - 1));
			lod.m_desiredPointCount = voxel.GetAmountOfLODPoints(lod.m_LOD);
		}
	}
	else
	{
		//refine the voxel container to reach the level of node bound
		const double log2e = 1.44269504088896340736; //log2(e)
		const uint8_t refineLevel = static_cast<uint8_t>(std::log(ratio) * log2e *-1.0);

		//use node bound to compute LOD and then increase by refineLevel to estimate level w.r.t svoBound
		double radius = svoBoundExtent * 0.5;

		//in case of there is only one point inside the voxel, the radius will be zero
		if (radius < Math::Constants::epsilon)
			bound.expand((float)(radius - Math::Constants::epsilon));

		if (calcLODs(renderer, bound, lodsOut, pointSize) == false)
			return false;

		for (auto& lod : lodsOut)
		{
			lod.m_LOD += refineLevel;
			lod.m_LOD = std::min(lod.m_LOD, (uint8_t)(32 - 1));
			lod.m_desiredPointCount = voxel.GetAmountOfLODPoints(lod.m_LOD);
		}
	}

	return true;
}

static inline LodRecord getLOD(const ScanContainerID& container, int viewIndex)
{
	if (static_cast<int>(container.m_LODs.size()) > viewIndex)
		return container.m_LODs[viewIndex];
	return LodRecord();
}

static std::uint8_t findMaxLod(const std::vector< LodRecord >& lods)
{
	std::uint8_t maxLod = 0;
	for (size_t i = 0; i != lods.size(); ++i)
	{
		const std::uint8_t lod = lods[i].m_LOD;
		if (lod > maxLod)
			maxLod = lod;
	}
	return maxLod;
}

//////////////////////////////////////////////////////////////////////////
// \brief: Sort voxels based on their least recently used value
//////////////////////////////////////////////////////////////////////////
class VoxelTreeSorterLRU
{
public:
	VoxelTreeSorterLRU() {}

	bool operator() (const AgVoxelContainer* p1, const AgVoxelContainer* p2) const
	{
		return (p1->m_lastTimeModified < p2->m_lastTimeModified);
	}
};

AgPointCloudEngine::AgPointCloudEngine(Context* context)
	: Component(context)
	, mMaxPointsLoad(75)
	, mPointSize(1.0f)
	, mIgnoreClip(false)
	, mIsProjectDirty(false)
	, mCoordinateSystemHasChanged(false)
	, m_maxAllocatedMemory(1200 * 1024 * 1024)
	, m_lruFreeMemory(400)
{
}

AgPointCloudEngine::~AgPointCloudEngine() = default;

void AgPointCloudEngine::RegisterObject(Context* context)
{
	context->RegisterFactory<AgPointCloudEngine>();

}

void AgPointCloudEngine::_UpdateLODs()
{
	for (size_t j = 0; j < mVisibleNodes.size(); ++j)
	{
		ScanContainerID& testContainer = mVisibleNodes[j];
		const AgVoxelTreeRunTime* scan = getScanAt(testContainer.m_scanId);
		if (!scan)
			continue;
		const AgVoxelContainer* renderLeaf = scan->getVoxelContainerAt(testContainer.m_containerId);
		if (!renderLeaf)
			continue;

		//update num points
		for (size_t k = 0; k != testContainer.m_LODs.size(); ++k)
		{
			std::uint8_t& lodLevel = testContainer.m_LODs[k].m_LOD;
			testContainer.m_LODs[k].m_pointCount = renderLeaf->getLODPointCount(lodLevel);
		}
	}
}

int AgPointCloudEngine::_DoPerformFrustumCulling(std::vector<PointCloudInformation> &visibleTreeListOut) const
{
	std::vector< LodRecord > lodRecs;

	for (uint16_t i = 0; i < m_scanList.size(); i++)
	{
		const AgVoxelTreeRunTime *curTreePtr = getScanAt(i);
		if (!curTreePtr || !curTreePtr->IsEnabled())
			continue;

		/*if (curTreePtr->isClipFlag(ALL_CLIPPED))
			continue;*/

		const Renderer* renderer = GetSubsystem<Renderer>();
		//ToDo: testLods is enough for this purpose?
		if (calcLODs(renderer, curTreePtr->getSvoBounds(), lodRecs, mPointSize))      // !! was: getGlobalTransformedBounds()
		{
			// do not test filter here, which has been done by doCropFilterForScanAndVoxelLevel
			//ToDo: Consider if we can use scanId to replace PointCloudInformation directly
			PointCloudInformation scanInfo;
			scanInfo.m_scanId = (int)i;

			visibleTreeListOut.push_back(scanInfo);
		}
	}
	return (int)visibleTreeListOut.size();
}

int AgPointCloudEngine::_GetViewIndex(const Viewport* viewport) const
{
	for (size_t i = 0; i != mViewports.Size(); ++i)
	{
		if (mViewports[i] == viewport)
			return (int)i;
	}
	return -1;
}

std::uint32_t AgPointCloudEngine::getTotalPointCount(const Viewport* viewport, const std::vector<ScanContainerID>& voxelContainerListOut) const
{
	std::uint32_t numPoints = 0;
	const int viewId = _GetViewIndex(viewport);

	for (size_t i = 0; i < voxelContainerListOut.size(); i++)
	{
		const ScanContainerID& contId = voxelContainerListOut[i];
		const AgVoxelTreeRunTime* voxelTree = getScanAt(contId.m_scanId);
		if (!voxelTree)
			continue;
		const AgVoxelContainer* containerPTr = voxelTree->getVoxelContainerAt(contId.m_containerId);
		if (!containerPTr)
			continue;
		std::uint8_t lod = getLOD(contId, viewId).m_LOD;
		lod = std::min(lod, (uint8_t)(32 - 1));
		numPoints += containerPTr->GetAmountOfLODPoints(lod);
	}
	return numPoints;
}

int AgPointCloudEngine::_ReducePointCloudLoad(std::uint32_t amountOfPointsVisible, const Viewport* viewport, std::vector<ScanContainerID>& voxelContainerListOut)
{
	const int viewId = _GetViewIndex(viewport);
	if (viewId < 0)
		return 0;

	auto totalVisiblePointsNew = amountOfPointsVisible;
	//int reductionFactor = 0;                      //in LOD levels
	auto amountOfPointsMax = mMaxPointsLoad * 1000000;
	while (totalVisiblePointsNew > amountOfPointsMax)
	{
		totalVisiblePointsNew = 0;
		for (size_t i = 0; i < voxelContainerListOut.size(); i++)
		{
			ScanContainerID& contId = voxelContainerListOut[i];
			const AgVoxelTreeRunTime* voxelTree = getScanAt(contId.m_scanId);
			if (!voxelTree)
				continue;
			const AgVoxelContainer* containerPTr = voxelTree->getVoxelContainerAt(contId.m_containerId);
			if (!containerPTr)
				continue;

			std::uint8_t& lodLevel = contId.m_LODs[viewId].m_LOD; //fetch lod
			std::uint32_t& lodCount = contId.m_LODs[viewId].m_pointCount;
			if (lodLevel > 0)
				lodLevel--;

			totalVisiblePointsNew += containerPTr->GetAmountOfLODPoints(lodLevel);

			if (lodLevel <= containerPTr->m_currentLODLoaded)
				lodCount = containerPTr->GetAmountOfLODPoints(lodLevel);
			else if (containerPTr->m_currentLODLoaded > 0)
				lodCount = containerPTr->GetAmountOfLODPoints(containerPTr->m_currentLODLoaded);
			else
				lodCount = 0;
		}
	}
	return totalVisiblePointsNew;
}

//visible voxels after frustum culling
void    AgPointCloudEngine::_GetVisibleNodesFromScan(int scan, std::vector<ScanContainerID>& voxelContainerListOut)
{
	const double curTime = RealityComputing::Utility::Time::getTimeSinceEpochinMilliSec();
	int treeIndex = scan;
	AgVoxelTreeRunTime* curTreePtr = getScanAt(treeIndex);
	if (!curTreePtr)
		return;

	PODVector<AgVoxelContainer*> containers;
	curTreePtr->GetComponents<AgVoxelContainer>(containers);

	// Allocate an element into the list that will become the next working element.
	voxelContainerListOut.reserve(voxelContainerListOut.size() + containers.Size() + 1);
	voxelContainerListOut.resize(voxelContainerListOut.size() + 1);

	//for each voxel container
	int j = 0;
	for (PODVector<AgVoxelContainer*>::Iterator i = containers.Begin(); i != containers.End(); ++i, ++j)
	{
		AgVoxelContainer* renderLeaf = *i;

		// If the voxel is clipped, skip it
		if (!renderLeaf || renderLeaf->isClipFlag(ALL_CLIPPED))
			continue;

		// See if its LOD is not zero
		const Renderer* renderer = GetSubsystem<Renderer>();
		ScanContainerID& containerId = voxelContainerListOut.back();
		if (calcVoxelLODs(renderer, mViewports, *renderLeaf, containerId.m_LODs, mPointSize))
		{
			// do not test for spatial filters here, which has been done by doCropFilterForScanAndVoxelLevel
			if (!renderLeaf->isClipFlag(ALL_CLIPPED))
			{
				renderLeaf->m_lastTimeModified = curTime;
				//renderLeaf->m_visibleListId = (int)voxelContainerListOut.size();

				containerId.m_scanId = treeIndex;
				containerId.m_containerId = j;

				//use flip flag on the voxel container to set spatial filter result
				switch (renderLeaf->getClipFlag())
				{
				case ALL_CLIPPED:
					containerId.m_spatialFilterResult = FILTER_OUTSIDE;
					break;
				case PARTIAL_CLIPPED:
					containerId.m_spatialFilterResult = FILTER_INTERSECTS;
					break;
				case NON_CLIPPED:
					containerId.m_spatialFilterResult = FILTER_INSIDE;
					break;
				}

				//update num points
				for (size_t k = 0; k != containerId.m_LODs.size(); ++k)
				{
					auto& lodLevel = containerId.m_LODs[k].m_LOD;
					auto& lodCount = containerId.m_LODs[k].m_pointCount;

					if (lodLevel <= renderLeaf->m_currentLODLoaded)
						lodCount = renderLeaf->GetAmountOfLODPoints(lodLevel);
					else if (renderLeaf->m_currentLODLoaded > 0)   //needs refinement reset to zero
						lodCount = renderLeaf->GetAmountOfLODPoints(renderLeaf->m_currentLODLoaded);
					else
						lodCount = 0;
				}
				//add this to result list by creating a new working scan ID
				voxelContainerListOut.resize(voxelContainerListOut.size() + 1);
			}
			else
				continue;
		}
		else
		{
			continue;
		}
	}
	// remove the last element, since it was just a working value
	voxelContainerListOut.pop_back();
}

int AgPointCloudEngine::_GetVisibleNodesFromScans(const std::vector<PointCloudInformation> &visibleScanList,
	std::vector<ScanContainerID>& voxelContainerListOut)
{
	//TODO: Consider if we can replace PointCloudInformation with int
	int potentialVisiblenodes = 0;
	for (size_t i = 0; i < visibleScanList.size(); i++)
	{
		int treeIndex = visibleScanList[i].m_scanId;
		const AgVoxelTreeRunTime* curTreePtr = getScanAt(treeIndex);
		if (!curTreePtr)
			continue;

		potentialVisiblenodes += curTreePtr->GetNumComponents();
	}
	//reserve already space, for huge point clouds with many nodes, this reduces culling
	//in about half of the time
	if (potentialVisiblenodes)
		voxelContainerListOut.reserve(potentialVisiblenodes);

	for (size_t i = 0; i < visibleScanList.size(); i++)
	{
		_GetVisibleNodesFromScan(visibleScanList[i].m_scanId, voxelContainerListOut);
	}

	//reduce amount of points needed to be loaded
	if (mMaxPointsLoad > 0)
	{
		for (const auto& viewport : mViewports)
		{
			if (!viewport)
				continue;
			std::uint32_t totalPoints = getTotalPointCount(viewport, voxelContainerListOut);
			if (totalPoints > mMaxPointsLoad * 1000000)
			{
				_ReducePointCloudLoad(totalPoints, viewport, voxelContainerListOut);
			}
		}
	}

	return int(voxelContainerListOut.size());
}

void AgPointCloudEngine::_DoFrustumCullingAndLODDetermination(std::vector<ScanContainerID>& idsOut)
{
	std::vector<PointCloudInformation> visibleList;

	if (_DoPerformFrustumCulling(visibleList))
	{
		//only perform frustum culling
		if (mCullMethod == kCullMethodFrustumCullingOnly)
		{
			_GetVisibleNodesFromScans(visibleList, idsOut);
		}
		else if (mCullMethod == kCullMethodOccludersOnly)
		{
			//assumes occluder target already has textures --> ( buildOccluders() )
			/*if (mOccluderTargetPtr)
			{
			cullUsingOccluderData(visibleList, idsOut);
			}*/
		}
	}
}

bool AgPointCloudEngine::_MustVisibleNodesBeFullyUpdated()
{
	return true;//TODO: mDrawState.cameraIsMoving || mWorldIsDirty;
}

void AgPointCloudEngine::_SetPointRequestsForVisibleNodes()
{
	// TODO:mutlithread
	freeLRUCache();

	refineVoxels(mVisibleNodes);
}

void AgPointCloudEngine::updateVisibleNodes(const Viewport* viewport)
{
	if (_MustVisibleNodesBeFullyUpdated())
	{
		mVisibleNodes.clear();
		_DoFrustumCullingAndLODDetermination(mVisibleNodes);
		std::uint32_t totalPointCount = getTotalPointCount(viewport, mVisibleNodes);
		std::uint32_t totalPointCountAfterReduction = _ReducePointCloudLoad(totalPointCount, viewport, mVisibleNodes);
		_SetPointRequestsForVisibleNodes();
	}
	else
	{
		//No frustum culling, but update LODS's since new data might have loaded
		_UpdateLODs();
	}
}

void AgPointCloudEngine::updateProjectBounds()
{
	m_visibleProjectBounds.clear();
	m_projectBounds.clear();
	for (uint16_t i = 0; i < m_scanList.size(); i++)
	{
		const AgVoxelTreeRunTime* curTreePtr = getScanAt(i);
		if (!curTreePtr)
			continue;

		const auto scanBounds = curTreePtr->getTransformedBounds();
		auto boundsLength = scanBounds.getMax().distanceSqrd(scanBounds.getMin());
		if (isinf(boundsLength) || isnan(boundsLength))
			continue;
		if (curTreePtr->IsEnabled())
		{
			m_visibleProjectBounds.updateBounds(scanBounds);
		}
		m_projectBounds.updateBounds(scanBounds);
	}
}

//////////////////////////////////////////////////////////////////////////
// \brief: set to global from world transform
//////////////////////////////////////////////////////////////////////////
void AgPointCloudEngine::setToGlobalFromWorldTransform(const RCTransform& toGlobalFromWorld)
{
	mToGlobalFromWorld = toGlobalFromWorld;

	for (uint16_t i = 0; i < m_scanList.size(); i++)
	{
		AgVoxelTreeRunTime* curTreePtr = getScanAt(i);
		if (!curTreePtr)
			continue;

		curTreePtr->updateAll();
	}

	updateProjectBounds();
	//updateSpatialFilters();
}

//////////////////////////////////////////////////////////////////////////
// \brief: get to global from world transform
//////////////////////////////////////////////////////////////////////////
RCTransform AgPointCloudEngine::getToGlobalFromWorldTransform() const
{
	return mToGlobalFromWorld;
}

const AgVoxelTreeRunTime* AgPointCloudEngine::getScanAt(int index) const
{
	assert(index < m_scanList.size());
	Node* node = node_->GetChild(index);
	return node ? dynamic_cast<const AgVoxelTreeRunTime *>(node) : nullptr;
}

AgVoxelTreeRunTime* AgPointCloudEngine::getScanAt(int index)
{
	assert(index < m_scanList.size());
	Node* node = node_->GetChild(index);
	return node ? dynamic_cast<AgVoxelTreeRunTime *>(node) : nullptr;
}

//bool AgPointCloudEngine::addScan(const Urho3D::String& name)
//{
//	if (!scene_)
//	{
//		return false;
//	}
//
//	AgVoxelTreeRunTime* treePtr = nullptr;// scene_->CreateChild("PointCloud");
//	if (treePtr->GetScene() != scene_)
//		return false;
//
//	m_scanList.push_back(treePtr->GetID());
//
//	//if geoReference is set for this project, we need to also set geoReference for this new scan
//	/*if (m_geoReference.length() > 1.0e-6)
//	{
//		treePtr->setGeoReference(m_geoReference);
//		treePtr->setPosition(treePtr->getOriginalTranslation() - m_geoReference);
//	}*/
//
//	// Apply any existing UCS.
//	treePtr->updateAll();
//
//	/*m_worldPtr->finishPointLoading();
//	m_worldPtr->setWorldDirty(true);*/
//
//	m_scanList.push_back(treePtr->GetID());
//	updateProjectBounds();
//
//	// For each filter, create a transformed version for this new tree
//	//RCTransform transInv = treePtr->getTransform().getInverse(TransformType::Rigid);
//	//for (FilterMap::iterator iFilt = m_filters.begin(); iFilt != m_filters.end(); ++iFilt)
//	//{
//	//	Interface::ARCSpatialFilter* filterTrans = iFilt->first->transformFilter(transInv);
//	//	iFilt->second.push_back(filterTrans);
//	//	treePtr->addSpatialFilter(filterTrans);
//	//}
//
//	//alLimitBox *limBox = m_worldPtr->getActiveLimitBox();
//	//if (limBox)
//	//{
//	//	const std::wstring limBoxName = limBox->getNodeName();
//
//	//	limBox->fromBounds(m_worldPtr->getExpandedBoundingBox());
//	//	if (limBox->getFirstTime())
//	//	{
//	//		limBox->setFirstTime(false);
//	//	}
//	//	else
//	//	{
//	//		// Because that LimitBox::fromBounds sets the node name to "Default",
//	//		// restore its name after the fromBounds call.
//	//		limBox->setNodeName(limBoxName);
//	//	}
//	//}
//
//
//	mIsProjectDirty = true;
//
//	return true;
//}
//
//bool AgPointCloudEngine::removeScan(unsigned id)
//{
//	// find the scan by id and remove it
//	bool found = false;
//	for (uint16_t i = 0; i < m_scanList.size(); i++)
//	{
//		if (m_scanList.at(i) == id)
//		{
//			//m_worldPtr->finishPointLoading();
//			//m_worldPtr->setWorldDirty(true);
//			scene_->NodeRemoved(getScanAt(id));
//			found = true;
//			m_scanList.erase(m_scanList.begin() + i);
//			break;
//		}
//	}
//
//	if (!found)
//	{
//		return false;
//	}
//
//	updateProjectBounds();
//
//	mIsProjectDirty = true;
//
//	return true;
//}
//
//bool AgPointCloudEngine::unloadProject()
//{
//	for (auto& s : m_scanList)
//	{
//		scene_->NodeRemoved(getScanAt(s));
//	}
//	m_scanList.clear();
//
//	//Do not delete the render target, just de-alloc it's members
//	/*if (mOccluderTargetPtr)
//		mOccluderTargetPtr->Clear();*/
//
//	m_visibleProjectBounds.clear();
//	m_projectBounds.clear();
//
//	// Nuke all the transformed filters that we have created for each scan.
//	/*for (FilterMap::iterator iFilter = m_filters.begin(); iFilter != m_filters.end(); ++iFilter)
//	{
//		for (size_t iScan = 0; iScan != iFilter->second.size(); ++iScan)
//		{
//			delete iFilter->second[iScan];
//		}
//		iFilter->second.clear();
//	}*/
//
//	mCoordinateSystemHasChanged = false;
//	mIsProjectDirty = false;
//
//	// set project unit to meter
//	/*RCUnitService::setUnitType(UnitType::UTMeter);
//
//	clearDebugBounds();
//	clearDebugPoints();
//	clearDebugTriangles();
//	clearDebugQuads();*/
//
//	return true;
//}

std::uint64_t AgPointCloudEngine::getAllocatedMemory() const
{
	std::uint64_t totalMemory = 0;
	for (const auto& s : m_scanList)
	{
		const AgVoxelTreeRunTime* curTreePtr = getScanAt(s);
		if (!curTreePtr)
			continue;
		totalMemory += curTreePtr->getAllocatedMemory();
	}
	return totalMemory;
}

std::uint64_t AgPointCloudEngine::freeLRUCache()
{
	std::uint64_t currentMemUsage = getAllocatedMemory();
	if (currentMemUsage < m_maxAllocatedMemory)
		return 0ULL;

	std::vector<AgVoxelContainer*> allCointainerList;
	if (currentMemUsage > m_maxAllocatedMemory)
	{
		for (uint16_t i = 0; i < m_scanList.size(); i++)
		{
			AgVoxelTreeRunTime *curTreePtr = getScanAt(i);
			PODVector<AgVoxelContainer*> containers;
			curTreePtr->GetComponents<AgVoxelContainer>(containers);
			for (PODVector<AgVoxelContainer*>::Iterator i = containers.Begin(); i != containers.End(); ++i)
			{
				if (!(*i)->isGeometryEmpty())
				{
					allCointainerList.push_back((*i));
				}
			}
		}
	}

	std::uint64_t memCleared = 0ULL;
	std::uint64_t memFreeInBytes = m_lruFreeMemory * 1024 * 1024;//currentMemUsage - m_maxAllocatedMemory;

																	//sort the containers LRU
	std::sort(allCointainerList.begin(), allCointainerList.end(), VoxelTreeSorterLRU());
	for (size_t i = 0; i < allCointainerList.size(); i++)
	{
		AgVoxelContainer *curContainerPtr = allCointainerList[i];
		if(!curContainerPtr)
			continue;
		//RCScopedLock<RCMutex> locker(*(curContainerPtr->m_loadMutexPtr));
		memCleared += curContainerPtr->clearGeometry();

		curContainerPtr->m_currentDrawLOD = -1;
		curContainerPtr->m_currentLODLoaded = -1;

		//done
		if (memCleared >= memFreeInBytes)
			break;

	}
	return memCleared;
}

float AgPointCloudEngine::refineVoxels(std::vector<ScanContainerID>& visibleLeafNodes, int timeOutInMS, const bool& interupted, std::vector<bool>* updatedViewports)
{
	if (visibleLeafNodes.empty())
	{
		return 1.0f;
	}

	/*if (m_worldPtr->getCanDrawWorld() == false)
		return 1.0f;*/

	//stream in from disk
	double startTime = RealityComputing::Utility::Time::getTimeSinceEpochinMilliSec();
	AgVoxelTreeRunTime       *curVoxelTree = NULL;

	// TODO: Actually determine which views have changed.
	// For now, just mark them all as changed.
	if (updatedViewports != NULL)
	{
		const Renderer* renderer = GetSubsystem<Renderer>();
		updatedViewports->assign(renderer->GetNumViewports(), false);
	}

	size_t nodesCompleted;
	for (nodesCompleted = 0; nodesCompleted < visibleLeafNodes.size(); nodesCompleted++)
	{
		if (interupted)
		{
			//DeletePtr(curMemMap);
			return false;
		}

		/*if (m_worldPtr->getCanDrawWorld() == false)
			break;*/

		ScanContainerID& curInfo = visibleLeafNodes[nodesCompleted];
		int treeIndex = curInfo.m_scanId;

		int containerIndex = curInfo.m_containerId;
		int wantedLOD = findMaxLod(curInfo.m_LODs);

		AgVoxelTreeRunTime *curTreePtr = getScanAt(treeIndex);
		AgVoxelContainer*   curContainerPtr = curTreePtr->getVoxelContainerAt(containerIndex);

		//already loaded
		if (wantedLOD <= curContainerPtr->m_currentLODLoaded)
		{
			for (size_t iLod = 0; iLod != curInfo.m_LODs.size(); ++iLod)
			{
				auto& lodRec = curInfo.m_LODs[iLod];
				int lod = (wantedLOD <= lodRec.m_LOD) ? wantedLOD : lodRec.m_LOD;
				lodRec.m_pointCount = curContainerPtr->GetAmountOfLODPoints(lod);
			}
			continue;
		}

		//new tree encountered
		int oldLod = curContainerPtr->m_currentLODLoaded;

		//for each view, set update to 'true' if the voxel is currently not complete, and this needs updating.
		if (updatedViewports != NULL)
		{
			const Renderer* renderer = GetSubsystem<Renderer>();
			updatedViewports->resize(renderer->GetNumViewports(), false); // the number of views may have changed since the last loop.
			for (size_t j = 0; j < updatedViewports->size(); ++j)
			{
				const std::uint8_t lod = j < curInfo.m_LODs.size() ? curInfo.m_LODs[j].m_LOD : 0;

				// Mark the viewport as needing updating if its desired LOD was greater than the old loaded LOD
				// (we assume that the newly loaded LOD is at least as big as the desired one, or else wantedLOD above will be wrong)
				if (lod > oldLod)
				{
					(*updatedViewports)[j] = true;  //mark as need upate
				}
			}
		}

		/*if (m_worldPtr->getCanDrawWorld() == false)
			break;*/
		//acquire lock

		//curContainerPtr->m_loadMutexPtr->lock();
		curContainerPtr->loadLODInternal(curTreePtr->isLidarData(), wantedLOD, true);

		/*if (m_worldPtr->getCanDrawWorld() == false)
			break;*/

		//update amount of points
		int amountOfPoints = curContainerPtr->GetAmountOfLODPoints(wantedLOD);
		if (amountOfPoints)
		{
			for (size_t iLod = 0; iLod != curInfo.m_LODs.size(); ++iLod)
			{
				LodRecord& lodRec = curInfo.m_LODs[iLod];
				const int lodDiff = wantedLOD - lodRec.m_LOD;
				if (lodDiff <= 0)
				{
					lodRec.m_pointCount = amountOfPoints;
				}
				else
				{
					lodRec.m_pointCount = curContainerPtr->GetAmountOfLODPoints(lodRec.m_LOD);
				}
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		//unlock
		//is there a timeout?
		if (timeOutInMS > 0)
		{
			double currentTime = RealityComputing::Utility::Time::getTimeSinceEpochinMilliSec();
			if (static_cast<int>(currentTime - startTime) > timeOutInMS)
			{
				//not done loading all points yet
				return false;
			}
		}
	}

	return float(nodesCompleted) / float(visibleLeafNodes.size());
}

//void AgPointCloudEngine::evaluate(std::vector<AgDrawInfo>& pointList, AgCameraView::Handle viewId, const std::vector<ScanContainerID>& visibleLeafNodes)
//{
//	pointList.clear();
//	pointList.reserve(visibleLeafNodes.size());
//
//	std::uint32_t pointsDrawn = 0;
//	for (const auto& curInfo : visibleLeafNodes)
//	{
//		const auto& lodRec = getLOD(curInfo, viewId);
//		const int amountOfPoints = lodRec.m_pointCount;
//		AgVoxelTreeRunTime *curTreePtr = m_scanList[curInfo.m_scanId];
//		if(!curTreePtr)
//			continue;
//
//		AgVoxelContainer* pContainer = curTreePtr->getVoxelContainerAt(curInfo.m_containerId);
//		if(!pContainer)
//			continue;
//
//		int currentLod = std::min(static_cast<int>(lodRec.m_LOD), pContainer->m_maximumLOD);
//
//		pointList.push_back(AgDrawInfo(curTreePtr->m_handle, curInfo.m_containerId, amountOfPoints, currentLod));
//		pointsDrawn += amountOfPoints;
//	}
//}