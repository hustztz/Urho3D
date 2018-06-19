#include "AgVoxelContainer.h"
#include "AgVoxelLidarPoints.h"
#include "AgVoxelTerrestialPoints.h"
#include "AgOctreeDefinitions.h"
#include "AgPointCloudEngine.h"

#include "../IO/Log.h"
#include "../Core/Context.h"
#include "../Graphics/Camera.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"

using namespace Urho3D;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::PointCloudEngine;

AgVoxelContainer::AgVoxelContainer(Context* context) :
	Drawable(context, DRAWABLE_POINTCLOUD),
	m_clipFlag(NON_CLIPPED),
	mExternalClipFlag(NON_CLIPPED),
	mInternalClipFlag(NON_CLIPPED),
	m_nClipIndex(0),
	m_currentLODLoaded(-1),
	m_currentDrawLOD(0),
	m_maximumLOD(0),
	m_lastTimeModified(0),
	m_numOldPoints(0)
{
}

AgVoxelContainer::~AgVoxelContainer() = default;

void AgVoxelContainer::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelContainer>();

	URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("VoxelPoints", GetVoxelPointsAttr, SetVoxelPointsAttr, ResourceRef, ResourceRef(AgVoxelPoints::GetTypeStatic()), AM_DEFAULT);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("LOD Infos", GetLODInfoAttr, SetLODInfoAttr, VariantVector, Variant::emptyVariantVector, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Amount Of Points", GetAmountOfPoints, SetAmountOfPoints, int, 0, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("SVO Bounds Min", GetSVOBoundsMin, SetSVOBoundsMin, Vector3, Vector3::ZERO, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("SVO Bounds Max", GetSVOBoundsMax, SetSVOBoundsMax, Vector3, Vector3::ZERO, AM_FILE);
}

void AgVoxelContainer::UpdateBatches(const FrameInfo& frame)
{
	const BoundingBox& worldBoundingBox = GetWorldBoundingBox();
	distance_ = frame.camera_->GetDistance(worldBoundingBox.Center());

	if (batches_.Size() == 1)
		batches_[0].distance_ = distance_;
	else
	{
		const Matrix3x4& worldTransform = node_->GetWorldTransform();
		for (unsigned i = 0; i < batches_.Size(); ++i)
			batches_[i].distance_ = frame.camera_->GetDistance(worldTransform * geometryData_[i].center_);
	}

	float scale = worldBoundingBox.Size().DotProduct(DOT_SCALE);
	float newLodDistance = frame.camera_->GetLodDistance(distance_, scale, lodBias_);

	if (newLodDistance != lodDistance_)
	{
		lodDistance_ = newLodDistance;
		CalculateLodLevels();
	}
}

std::uint32_t	AgVoxelContainer::getLODPointCount(std::uint8_t lod) const
{
	if (lod <= m_currentLODLoaded)
		return LODInfos_.At(lod).m_amountOfLODPoints;
	else if (m_currentLODLoaded > 0)   //needs refinement reset to zero
		return LODInfos_.At(m_currentLODLoaded).m_amountOfLODPoints;
	
	return 0;
}

bool AgVoxelContainer::isGeometryEmpty() const
{
	return voxelPoints_ || voxelPoints_->isEmpty();
}

std::uint64_t	AgVoxelContainer::clearGeometry()
{
	if (!voxelPoints_)
		return 0;
	
	setClipIndex(0);
	return voxelPoints_->clear();
}

std::uint64_t	AgVoxelContainer::getAllocatedMemory() const
{
	if (!voxelPoints_)
		return 0;

	return voxelPoints_->getAllocatedMemory();
}


bool AgVoxelContainer::isClipFlag(ClipFlag rhs) const
{
	return m_clipFlag == rhs;
}
ClipFlag AgVoxelContainer::getClipFlag() const
{
	return m_clipFlag;
}

ambergris::PointCloudEngine::ClipFlag intersectClipFlag(
	const ambergris::PointCloudEngine::ClipFlag& lhs,
	const ambergris::PointCloudEngine::ClipFlag& rhs)
{
	if (lhs == ALL_CLIPPED || rhs == ALL_CLIPPED)
	{
		return ALL_CLIPPED;
	}
	if (lhs == PARTIAL_CLIPPED || rhs == PARTIAL_CLIPPED)
	{
		return PARTIAL_CLIPPED;
	}
	return NON_CLIPPED;
}

