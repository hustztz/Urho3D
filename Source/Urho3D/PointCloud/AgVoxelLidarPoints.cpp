#include "AgVoxelLidarPoints.h"
#include "AgOctreeDefinitions.h"

#include "../Core/Context.h"
#include "../IO/Log.h"
#include "../IO/File.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/Geometry.h"

#include <algorithm>

using namespace Urho3D;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::PointCloudEngine;

AgVoxelLidarPoints::AgVoxelLidarPoints(Context* context) :
	AgVoxelPoints(context)
{
}

AgVoxelLidarPoints::~AgVoxelLidarPoints() = default;

void AgVoxelLidarPoints::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelLidarPoints>();
}

bool AgVoxelLidarPoints::BeginLoad(Deserializer& source)
{
	Urho3D::String fileID = source.ReadFileID();
	if (fileID != "PCVL")
	{
		URHO3D_LOGERROR(source.GetName() + " is not a valid point cloud file");
		return false;
	}

	// Read offset
	Vector3 offset = source.ReadVector3();

	const unsigned   amountOfPoints = source.ReadUInt();

	// Read voxel buffers
	m_lidarPointList.resize(amountOfPoints);

	for (unsigned i = 0; i < amountOfPoints; ++i)
	{
		std::uint64_t data1 = source.ReadUInt64();
		std::uint64_t data2 = source.ReadUInt64();
		AgVoxelLeafNode rawPoint(data1, data2);
		m_lidarPointList[i].setRawCoord(rawPoint.getRawOffsetFromBoundingBox() * 0.001f + RCVector3f(offset.x_, offset.y_, offset.z_));
		m_lidarPointList[i].setRGBA(rawPoint.getRGBA());
		m_lidarPointList[i].setNormalIndex(rawPoint.getNormal());
		m_lidarPointList[i].setLidarClassification(rawPoint.getLidarClassification());

		if (rawPoint.getDeleteFlag()) {
			m_lidarPointList[i].setFiltered(true);
			//hasDeletedPts = true;
		}
	}

	m_timeStampList.resize(amountOfPoints);
	for (unsigned i = 0; i < amountOfPoints; ++i)
	{
		m_timeStampList[i] = source.ReadDouble();
	}

	return true;
}

bool AgVoxelLidarPoints::EndLoad()
{
	if (m_lidarPointList.empty())
		return true;

	PODVector<VertexElement> elements;
	elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION));
	elements.Push(VertexElement(TYPE_UBYTE4_NORM, SEM_COLOR));
	elements.Push(VertexElement(TYPE_INT, SEM_TEXCOORD));

	// Upload vertex buffer data
	if(!vertexBuffers_)
		vertexBuffers_ = new VertexBuffer(context_);
	vertexBuffers_->SetShadowed(false);
	vertexBuffers_->SetSize(m_lidarPointList.size(), elements);
	vertexBuffers_->SetData(m_lidarPointList.data());
	
	if(!geometry_)
		geometry_ = new Geometry(context_);
	geometry_->SetPrimitiveType(POINT_LIST);
	geometry_->SetVertexBuffer(0, vertexBuffers_);

	m_lidarPointList.clear();
	m_timeStampList.clear();

	return true;
}

bool AgVoxelLidarPoints::isEmpty() const
{
	return m_lidarPointList.empty();
}

std::uint64_t	AgVoxelLidarPoints::clear()
{
	std::uint64_t memCleared = 0ULL;
	if (m_lidarPointList.size())
	{
		memCleared += sizeof(AgLidarPoint) * m_lidarPointList.size();
		m_lidarPointList.clear();
		std::vector<AgLidarPoint>().swap(m_lidarPointList);
	}
	if (m_timeStampList.size())
	{
		memCleared += sizeof(double) * m_lidarPointList.size();
		m_timeStampList.clear();
		std::vector<double>().swap(m_timeStampList);
	}
	return memCleared;
}

std::uint64_t	AgVoxelLidarPoints::getAllocatedMemory() const
{
	return (sizeof(AgLidarPoint) * m_lidarPointList.size() + sizeof(double) * m_timeStampList.size());
}