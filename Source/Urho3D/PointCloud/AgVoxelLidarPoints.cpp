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
	geometry_ = new Geometry(context_);
}

AgVoxelLidarPoints::~AgVoxelLidarPoints() = default;

void AgVoxelLidarPoints::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelLidarPoints>();
}

bool AgVoxelLidarPoints::BeginLoad(Deserializer& source, unsigned offset, unsigned size)
{
	Urho3D::String fileID = source.ReadFileID();
	if (fileID != "PCVL")
	{
		URHO3D_LOGERROR(source.GetName() + " is not a valid point cloud file");
		return false;
	}

	const unsigned   totalPoints = source.ReadUInt();

	// Read voxel buffers
	const unsigned needPoints = std::min((offset + size), totalPoints);
	m_lidarPointList.resize(needPoints);

	for (unsigned i = offset; i < needPoints; ++i)
	{
		AgVoxelLeafNode rawPoint(source.ReadUInt64(), source.ReadUInt64());
		m_lidarPointList[i].setRawCoord(rawPoint.getRawOffsetFromBoundingBox());
		m_lidarPointList[i].setRGBA(rawPoint.getRGBA());
		m_lidarPointList[i].setNormalIndex(rawPoint.getNormal());
		m_lidarPointList[i].setLidarClassification(rawPoint.getLidarClassification());

		if (rawPoint.getDeleteFlag()) {
			m_lidarPointList[i].setFiltered(true);
			//hasDeletedPts = true;
		}
	}

	int pointSizeInBytes = sizeof(AgVoxelLeafNode)     * totalPoints;
	source.Seek(pointSizeInBytes + fileID.Length() + 4);
	const unsigned amountOfTimestamps = std::min((offset + size), source.ReadUInt());
	for (unsigned i = offset; i < amountOfTimestamps; ++i)
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
	elements.Push(VertexElement(TYPE_VECTOR3, SEM_POSITION, 0));
	elements.Push(VertexElement(TYPE_UBYTE4_NORM, SEM_COLOR, 1));
	elements.Push(VertexElement(TYPE_INT, SEM_TEXCOORD, 2));

	// Upload vertex buffer data
	SharedPtr<VertexBuffer> buffer(new VertexBuffer(context_));
	buffer->SetShadowed(false);
	buffer->SetSize(m_lidarPointList.size(), elements);
	buffer->SetData(m_lidarPointList.data());
	vertexBuffers_.Push(buffer);
	
	const unsigned existedNumVB = geometry_->GetNumVertexBuffers();
	geometry_->SetNumVertexBuffers(existedNumVB + 1);
	geometry_->SetVertexBuffer(existedNumVB, buffer);

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
	return (sizeof(AgLidarPoint) * m_lidarPointList.size());
}