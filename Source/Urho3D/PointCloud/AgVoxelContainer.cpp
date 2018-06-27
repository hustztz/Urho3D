#include "AgVoxelContainer.h"
#include "AgVoxelLidarPoints.h"
#include "AgVoxelTerrestialPoints.h"
#include "AgPointCloudOptions.h"

#include "../IO/Log.h"
#include "../Core/Context.h"
#include "../Core/CoreEvents.h"
#include "../Scene/Node.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Material.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"

#include <algorithm>

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;

AgVoxelContainer::AgVoxelContainer(Context* context) :
	Drawable(context, DRAWABLE_POINTCLOUD),
	m_clipFlag(NON_CLIPPED),
	mExternalClipFlag(NON_CLIPPED),
	mInternalClipFlag(NON_CLIPPED),
	m_nClipIndex(0),
	maximumLOD_(0),
	currentDrawLOD_(0)
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(AgVoxelContainer, HandleUpdate));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(AgVoxelContainer, HandlePostRenderUpdate));
}

AgVoxelContainer::~AgVoxelContainer() = default;

void AgVoxelContainer::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelContainer>();

	URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Resource Name", GetResourceName, SetResourceName, String, String::EMPTY, AM_DEFAULT);
	//URHO3D_MIXED_ACCESSOR_ATTRIBUTE("LOD Infos", GetLODInfoAttr, SetLODInfoAttr, VariantVector, Variant::emptyVariantVector, AM_DEFAULT);
	URHO3D_ACCESSOR_ATTRIBUTE("Amount Of Points", GetAmountOfPoints, SetAmountOfPoints, int, 0, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("Maximum LOD", GetMaxLOD, SetMaxLOD, int, 0, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("SVO Bounds Min", GetSVOBoundsMin, SetSVOBoundsMin, Vector3, Vector3::ZERO, AM_FILE);
	URHO3D_ACCESSOR_ATTRIBUTE("SVO Bounds Max", GetSVOBoundsMax, SetSVOBoundsMax, Vector3, Vector3::ZERO, AM_FILE);
}

uint8_t  AgVoxelContainer::CalcLOD(const FrameInfo& frame, float pointSize)
{
	const BoundingBox& worldBoundingBox = GetWorldBoundingBox();

	Vector3 voxCorners[8];
	voxCorners[0] = Vector3(worldBoundingBox.min_.x_, worldBoundingBox.min_.y_, worldBoundingBox.min_.z_);
	voxCorners[1] = Vector3(worldBoundingBox.max_.x_, worldBoundingBox.min_.y_, worldBoundingBox.min_.z_);
	voxCorners[2] = Vector3(worldBoundingBox.min_.x_, worldBoundingBox.max_.y_, worldBoundingBox.min_.z_);
	voxCorners[3] = Vector3(worldBoundingBox.max_.x_, worldBoundingBox.max_.y_, worldBoundingBox.min_.z_);

	voxCorners[4] = Vector3(worldBoundingBox.min_.x_, worldBoundingBox.min_.y_, worldBoundingBox.max_.z_);
	voxCorners[5] = Vector3(worldBoundingBox.max_.x_, worldBoundingBox.min_.y_, worldBoundingBox.max_.z_);
	voxCorners[6] = Vector3(worldBoundingBox.min_.x_, worldBoundingBox.max_.y_, worldBoundingBox.max_.z_);
	voxCorners[7] = Vector3(worldBoundingBox.max_.x_, worldBoundingBox.max_.y_, worldBoundingBox.max_.z_);
	const Frustum& frustum = frame.camera_->GetFrustum();
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
		Vector3 cornerWS = voxCorners[i];
		if (nearPlane.Distance(cornerWS) < 0)
		{
			cornerWS = nearPlane.Project(cornerWS);
		}
		else
		{
			anyBehind = true;
		}
		Vector4 cornerSS = frame.camera_->GetProjection() * frame.camera_->GetView() * Vector4(cornerWS, 1.0f);
		ndcBounds.Merge(Vector3(cornerSS.x_, cornerSS.y_, cornerSS.z_) / cornerSS.w_);
	}
	if (!anyBehind)
	{
		return 0;
	}

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

	// Convert NDC space to pixel space.
	Vector3 ndcSpan = ndcBounds.max_ - ndcBounds.min_;
	Vector2 pixBounds(ndcSpan.x_ * frame.viewSize_.x_  * 0.5,
		ndcSpan.y_ * frame.viewSize_.y_ * 0.5);
	double pixSize = std::max(pixBounds.x_, pixBounds.y_);
	if (pixSize <= 0)
	{
		return 0;
	}

	// Check for infinity
	if (pixSize > std::numeric_limits<double>::max())
	{
		return maximumLOD_;
	}

						   // Calc the actual LOD
	int lodLevel = 1;
	while (pixSize > pointSize)
	{
		pixSize *= 0.5f;
		lodLevel++;
	}
	return (std::uint8_t)lodLevel;
}

