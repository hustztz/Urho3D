#include "AgLidarPoint.h"
#include "AgPointCloudNormalUtils.h"

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;

AgLidarPoint::AgLidarPoint()
{
	m_pos[0] = m_pos[1] = m_pos[2] = 0.0f;
	m_misc = 0;
}

Vector3 AgLidarPoint::getRawCoord() const
{
	return Vector3(m_pos[0], m_pos[1], m_pos[2]);
}

void AgLidarPoint::setRawCoord(const Vector3& val)
{
	m_pos[0] = val.x_;
	m_pos[1] = val.y_;
	m_pos[2] = val.z_;
}

void AgLidarPoint::setNormal(const Vector3& normal)
{
	int normalIndex = AgPointCloudNormalUtils::indexForNormal(normal);
	setNormalIndex(normalIndex);
}

Vector3 AgLidarPoint::getNormal() const
{
	return AgPointCloudNormalUtils::normalForIndex(getNormalIndex());
}

void AgLidarPoint::setNormalIndex(uint32_t normalIndex)
{
	m_misc &= ~0x3FFF;
	m_misc |= (0x3FFF & normalIndex);
}

int AgLidarPoint::getNormalIndex() const
{
	return (m_misc & 0x3FFF);
}

void AgLidarPoint::setRGBA(const AgCompactColor& rgba)
{
	m_rgba = rgba;
}

AgCompactColor AgLidarPoint::getRGBA() const
{
	return m_rgba;
}

void AgLidarPoint::setLidarClassification(uint8_t val)
{
	uint32_t lidarData = val;
	m_misc &= ~(0xFF << 14);          //clear layer bits
	m_misc |= (lidarData << 14);       //set layer bits
}

void AgLidarPoint::setFiltered(bool val)
{
	if (val)
		m_misc |= (0x01 << 30);
	else
		m_misc &= ~(0x01 << 30);
}

bool AgLidarPoint::isFiltered() const
{
	return bool((m_misc >> 30) & 0x01);
}

void AgLidarPoint::setClipped(bool val)
{
	if (val)
		m_misc |= (0x01u << 31);
	else
		m_misc &= ~(0x01u << 31);
}

bool AgLidarPoint::isClipped() const
{
	return bool((m_misc >> 31) & 0x01);
}

/*bool AgVoxelLidarPoint::isDeleted(const VoxelContainer* parentVoxelContainer) const
{
	uint8_t layerInfo = getInheritRegionIndex(parentVoxelContainer);
	return (alPointSelect::SELECTION_AND_DELETE_POINTS == layerInfo);
}*/

uint8_t AgLidarPoint::getLidarClassification() const
{
	return uint8_t((m_misc >> 14) & 0xFF);
}

void AgLidarPoint::setLayerInfo(uint8_t val)
{
	uint32_t layerData = val;
	m_misc &= ~(0x7F << 22);          //clear layer bits
	m_misc |= (layerData << 22);      //set layer bits
}

bool AgLidarPoint::getInTempSelection() const
{
	return uint8_t((m_misc >> 29) & 0x1);
}

void AgLidarPoint::setInTempSelection(bool val)
{
	m_misc &= ~(0x1 << 29);           //clear 

	if (val)
		m_misc |= (0x1 << 29);        //set 
	else
		m_misc |= (0x0 << 29);        //set 
}

uint8_t AgLidarPoint::getLayerInfo() const
{
	return uint8_t((m_misc >> 22) & 0x7F);
}

/*uint8_t AgVoxelLidarPoint::getInheritRegionIndex(const VoxelContainer* parentVoxelContainer) const
{
	if (parentVoxelContainer->isInheritRegionFlagValid())
	{
		return parentVoxelContainer->getInheritRegionIndex();
	}
	else
	{
		return getLayerInfo();
	}
}*/