#include "AgOctreeDefinitions.h"

#include <common/RCMath.h>
#include <common/RCString.h>
#include <common/RCMemoryMapFile.h>
#include <utility/RCLog.h>
#include <utility/RCStringUtils.h>

#include <sstream>

using namespace ambergris::PointCloudEngine;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility::Log;
using namespace ambergris::RealityComputing::Utility::String;

RCBox ambergris::PointCloudEngine::boundsForId(const RCBox& bounds, int id)
{
	const auto min = bounds.getMin();
	const auto max = bounds.getMax();
	const auto center = bounds.getCenter();

	switch (id)
	{
		// set the nodes id
	case LEFT_FRONT_BOTTOM: //0
		return RCBox(min, center);
	case RIGHT_FRONT_BOTTOM:    //1
		return RCBox(RCVector3d(center.x, min.y, min.z), RCVector3d(max.x, center.y, center.z));
	case LEFT_BACK_BOTTOM:  //2
		return RCBox(RCVector3d(min.x, center.y, min.z), RCVector3d(center.x, max.y, center.z));
		//break;
	case RIGHT_BACK_BOTTOM: //3
		return RCBox(RCVector3d(center.x, center.y, min.z), RCVector3d(max.x, max.y, center.z));
	case LEFT_FRONT_TOP:    //4
		return RCBox(RCVector3d(min.x, min.y, center.z), RCVector3d(center.x, center.y, max.z));
	case RIGHT_FRONT_TOP:   //5
		return RCBox(RCVector3d(center.x, min.y, center.z), RCVector3d(max.x, center.y, max.z));
	case LEFT_BACK_TOP: //6
		return RCBox(RCVector3d(min.x, center.y, center.z), RCVector3d(center.x, max.y, max.z));
	case RIGHT_BACK_TOP:  //7
		return RCBox(center, max);
	}
	return RCBox();
}

alSvoSliceHeader::alSvoSliceHeader()
{
	m_offsetOnDisk = 0;
	m_sliceSizeInBytes = 0;
}

OctreeFileHeader::OctreeFileHeader()
	: m_magicWord{ 0 }, m_majorVersion(MAJOR_VERSION), m_minorVersion(MINOR_VERSION),
	m_scale(RCVector3d(1.0, 1.0, 1.0)),
	m_octreeFlags(0), m_pointCloudProvider(RCScanProvider::PROVIDER_UNKNOWN),
	m_totalAmountOfPoints(0), mHasRGB(false), mHasNormals(false), mHasIntensity(false),
	mScanId{ 0 }, mCoordinateSystem{ 0 }, m_numFileChunks(0)
{
	m_magicWord[0] = 'A';
	m_magicWord[1] = 'D';
	m_magicWord[2] = 'O';
	m_magicWord[3] = 'C';
	m_magicWord[4] = 'T';
}

ScanInfo::~ScanInfo()
{
	mScanName = L"";
}

//////////////////////////////////////////////////////////////////////////
//alSVOStorageNode
//////////////////////////////////////////////////////////////////////////
alSVOStorageNode::alSVOStorageNode()
{
	//settings this all to '1' will improve compression
	m_rgba[0] = m_rgba[1] = m_rgba[2] = m_rgba[3] = 0xFF;
	m_bitSet = 0xFFFFFFFF;
	m_offsetInSlice = MAX_NODES_IN_SLICE;
	setSliceId(MAX_SLICES_IN_TREE + 1);
}

alSVOStorageNode::alSVOStorageNode(const alSVOStorageNode& other)
{
	m_rgba[0] = other.m_rgba[0];
	m_rgba[1] = other.m_rgba[1];
	m_rgba[2] = other.m_rgba[2];
	m_rgba[3] = other.m_rgba[3];
	m_bitSet = other.m_bitSet;
	m_offsetInSlice = other.m_offsetInSlice;
	setSliceId(other.getSliceId());

}

std::uint32_t alSVOStorageNode::getSliceId() const
{
	std::uint32_t val = (m_sliceId[0] << 16) | (m_sliceId[1] << 8) | (m_sliceId[2]);
	return val;
}

std::uint16_t alSVOStorageNode::getOffsetInSlice() const
{
	return m_offsetInSlice;
}

bool alSVOStorageNode::isValid()
{
	if (getSliceId() == (MAX_SLICES_IN_TREE + 1) && m_offsetInSlice == MAX_NODES_IN_SLICE &&
		m_bitSet == static_cast<int>(0xFFFFFFFF) &&
		m_rgba[0] == 0xFF && m_rgba[1] == 0xFF && m_rgba[2] == 0xFF && m_rgba[3] == 0xFF
		)
		return false;

	return true;
}

std::uint16_t alSVOStorageNode::getNormal()
{
	int val = m_bitSet >> 10;
	return (val & 0x3FFF);
}

void alSVOStorageNode::setNormal(int  val)
{
	m_bitSet &= ~(0x3FFF << 10);
	m_bitSet |= (val << 10);
}

void alSVOStorageNode::setSliceId(int val)
{
	m_sliceId[0] = (val >> 16) & 0xff;
	m_sliceId[1] = (val >> 8) & 0xff;
	m_sliceId[2] = val & 0xff;
}