void AgVoxelContainer::UpdateBatches(const FrameInfo& frame)
{
	float pointSize = node_->GetVar(VoxelTreeRunTimeVars::VAR_POINTSIZE).GetFloat();
	if (pointSize < 1.0f)
		pointSize = 1.0f;
	std::uint8_t currentLod = CalcLOD(frame, pointSize);
	currentDrawLOD_ = std::max(currentLod, currentDrawLOD_);
}

void AgVoxelContainer::UpdateGeometry(const FrameInfo& frame)
{
	float lodScale = pow(0.5, static_cast<float>(currentDrawLOD_));
	BoundingBox svoBounds;
	svoBounds.max_ = node_->GetVar(VoxelTreeRunTimeVars::VAR_SVOBOUNDSMAX).GetVector3();
	svoBounds.min_ = node_->GetVar(VoxelTreeRunTimeVars::VAR_SVOBOUNDSMIN).GetVector3();
	svoBounds.Transform(node_->GetWorldTransform());
	float radius = svoBounds.max_.x_ - svoBounds.min_.x_;

	float pointScalePersp = 0.0f;
	float pointScaleOrtho = 1.0f;

	if (frame.camera_ ->IsOrthographic())
	{
		//camera is orthographic
		pointScalePersp = 0;
		/*const float fudgeFactor = 16;
		pointScaleOrtho = fudgeFactor * lodScale * radius * std::max(frame.viewSize_.x_ / camPlaneDistances.x, frame.viewSize_.y_ / camPlaneDistances.y);*/
	}
	else
	{
		// camera is perspective
		float fovScale = std::max(frame.viewSize_.x_ / frame.camera_->GetFov(), frame.viewSize_.y_ / frame.camera_->GetFov());
		pointScalePersp = 8 * radius * fovScale * lodScale;
		pointScaleOrtho = 0;
	}

	for (unsigned i = 0; i < batches_.Size(); i++)
	{
		batches_.At(i).material_->SetShaderParameter("PointScalePersp", pointScalePersp);
		batches_.At(i).material_->SetShaderParameter("PointScaleOrtho", pointScaleOrtho);
	}
}

std::uint32_t	AgVoxelContainer::getLODPointCount(std::uint8_t lod) const
{
	//const bool isLidarData = node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool();
	//auto* cache = GetSubsystem<ResourceCache>();
	//String resourceName = resourceName_;

	//if (lod <= m_currentLODLoaded)
	//{
	//	String fileName = resourceName + "_" + Urho3D::String(lod) + ".vxl";
	//	AgVoxelPoints* res = isLidarData ? (AgVoxelPoints*)cache->GetExistingResource<AgVoxelLidarPoints>(fileName) :
	//		cache->GetExistingResource<AgVoxelTerrestialPoints>(fileName);
	//	return res ? res->getCount() : 0;
	//}
	//else if (m_currentLODLoaded > 0)   //needs refinement reset to zero
	//{
	//	String fileName = resourceName + "_" + Urho3D::String(m_currentLODLoaded) + ".vxl";
	//	AgVoxelPoints* res = isLidarData ? (AgVoxelPoints*)cache->GetExistingResource<AgVoxelLidarPoints>(fileName) :
	//		cache->GetExistingResource<AgVoxelTerrestialPoints>(fileName);
	//	return res ? res->getCount() : 0;
	//}
	
	return 0;
}

std::uint64_t	AgVoxelContainer::getAllocatedMemory() const
{
	const bool isLidarData = node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool();
	auto* cache = GetSubsystem<ResourceCache>();

	std::uint64_t mem = 0;
	for (unsigned i = 0; i < batches_.Size(); i ++)
	{
		String fileName = resourceName_ + "_" + Urho3D::String(i) + ".vxl";
		AgVoxelPoints* res = isLidarData ? (AgVoxelPoints*)cache->GetExistingResource<AgVoxelLidarPoints>(fileName) :
			cache->GetExistingResource<AgVoxelTerrestialPoints>(fileName);
		if(!res)
			continue;
		mem += res->getAllocatedMemory();
	}

	return mem;
}

