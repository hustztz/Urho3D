#include "AgVoxelTreeRunTime.h"
#include "AgProjectInformation.h"
#include "AgOctreeDefinitions.h"
#include "AgVoxelLidarPoints.h"

#include "../Core/Context.h"
#include "../Resource/JSONFile.h"
#include "../Scene/SceneResolver.h"
#include "../Scene/Scene.h"

#include <common/RCMemoryHelper.h>
#include <common/RCMemoryMapFile.h>
#include <common/RCTransform.h>
#include <utility/RCSpatialReference.h>
#include <utility/RCFilesystem.h>
#include <utility/RCStringUtils.h>
#include <io/RCChecksum.h>
#include <import/OctreeIntermediateNode.h>

#include <assert.h>
#include <algorithm>

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Import;

AgVoxelTreeRunTime::AgVoxelTreeRunTime(Context* context)
	: Node(context)
	, mIsLidarData(false)
	, mHasRGB(false)
	, mHasNormals(false)
	, mHasIntensity(false)
	, m_totalAmountOfPoints(0)
{
}

AgVoxelTreeRunTime::~AgVoxelTreeRunTime()
{
}

void AgVoxelTreeRunTime::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelTreeRunTime>();

	//TODO
}

bool AgVoxelTreeRunTime::LoadJSON(const JSONValue& source, SceneResolver& resolver, bool loadChildren, bool rewriteIDs, CreateMode mode)
{
	// Remove all children and components first in case this is not a fresh load
	RemoveAllChildren();
	RemoveAllComponents();

	if (!Animatable::LoadJSON(source))
		return false;

	JSONArray transArray = source.Get("translation").GetArray();
	Vector3 translation;
	translation.x_ = transArray.At(0).GetFloat();
	translation.y_ = transArray.At(1).GetFloat();
	translation.z_ = transArray.At(2).GetFloat();
	SetPositionSilent(translation);

	JSONArray rotArray = source.Get("rotation").GetArray();
	Vector3 rotation;
	rotation.x_ = rotArray.At(0).GetFloat();
	rotation.y_ = rotArray.At(1).GetFloat();
	rotation.z_ = rotArray.At(2).GetFloat();
	SetRotationSilent(Quaternion(rotation.x_, rotation.y_, rotation.z_));

	JSONArray scaleArray = source.Get("scale").GetArray();
	Vector3 scale;
	scale.x_ = scaleArray.At(0).GetFloat();
	scale.y_ = scaleArray.At(1).GetFloat();
	scale.z_ = scaleArray.At(2).GetFloat();
	SetScaleSilent(scale);

	JSONArray originArray = source.Get("scannerOrigin").GetArray();
	m_scannerOrigin.x = originArray.At(0).GetDouble();
	m_scannerOrigin.y = originArray.At(1).GetDouble();
	m_scannerOrigin.z = originArray.At(2).GetDouble();

	JSONArray svobbArray = source.Get("svoBounds").GetArray();
	m_svoBounds.setBounds(RCVector3d(svobbArray.At(0).GetDouble(), svobbArray.At(1).GetDouble(), svobbArray.At(2).GetDouble()), 
		RCVector3d(svobbArray.At(3).GetDouble(), svobbArray.At(4).GetDouble(), svobbArray.At(5).GetDouble()));

	JSONArray scanbbArray = source.Get("scanBounds").GetArray();
	m_scanBounds.setBounds(RCVector3d(scanbbArray.At(0).GetDouble(), scanbbArray.At(1).GetDouble(), scanbbArray.At(2).GetDouble()),
		RCVector3d(scanbbArray.At(3).GetDouble(), scanbbArray.At(4).GetDouble(), scanbbArray.At(5).GetDouble()));

	m_octreeFlags = source.Get("octreeFlags").GetUInt();
	m_totalAmountOfPoints = source.Get("totalAmountOfPoints").GetUInt();
	mHasRGB = source.Get("hasRGB").GetBool();
	mHasNormals = source.Get("hasNormals").GetBool();
	mHasIntensity = source.Get("hasIntensity").GetBool();

	mIsLidarData = source.Get("isLidarData").GetBool();

	//update bounding box
	updateAll();

	const JSONArray& componentsArray = source.Get("components").GetArray();

	for (unsigned i = 0; i < componentsArray.Size(); i++)
	{
		const JSONValue& compVal = componentsArray.At(i);
		Urho3D::String typeName = compVal.Get("type").GetString();
		unsigned compID = compVal.Get("id").GetUInt();
		Component* newComponent = SafeCreateComponent(typeName, StringHash(typeName),
			(mode == REPLICATED && Scene::IsReplicatedID(compID)) ? REPLICATED : LOCAL, rewriteIDs ? 0 : compID);
		if (newComponent)
		{
			resolver.AddComponent(compID, newComponent);
			if (!newComponent->LoadJSON(compVal))
				return false;
		}
	}

	if (!loadChildren)
		return true;

	const JSONArray& childrenArray = source.Get("children").GetArray();
	for (unsigned i = 0; i < childrenArray.Size(); i++)
	{
		const JSONValue& childVal = childrenArray.At(i);

		unsigned nodeID = childVal.Get("id").GetUInt();
		Node* newNode = CreateChild(rewriteIDs ? 0 : nodeID, (mode == REPLICATED && Scene::IsReplicatedID(nodeID)) ? REPLICATED :
			LOCAL);
		resolver.AddNode(nodeID, newNode);
		if (!newNode->LoadJSON(childVal, resolver, loadChildren, rewriteIDs, mode))
			return false;
	}

	return true;
}