void alSVOStorageNode::enable()
{
	m_rgba[0] = m_rgba[1] = m_rgba[2] = m_rgba[3] = 0;
	m_bitSet = 0;
	m_offsetInSlice = 0;
}

bool alSVOStorageNode::hasChild(int childNum) const
{
	return ((m_bitSet & (1 << childNum)) != 0);
}

void alSVOStorageNode::setOffsetInSlice(std::uint16_t val)
{
	m_offsetInSlice = val;
}

int alSVOStorageNode::getNumChilds()
{
	return Math::bitCount(m_bitSet & 0xFF);
}

std::uint8_t* alSVOStorageNode::getSliceIdAsPointerUnSafe()
{
	return &m_sliceId[0];
}

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

void AgVoxelLeafNode::setRawOffsetFromBoundingBox(const RCVector3f& offset)
{
	int xOffset = static_cast<int>(offset.x + 0.5);
	int yOffset = static_cast<int>(offset.y + 0.5);
	int zOffset = static_cast<int>(offset.z + 0.5);

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

void AgVoxelLeafNode::setRGBA(const RCVector4ub& rgba)
{
	m_data[1] &= ~(0xFFFFFFFFULL);
	m_data[1] |= std::uint64_t(rgba.x) << 24;
	m_data[1] |= std::uint64_t(rgba.y) << 16;
	m_data[1] |= std::uint64_t(rgba.z) << 8;
	m_data[1] |= std::uint64_t(rgba.w) << 0;
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

RCVector3f AgVoxelLeafNode::getRawOffsetFromBoundingBox() const
{
	std::uint32_t   xOffset,
		yOffset,
		zOffset;

	xOffset = std::uint32_t((m_data[0]) & 0x3FFFFULL);
	yOffset = std::uint32_t((m_data[0] >> 18) & 0x3FFFFULL);
	zOffset = std::uint32_t((m_data[0] >> 36) & 0x3FFFFULL);

	return RCVector3f(float(xOffset), float(yOffset), float(zOffset));
}


std::uint8_t AgVoxelLeafNode::getLidarClassification() const
{
	return std::uint8_t((m_data[0] >> 54) & 0xFFULL);
}


bool AgVoxelLeafNode::getDeleteFlag() const
{
	return (m_data[0] >> 63) & 0x1ULL;
}


RCVector4ub AgVoxelLeafNode::getRGBA() const
{
	std::uint8_t r,
		g,
		b,
		a;

	r = std::uint8_t((m_data[1] >> 24) & 0xFFULL);
	g = std::uint8_t((m_data[1] >> 16) & 0xFFULL);
	b = std::uint8_t((m_data[1] >> 8) & 0xFFULL);
	a = std::uint8_t((m_data[1]) & 0xFFULL);

	return RCVector4ub(r, g, b, a);
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

//////////////////////////////////////////////////////////////////////////
// VoxelInteriorNode
//////////////////////////////////////////////////////////////////////////

AgVoxelInteriorNode::AgVoxelInteriorNode()
{
	m_data = 0;
}

AgVoxelInteriorNode::AgVoxelInteriorNode(const AgVoxelInteriorNode& cpy)
{
	m_data = cpy.m_data;
}

AgVoxelInteriorNode::~AgVoxelInteriorNode()
{

}

void AgVoxelInteriorNode::setIndexToChild(int childIndex)
{
	m_data &= ~(0x7FFFF);
	m_data |= childIndex;
}

void AgVoxelInteriorNode::setChildInfo(int val)
{
	m_data &= ~(0xFF << 19);
	m_data |= (0xFF & val) << 19;
}

void AgVoxelInteriorNode::setLeafInfo(int val)
{
	m_data &= ~(0x1 << 27);
	m_data |= (0x1 & val) << 27;
}


void AgVoxelInteriorNode::testData()
{
	for (int i = 0; i < 1000; i++)
	{
		int indexToChild = Math::random(1, 512000);
		int childInfo = Math::random(0, 255);
		int isLeaf = Math::random(0, 1);

		setIndexToChild(indexToChild);
		setChildInfo(childInfo);
		setLeafInfo(isLeaf);

		{
			RCLog::getLogFile()->addMessage(L"================================================");
			RCLog::getLogFile()->addMessage(L"Before");

			std::wstringstream ss;
			ss << indexToChild << ", " << childInfo << ", " << isLeaf;
			RCLog::getLogFile()->addMessage(ss.str().c_str());
		}

		indexToChild = getChildIndex();
		childInfo = getChildInfo();
		isLeaf = getLeafInfo();

		{
			RCLog::getLogFile()->addMessage(L"After");

			std::wstringstream ss;
			ss << indexToChild << ", " << childInfo << ", " << isLeaf;
			RCLog::getLogFile()->addMessage(ss.str().c_str());
		}


	}
}

int AgVoxelInteriorNode::getChildIndex() const
{
	return int(m_data & 0x7FFFF);
}

int AgVoxelInteriorNode::getChildInfo() const
{
	return int((m_data >> 19) & 0xFF);
}

int AgVoxelInteriorNode::getLeafInfo()
{
	return int((m_data >> 27) & 0x1);
}