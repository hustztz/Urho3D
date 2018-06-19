#include "AgLidarPoint.h"

#include <common/RCVector.h>

using namespace ambergris::RealityComputing::Common;
using namespace ambergris::PointCloudEngine;

AgLidarPoint::AgLidarPoint()
{
	m_pos[0] = m_pos[1] = m_pos[2] = 0.0f;
	m_rgba[0] = m_rgba[1] = m_rgba[2] = m_rgba[3] = 0;
	m_misc = 0;
}

RCVector3f AgLidarPoint::getRawCoord() const
{
	return RCVector3f(m_pos[0], m_pos[1], m_pos[2]);
}

void AgLidarPoint::setRawCoord(const RCVector3f& val)
{
	m_pos[0] = val.x;
	m_pos[1] = val.y;
	m_pos[2] = val.z;
}

void AgLidarPoint::setNormal(const RCVector3f& normal)
{
	int normalIndex = Math::NormalUtils::indexForNormal(normal);
	setNormalIndex(normalIndex);
}

RCVector3f AgLidarPoint::getNormal() const
{
	return Math::NormalUtils::normalForIndex(getNormalIndex());
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

void AgLidarPoint::setRGBA(const RCVector4ub& rgba)
{
	m_rgba[0] = rgba.x;
	m_rgba[1] = rgba.y;
	m_rgba[2] = rgba.z;
	m_rgba[3] = rgba.w;
}

RCVector4ub AgLidarPoint::getRGBA() const
{
	return RCVector4ub(m_rgba[0], m_rgba[1], m_rgba[2], m_rgba[3]);
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