RCBox AgVoxelTreeRunTime::getSvoBounds() const
{
	return m_svoBounds;
}

void AgVoxelTreeRunTime::setRegionFlagInvalid()
{
	m_regionFlag &= ~(0x1 << 2);
}
void AgVoxelTreeRunTime::setRegionFlagValid()
{
	m_regionFlag |= 0x1 << 2;
}
bool AgVoxelTreeRunTime::isRegionFlagValid() const
{
	return (m_regionFlag & 0x1 << 2) != 0;
}
void AgVoxelTreeRunTime::setRegionIndex(std::uint8_t index)
{
	setRegionFlagValid();
	m_regionFlag &= ~(0x7F << 3);
	m_regionFlag |= index << 3;
}
std::uint8_t AgVoxelTreeRunTime::getRegionIndex() const
{
	return static_cast<std::uint8_t>((m_regionFlag >> 3) & 0x7F);
}
void AgVoxelTreeRunTime::setInTempSelectionFlagInvalid()
{
	m_regionFlag &= ~(0x1);
}
void AgVoxelTreeRunTime::setInTempSelectionFlagValid()
{
	m_regionFlag |= 0x1;
}
bool AgVoxelTreeRunTime::isInTempSelectionFlagValid() const
{
	return m_regionFlag & 0x1;
}
void AgVoxelTreeRunTime::setIsInTempSelection(bool flag)
{
	setInTempSelectionFlagValid();
	m_regionFlag &= ~(0x1 << 1);
	m_regionFlag |= flag << 1;
}
// \brief: only valid when 
bool AgVoxelTreeRunTime::getIsInTempSelection() const
{
	return (m_regionFlag >> 1) & 0x1;
}

bool AgVoxelTreeRunTime::isLidarData() const
{
	return mIsLidarData;
}

std::uint64_t AgVoxelTreeRunTime::getAllocatedMemory() const
{
	std::uint64_t amountOfMem = 0;

	PODVector<AgVoxelContainer*> leafNodeList;
	GetComponents<AgVoxelContainer>(leafNodeList);
	for (const auto& leafNode : leafNodeList)
	{
		amountOfMem += leafNode->getAllocatedMemory();
	}

	return amountOfMem;
}

bool AgVoxelTreeRunTime::hasTimeStamp() const
{
	if (!(m_octreeFlags & ENUM_OCTREE_FLAGS_HAS_TIME_STAMP))
		return false;

	PODVector<AgVoxelContainer*> containers;
	GetComponents<AgVoxelContainer>(containers);
	for (PODVector<AgVoxelContainer*>::Iterator i = containers.Begin(); i != containers.End(); ++i)
	{
		AgVoxelPoints* points = (*i)->GetVoxelPoints();
		if(!points)
			continue;
		AgVoxelLidarPoints* lidarPoints = dynamic_cast<AgVoxelLidarPoints*>(points);
		if (lidarPoints)
		{
			if (lidarPoints->hasTimestamp())
				return true;
		}
	}

	return false;
}

void AgVoxelTreeRunTime::updateAll()
{
	RCTransform transform;
	const Vector3& pos = GetPosition();
	transform.setTranslation(RCVector3d(pos.x_, pos.y_, pos.z_));
	//transform.setRotation();
	m_transformedBounds = m_scanBounds.getTransformed(transform);

	/*PODVector<AgVoxelContainer*> containers;
	GetComponents<AgVoxelContainer>(containers);
	for (PODVector<AgVoxelContainer*>::Iterator i = containers.Begin(); i != containers.End(); ++i)
	{
		(*i)->setTransformedSVOBound((*i)->getSVOBound().getTransformed(transform));
		(*i)->setTransformedCenter((*i)->getTransformedSVOBound().getCenter());
	}*/
}

const AgVoxelContainer*   AgVoxelTreeRunTime::getVoxelContainerAt(int index) const
{
	const Vector<SharedPtr<Component> >& components = GetComponents();
	if (index >= components.Size())
		return nullptr;
	return dynamic_cast<const AgVoxelContainer*>(components[index].Get());
}

AgVoxelContainer*   AgVoxelTreeRunTime::getVoxelContainerAt(int index)
{
	const Vector<SharedPtr<Component> >& components = GetComponents();
	if (index >= components.Size())
		return nullptr;
	return dynamic_cast<AgVoxelContainer*>(components[index].Get());
}