std::uint64_t	AgVoxelContainer::clearGeometry()
{
	const bool isLidarData = node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool();
	auto* cache = GetSubsystem<ResourceCache>();

	std::uint64_t mem = 0;
	for (unsigned i = 0; i < batches_.Size(); i++)
	{
		String fileName = resourceName_ + "_" + Urho3D::String(i) + ".vxl";
		AgVoxelPoints* res = isLidarData ? (AgVoxelPoints*)cache->GetExistingResource<AgVoxelLidarPoints>(fileName) :
			cache->GetExistingResource<AgVoxelTerrestialPoints>(fileName);
		if (!res)
			continue;
		mem += res->clear();
	}
	batches_.Clear();
	return mem;
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
	if (!points)
		return;

	for (int i = 0; i < batches_.Size(); i++)
	{
		if (batches_.At(i).geometry_ == points->getGeometry())
			return;
	}

	if (!node_)
	{
		URHO3D_LOGERROR("Can not set model while model component is not attached to a scene node");
		return;
	}

	// Unsubscribe from the reload event of previous model (if any), then subscribe to the new
	/*if (voxelPoints_)
		UnsubscribeFromEvent(voxelPoints_, E_RELOADFINISHED);*/

	SourceBatch srcBatch;
	srcBatch.geometry_ = points->getGeometry();
	srcBatch.worldTransform_ = node_ ? &node_->GetWorldTransform() : nullptr;

	const ResourceRef& resRef = node_->GetVar(VoxelTreeRunTimeVars::VAR_MATERIAL).GetResourceRef();
	auto* cache = GetSubsystem<ResourceCache>();
	srcBatch.material_ = cache->GetResource<Material>(resRef.name_);
	if (srcBatch.material_)
	{
		float pointSize = node_->GetVar(VoxelTreeRunTimeVars::VAR_POINTSIZE).GetFloat();
		if (pointSize < 1.0f)
			pointSize = 1.0f;
		srcBatch.material_->SetShaderParameter("PointSize", pointSize);

		//if (bFalseColor)
		{
			const Vector3& scanBoundsMax = node_->GetVar(VoxelTreeRunTimeVars::VAR_SCANBOUNDSMAX).GetVector3();
			const Vector3& scanBoundsMin = node_->GetVar(VoxelTreeRunTimeVars::VAR_SCANBOUNDSMIN).GetVector3();
			if (scanBoundsMax.z_ > scanBoundsMin.z_)
			{
				srcBatch.material_->SetShaderParameter("HeightMax", scanBoundsMax.z_);
				srcBatch.material_->SetShaderParameter("HeightMin", scanBoundsMin.z_);
			}
		}
	}
	batches_.Push(srcBatch);

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

void AgVoxelContainer::loadLODInternal(bool isLidarData, std::uint8_t lodLevel)
{
	if (resourceName_.Empty())
		return;

	if (lodLevel > maximumLOD_)
		lodLevel = maximumLOD_;

	const unsigned oldLod = batches_.Size();
	auto* cache = GetSubsystem<ResourceCache>();
	for (unsigned i = oldLod; i < (unsigned)lodLevel; i ++)
	{
		SubscribeToEvent(E_RESOURCEBACKGROUNDLOADED, URHO3D_HANDLER(AgVoxelContainer, HandleResourceBackgroundLoaded));

		String fileName = resourceName_ + "_" + Urho3D::String(i) + ".vxl";
		if(isLidarData)
			cache->BackgroundLoadResource<AgVoxelLidarPoints>(fileName, true, nullptr, id_);
		else
			cache->BackgroundLoadResource<AgVoxelTerrestialPoints>(fileName, true, nullptr, id_);
	}
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

bool AgVoxelContainer::isComplete(unsigned lod) const
{
	return lod <= batches_.Size();
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

void AgVoxelContainer::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	const bool isLidarData = node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool();
	const int oldLod = batches_.Size();
	if (!batches_.Empty() && (oldLod - (int)currentDrawLOD_ > 3))
	{
		for (unsigned i = currentDrawLOD_; i < oldLod; i++)
		{
			batches_.Pop();
			String fileName = resourceName_ + "_" + Urho3D::String(i) + ".vxl";
			auto* cache = GetSubsystem<ResourceCache>();
			if (isLidarData)
				cache->ReleaseResource<AgVoxelLidarPoints>(fileName, /*force*/true);//TODO
			else
				cache->ReleaseResource<AgVoxelTerrestialPoints>(fileName, /*force*/true);
		}
	}
	//reset
	currentDrawLOD_ = 0;
	
}

void AgVoxelContainer::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	const bool isLidarData = node_->GetVar(VoxelTreeRunTimeVars::VAR_LIDARDATA).GetBool();
	const int oldLod = batches_.Size();
	if (currentDrawLOD_ > oldLod)
	{
		loadLODInternal(isLidarData, oldLod + 1);
	}
}