void AgVoxelContainer::updateClipFlag()
{
	m_clipFlag = intersectClipFlag(mInternalClipFlag, mExternalClipFlag);
}

void AgVoxelContainer::OnWorldBoundingBoxUpdate()
{
	worldBoundingBox_ = boundingBox_.Transformed(node_->GetWorldTransform());
}

void AgVoxelContainer::SetVoxelPoints(AgVoxelPoints* points)
{
	if (points == voxelPoints_)
		return;

	if (!node_)
	{
		URHO3D_LOGERROR("Can not set model while model component is not attached to a scene node");
		return;
	}

	// Unsubscribe from the reload event of previous model (if any), then subscribe to the new
	if (voxelPoints_)
		UnsubscribeFromEvent(voxelPoints_, E_RELOADFINISHED);

	voxelPoints_ = points;

	//if (geometry)
	//{
	//	SubscribeToEvent(geometry, E_RELOADFINISHED, URHO3D_HANDLER(AgVoxelContainer, HandleModelReloadFinished));

	//	// Copy the subgeometry & LOD level structure
	//	SetNumGeometries(model->GetNumGeometries());
	//	const Vector<Vector<SharedPtr<Geometry> > >& geometries = model->GetGeometries();
	//	const PODVector<Vector3>& geometryCenters = model->GetGeometryCenters();
	//	const Matrix3x4* worldTransform = node_ ? &node_->GetWorldTransform() : nullptr;
	//	for (unsigned i = 0; i < geometries.Size(); ++i)
	//	{
	//		batches_[i].worldTransform_ = worldTransform;
	//		geometryData_[i].center_ = geometryCenters[i];
	//	}

	//	SetBoundingBox(model->GetBoundingBox());
	//	ResetLodLevels();
	//}
	//else
	//{
	//	SetNumGeometries(0);
	//	SetBoundingBox(BoundingBox());
	//}

	MarkNetworkUpdate();
}

ResourceRef AgVoxelContainer::GetVoxelPointsAttr() const
{
	return GetResourceRef(GetVoxelPoints(), node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool() ? AgVoxelLidarPoints::GetTypeStatic() : AgVoxelTerrestialPoints::GetTypeStatic());
}

void AgVoxelContainer::SetVoxelPointsAttr(const ResourceRef& value)
{
	//SubscribeToEvent(E_RESOURCEBACKGROUNDLOADED, URHO3D_HANDLER(AgVoxelContainer, HandleResourceBackgroundLoaded));

	auto* cache = GetSubsystem<ResourceCache>();
	if (node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool())
		SetVoxelPoints(cache->GetResourceWithoutLoad<AgVoxelLidarPoints>(value.name_));
	else
		SetVoxelPoints(cache->GetResourceWithoutLoad<AgVoxelTerrestialPoints>(value.name_));
}

int AgVoxelContainer::GetAmountOfOctreeNodes(std::uint8_t lod) const
{
	return LODInfos_.At(lod).m_amountOfOctreeNodes;
}

int AgVoxelContainer::GetAmountOfLODPoints(std::uint8_t lod) const
{
	return LODInfos_.At(lod).m_amountOfLODPoints;
}

/// Get wheel data attribute for serialization.
VariantVector AgVoxelContainer::GetLODInfoAttr() const
{
	VariantVector ret;

	ret.Reserve(LODInfos_.Size() * 2);
	for (unsigned i = 0; i < LODInfos_.Size(); i++)
	{
		ret.Push(LODInfos_.At(i).m_amountOfOctreeNodes);
		ret.Push(LODInfos_.At(i).m_amountOfLODPoints);
	}
	return ret;
}

/// Set wheel data attribute during loading.
void AgVoxelContainer::SetLODInfoAttr(const VariantVector& value)
{
	LODInfos_.Clear();
	for (unsigned i = 0; i < value.Size() / 2; i ++)
	{
		VoxelLODInfo lod;
		lod.m_amountOfOctreeNodes = value.At(2 * i).GetInt();
		lod.m_amountOfLODPoints = value.At(2 * i + 1).GetInt();
		LODInfos_.Push(lod);
	}
}

