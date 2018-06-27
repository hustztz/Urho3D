#include "AgTerrestialPoint.h"
#include "AgPointCloudNormalUtils.h"

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;


AgTerrestialPoint::AgTerrestialPoint()
{
	m_data[0] = m_data[1] = m_data[2] = 0;
}

Vector3 AgTerrestialPoint::getRawCoord() const
{
	int x, y, z;
	x = m_data[0] & 0xFFF;
	y = (m_data[0] >> 12) & 0xFFF;
	z = (m_data[1]) & 0xFFF;

	Vector3 coord;
	coord.x_ = static_cast<float>(x);
	coord.y_ = static_cast<float>(y);
	coord.z_ = static_cast<float>(z);

	return coord;
}

void AgTerrestialPoint::setRawCoord(const Vector3& val)
{
	uint32_t offsetX = (uint32_t)val.x_;
	uint32_t offsetY = (uint32_t)val.y_;
	uint32_t offsetZ = (uint32_t)val.z_;

	m_data[0] &= ~(0xFFF);        //clear x-coord bits
	m_data[0] |= offsetX;           //x coord bits

	m_data[0] &= ~(0xFFF << 12);  //clear y-coord bits
	m_data[0] |= (offsetY << 12); //y coord bits

	m_data[1] &= ~(0xFFF);        //clear z-coord bits
	m_data[1] |= offsetZ;           //z coord bits
}

AgCompactColor AgTerrestialPoint::getRGBA() const
{
	AgCompactColor rgba;

	rgba.r_ = (m_data[2] >> 24) & 0xFF;
	rgba.g_ = (m_data[2] >> 16) & 0xFF;
	rgba.b_ = (m_data[2] >> 8) & 0xFF;
	rgba.a_ = (m_data[2] >> 0) & 0xFF;

	return rgba;
}

void AgTerrestialPoint::setRGBA(const AgCompactColor& val)
{
	m_data[2] = 0;      //reset
	m_data[2] |= (val.r_ << 24);
	m_data[2] |= (val.g_ << 16);
	m_data[2] |= (val.b_ << 8);
	m_data[2] |= (val.a_ << 0);

}

Vector3 AgTerrestialPoint::getNormal() const
{
	return AgPointCloudNormalUtils::normalForIndex((m_data[1] >> 12) & 0x3FFF);
}

void AgTerrestialPoint::setNormalIndex(uint32_t normalIndex)
{
	m_data[1] &= ~(0x3FFF << 12);         //clear normal bits
	m_data[1] |= (0x3FFF & normalIndex) << 12;    //set normal bits
}


int AgTerrestialPoint::getNormalIndex() const
{
	return ((m_data[1] >> 12) & 0x3FFF);
}

void AgTerrestialPoint::setNormal(const Vector3& normal)
{
	uint32_t index = AgPointCloudNormalUtils::indexForNormal(normal);
	m_data[1] &= ~(0x3FFF << 12);         //clear normal bits
	m_data[1] |= (0x3FFF & index) << 12;  //set normal bits
}

void AgTerrestialPoint::setLidarClassification(uint8_t/* lidarData*/)
{

}

uint8_t AgTerrestialPoint::getLidarClassification() const
{
	return 0;
}

uint8_t AgTerrestialPoint::getLayerInfo() const
{
	return ((m_data[0] >> 24) & 0x7F);
}

/*uint8_t AgVoxelTerrestialPoint::getInheritRegionIndex(const VoxelContainer* parentVoxelContainer) const
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

void AgTerrestialPoint::setLayerInfo(uint8_t val)
{

	uint32_t layerInfo = val;
	m_data[0] &= ~(0x7F << 24);           //clear layer bits
	m_data[0] |= (layerInfo << 24);       //set layer bits

}

/*bool AgVoxelTerrestialPoint::isDeleted(const VoxelContainer* parentVoxelContainer) const
{
	uint8_t layerInfo = getInheritRegionIndex(parentVoxelContainer);
	return (alPointSelect::SELECTION_AND_DELETE_POINTS == layerInfo);
}*/

bool AgTerrestialPoint::getInTempSelection() const
{
	return ((m_data[0] >> 31) & 0x1);
}

void AgTerrestialPoint::setInTempSelection(bool val)
{
	m_data[0] &= ~(0x1u << 31);            //clear

	if (val)
		m_data[0] |= (0x1u << 31);     //set
	else
		m_data[0] |= (0x0 << 31);     //set 
}

bool AgTerrestialPoint::isFiltered() const
{
	return bool((m_data[1] >> 26) & 0x01);
}

void AgTerrestialPoint::setFiltered(bool val)
{
	if (val)
		m_data[1] |= (0x01 << 26);
	else
	{
		m_data[1] &= ~(0x01 << 26);
	}
}

bool AgTerrestialPoint::isClipped() const
{
	return bool((m_data[1] >> 27) & 0x01);
}
	
void AgTerrestialPoint::setClipped(bool val)
{
	if (val)
		m_data[1] |= ((0x01 & val) << 27);
	else
	{
		m_data[1] &= ~(0x01 << 27);
	}
}