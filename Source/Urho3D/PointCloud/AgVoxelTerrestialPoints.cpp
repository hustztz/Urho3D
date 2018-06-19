#include "AgVoxelTerrestialPoints.h"

#include "../Core/Context.h"
#include "../IO/Log.h"
#include "../IO/File.h"

using namespace Urho3D;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::PointCloudEngine;

AgVoxelTerrestialPoints::AgVoxelTerrestialPoints(Context* context) :
	AgVoxelPoints(context)
{
}

AgVoxelTerrestialPoints::~AgVoxelTerrestialPoints() = default;

void AgVoxelTerrestialPoints::RegisterObject(Context* context)
{
	context->RegisterFactory<AgVoxelTerrestialPoints>();
}

bool AgVoxelTerrestialPoints::BeginLoad(Deserializer& source, unsigned offset, unsigned size)
{
	String fileID = source.ReadFileID();
	if (fileID != "PCVT")
	{
		URHO3D_LOGERROR(source.GetName() + " is not a valid point cloud file");
		return false;
	}

	return true;
}

bool AgVoxelTerrestialPoints::EndLoad()
{
	return true;
}

bool AgVoxelTerrestialPoints::isEmpty() const
{
	return m_terrestialPointList.empty();
}

std::uint64_t	AgVoxelTerrestialPoints::clear()
{
	std::uint64_t memCleared = 0ULL;
	if (m_terrestialPointList.size())
	{
		memCleared += sizeof(AgTerrestialPoint) * m_terrestialPointList.capacity();
		m_terrestialPointList.clear();
		std::vector<AgTerrestialPoint>().swap(m_terrestialPointList);
	}
	if (m_terrestialSegIdList.size()) {
		memCleared += sizeof(std::uint16_t) * m_terrestialSegIdList.capacity();
		m_terrestialSegIdList.clear();
		std::vector<std::uint16_t>().swap(m_terrestialSegIdList);
	}
	return memCleared;
}

std::uint64_t	AgVoxelTerrestialPoints::getAllocatedMemory() const
{
	return (sizeof(AgTerrestialPoint) * m_terrestialPointList.size());
}