void AgVoxelContainer::loadLODInternal(bool isLidarData, int lodLevel, bool lock)
{
	int amountOfPoints = GetAmountOfLODPoints(lodLevel);
	if (amountOfPoints <= 0)
		return;
	//only stream in additional points
	int oldPoints = 0;
	int startLod = 0;
	if (m_currentLODLoaded > 0) //only load in additional data, not starting from beginning!
	{
		oldPoints = GetAmountOfLODPoints(m_currentLODLoaded);
		startLod = m_currentLODLoaded;
	}
	m_numOldPoints = oldPoints;

	//new points to be streamed in
	int pointsToBeStreamedIn = amountOfPoints - oldPoints;
	if (pointsToBeStreamedIn <= 0)
		return;

	if (!voxelPoints_)
		return;

	auto* cache = GetSubsystem<ResourceCache>();
	cache->BackgroundLoadPatchResource<AgVoxelPoints>(voxelPoints_->GetName(), oldPoints, pointsToBeStreamedIn, true, nullptr, id_);
}

void AgVoxelContainer::HandleResourceBackgroundLoaded(StringHash eventType, VariantMap& eventData)
{
	using namespace ResourceBackgroundLoaded;

	AgVoxelPoints* voxelPoints = dynamic_cast<AgVoxelPoints*>(eventData[P_RESOURCE].GetPtr());
	if (voxelPoints)
	{
		if (eventData[P_OWNERID].GetUInt() == id_)
		{
			SetVoxelPoints(voxelPoints);
			UnsubscribeFromEvent(E_RESOURCEBACKGROUNDLOADED);
		}
	}
}

/*RCVector3d AgVoxelContainer::getTransformedCenter() const
{
	RCVector3d center(m_transformedCenter);
	if (m_parentTreePtr != NULL)
	{
		RCSpatialReference* pTransformation = m_parentTreePtr->getMentorTransformation();
		if (pTransformation != NULL)
		{
			center = m_parentTreePtr->getWorld()->globalToWorld(center);

			pTransformation->transform(center);

			center = m_parentTreePtr->getWorld()->worldToGlobal(center);
		}
	}

	return center;
}*/

/*RCBox AgVoxelContainer::getMentorTransformedSVOBound() const
{
	RCBox svoBound = getTransformedSVOBound();

	RCSpatialReference* mentorTrans = m_parentTreePtr->getMentorTransformation();
	if (mentorTrans != NULL && m_parentTreePtr->isLidarData())
	{
		svoBound = m_parentTreePtr->getWorld()->globalToWorld(svoBound);

		svoBound = mentorTrans->transformBounds(svoBound);

		svoBound = m_parentTreePtr->getWorld()->worldToGlobal(svoBound);
	}

	return svoBound;
}*/

RCBox AgVoxelContainer::getSVOBound() const
{
	return m_svoBounds;
}

/*double AgVoxelContainer::getFinestBoundLength()
{
	double svoBoundLength = m_svoBounds.getMax().x - m_svoBounds.getMin().x;
	double minLeafNodeDist = svoBoundLength / pow(2.0, m_maximumLOD - 1);
	return minLeafNodeDist;
}

std::vector<int> AgVoxelContainer::getVisibleNodeIndices()
{
	RCEngineState* worldPtr = m_parentTreePtr->getWorld();
	alPointSelectionManager *selManger = worldPtr->getPointSelectionManager();
	PointRegionManager*      regionManager = selManger->getPointRegionManager();

	std::vector<int> allVisibleNodeIndices;
	if (!m_parentTreePtr->isLidarData())
	{
		for (size_t i = 0; i < m_terrestialPointList.size(); i++)
		{
			AgVoxelTerrestialPoint terrestialPoint = m_terrestialPointList.at(i);
			// continue if the point is filtered
			if (terrestialPoint.isClipped() || terrestialPoint.isFiltered() || terrestialPoint.isDeleted(this))
			{
				continue;
			}

			std::uint8_t layerIndex = terrestialPoint.getInheritRegionIndex(this);
			if (layerIndex > 0)
			{
				alLayer* layer = regionManager->getLayerAt(layerIndex - 1);
				// continue if the layer is invisible
				if (layer != NULL && !layer->isLayerRemoved() && !layer->getLayerVisible())
				{
					continue;
				}
			}

			allVisibleNodeIndices.push_back((int)i);
		}
	}
	else
	{
		for (size_t i = 0; i < m_lidarPointList.size(); i++)
		{
			AgVoxelLidarPoint lidarPoint = m_lidarPointList.at(i);
			// continue if the point is filtered
			if (lidarPoint.isClipped() || lidarPoint.isFiltered() || lidarPoint.isDeleted(this))
			{
				continue;
			}

			std::uint8_t layerIndex = lidarPoint.getLayerInfo();
			if (layerIndex > 0)
			{
				alLayer* layer = regionManager->getLayerAt(layerIndex - 1);
				// continue if the layer is invisible
				if (layer != NULL && !layer->isLayerRemoved() && !layer->getLayerVisible())
				{
					continue;
				}
			}

			allVisibleNodeIndices.push_back((int)i);
		}
	}

	return allVisibleNodeIndices;
}*/

