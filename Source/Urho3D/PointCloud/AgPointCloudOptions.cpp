#include "AgPointCloudOptions.h"

using namespace Urho3D;
using namespace ambergris::PointCloudEngine;

//////////////////////////////////////////////////////////////////////////
//Class VoxelLeafNode
//////////////////////////////////////////////////////////////////////////
AgVoxelLeafNode::AgVoxelLeafNode()
{
	m_data[0] = m_data[1] = 0UL;
}

AgVoxelLeafNode::AgVoxelLeafNode(const AgVoxelLeafNode& cpy)
{
	m_data[0] = cpy.m_data[0];
	m_data[1] = cpy.m_data[1];
}

AgVoxelLeafNode::AgVoxelLeafNode(std::uint64_t data1, std::uint64_t data2)
{
	m_data[0] = data1;
	m_data[1] = data2;
}

AgVoxelLeafNode::~AgVoxelLeafNode()
{

}

void AgVoxelLeafNode::setRawOffsetFromBoundingBox(const Vector3& offset)
{
	int xOffset = static_cast<int>(offset.x_ + 0.5);
	int yOffset = static_cast<int>(offset.y_ + 0.5);
	int zOffset = static_cast<int>(offset.z_ + 0.5);

	m_data[0] &= ~(0x3FFFFULL);
	m_data[0] |= std::uint64_t(xOffset);

	m_data[0] &= ~(0x3FFFFULL << 18);
	m_data[0] |= std::uint64_t(yOffset) << 18;

	m_data[0] &= ~(0x3FFFFULL << 36);
	m_data[0] |= std::uint64_t(zOffset) << 36;
}

void AgVoxelLeafNode::setLidarClassification(std::uint8_t val)
{
	m_data[0] &= ~(0xFFULL << 54);
	m_data[0] |= std::uint64_t(val) << 54;
}

void AgVoxelLeafNode::setDeleteFlag(bool setFlag)
{
	m_data[0] &= ~(0x1ULL << 63);
	if (setFlag) {
		m_data[0] |= 0x1ULL << 63;
	}
}

void AgVoxelLeafNode::setRGBA(const AgCompactColor& rgba)
{
	m_data[1] &= ~(0xFFFFFFFFULL);
	m_data[1] |= std::uint64_t(rgba.r_) << 24;
	m_data[1] |= std::uint64_t(rgba.g_) << 16;
	m_data[1] |= std::uint64_t(rgba.b_) << 8;
	m_data[1] |= std::uint64_t(rgba.a_) << 0;
}

void AgVoxelLeafNode::setNormal(std::uint16_t normal)
{
	// Bits [32...45] Normal
	m_data[1] &= ~(0x3FFFULL << 32);
	m_data[1] |= (0x3FFFULL & std::uint64_t(normal)) << 32;
}

void AgVoxelLeafNode::setPrimitiveClassification(std::uint8_t val)
{
	m_data[1] &= ~(0xFULL << 46);
	m_data[1] |= (0xFULL & std::uint64_t(val)) << 46;
}

Vector3 AgVoxelLeafNode::getRawOffsetFromBoundingBox() const
{
	std::uint32_t   xOffset,
		yOffset,
		zOffset;

	xOffset = std::uint32_t((m_data[0]) & 0x3FFFFULL);
	yOffset = std::uint32_t((m_data[0] >> 18) & 0x3FFFFULL);
	zOffset = std::uint32_t((m_data[0] >> 36) & 0x3FFFFULL);

	return Vector3(float(xOffset), float(yOffset), float(zOffset));
}


std::uint8_t AgVoxelLeafNode::getLidarClassification() const
{
	return std::uint8_t((m_data[0] >> 54) & 0xFFULL);
}


bool AgVoxelLeafNode::getDeleteFlag() const
{
	return (m_data[0] >> 63) & 0x1ULL;
}


AgCompactColor AgVoxelLeafNode::getRGBA() const
{
	std::uint8_t r,
		g,
		b,
		a;

	r = std::uint8_t((m_data[1] >> 24) & 0xFFULL);
	g = std::uint8_t((m_data[1] >> 16) & 0xFFULL);
	b = std::uint8_t((m_data[1] >> 8) & 0xFFULL);
	a = std::uint8_t((m_data[1]) & 0xFFULL);

	return AgCompactColor(r, g, b, a);
}

std::uint16_t AgVoxelLeafNode::getNormal() const
{
	// Bits [32...45] Normal
	return std::uint16_t((m_data[1] >> 32) & 0x3FFFULL);
}

std::uint8_t AgVoxelLeafNode::getPrimitiveClassification() const
{
	return std::uint8_t((m_data[1] >> 46) & 0xFULL);
}