bool AgVoxelContainer::isComplete(int LOD) const
{
	return GetAmountOfLODPoints(LOD) <= GetAmountOfLODPoints(m_currentLODLoaded);
}

//bool AgVoxelContainer::LoadJSON(const JSONValue& source)
//{
//	JSONValue containerValue = source.Get("container");
//
//	JSONArray svobbArray = source.Get("svoBounds").GetArray();
//	m_svoBounds.setBounds(RCVector3d(svobbArray.At(0).GetDouble(), svobbArray.At(1).GetDouble(), svobbArray.At(2).GetDouble()),
//		RCVector3d(svobbArray.At(3).GetDouble(), svobbArray.At(4).GetDouble(), svobbArray.At(5).GetDouble()));
//
//	JSONArray nodebbArray = source.Get("nodeBounds").GetArray();
//	m_nodeBounds.setBounds(RCVector3d(nodebbArray.At(0).GetDouble(), nodebbArray.At(1).GetDouble(), nodebbArray.At(2).GetDouble()),
//		RCVector3d(nodebbArray.At(3).GetDouble(), nodebbArray.At(4).GetDouble(), nodebbArray.At(5).GetDouble()));
//
//	amountOfPoints_ = source.Get("amountOfPoints").GetInt();
//	m_maximumLOD = source.Get("maxDepth").GetInt();
//
//	int cumLeafPoints = 0;
//	int cumNodePoints = 0;
//	JSONArray nonLeafNodeArray = source.Get("numNonLeafNodes").GetArray();
//	JSONArray leafNodeArray = source.Get("numLeafNodes").GetArray();
//
//	for (int j = 0; j < 32; j++)
//	{
//		//cumulative stuff
//		cumLeafPoints += leafNodeArray.At(j).GetInt();
//		cumNodePoints += leafNodeArray.At(j).GetInt() + nonLeafNodeArray.At(j).GetInt();
//
//		m_amountOfOctreeNodes[j] = leafNodeArray.At(j).GetInt() + nonLeafNodeArray.At(j).GetInt();
//		m_amountOfLODPoints[j] = nonLeafNodeArray.At(j).GetInt() + cumLeafPoints;
//	}
//
//	//suppress all the nodes below maxDepth level
//	//but don't change the offset information in rcs file
//	//note: runTimeLod->m_amountofPoints cannot be changed, otherwise segmentinfo and time stamp fetch will get wrong
//	for (int jLevel = m_maximumLOD; jLevel < 32; ++jLevel)
//	{
//		m_amountOfOctreeNodes[jLevel] = 0;
//		m_amountOfLODPoints[jLevel] = m_amountOfLODPoints[m_maximumLOD - 1];
//	}
//
//
//	Urho3D::String resourceName = source.Get("resource").GetString();
//	AgVoxelTreeRunTime* voxTree = dynamic_cast<AgVoxelTreeRunTime*>(node_);
//	if (voxTree)
//	{
//		auto* cache = GetSubsystem<ResourceCache>();
//		if (voxTree->isLidarData())
//			SetVoxelPoints(cache->GetResourceWithoutLoad<AgVoxelLidarPoints>(resourceName));
//		else
//			SetVoxelPoints(cache->GetResourceWithoutLoad<AgVoxelTerrestialPoints>(resourceName));
//	}
//
//	return true;
//}