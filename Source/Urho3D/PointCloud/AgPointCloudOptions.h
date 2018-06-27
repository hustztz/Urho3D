#pragma once

#include "../Scene/Component.h"
#include "AgCompactColor.h"

#ifndef ALIGNAS
#ifndef _MSC_VER
#define ALIGNAS(x) alignas(x)
#else
#define ALIGNAS(x)
#endif
#endif

namespace ambergris {
	namespace PointCloudEngine {

		//////////////////////////////////////////////////////////////////////////
		// \brief: Stores the spatial information of a umbrella leaf node
		//////////////////////////////////////////////////////////////////////////
		class ALIGNAS(8) AgVoxelLeafNode
		{
		public:

			/*
			SVO Leaf Node ( 128 bits total )
			//ULong #1
			[0.....53]          XYZ Coordinates( 18 bits per axis )
			[54....61]          Classification LIDAR
			[62]                Not used
			[63]                Delete flag for persistent deletion

			//ULong #2
			[0....31]           RGBA
			[32...45]           Normal ( QSplat Paper )     --> http://graphics.stanford.edu/papers/qsplat/
			[46...49]           Classification(PRIMITIVES)  --> Cylinder, Plane, Sphere, Edge, Noise, Smooth Surface, Undefined
			[50...63]           Reserved                    --> segment id?
			*/


			AgVoxelLeafNode();
			AgVoxelLeafNode(const AgVoxelLeafNode& cpy);
			AgVoxelLeafNode(std::uint64_t data1, std::uint64_t data2);
			~AgVoxelLeafNode();

			//////////////////////////////////////////////////////////////////////////
			//\brief: Set the offset, in meters, from the lower left part of the bounding box
			//////////////////////////////////////////////////////////////////////////
			void                    setRawOffsetFromBoundingBox(const Urho3D::Vector3& offset);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the lidar classification data
			//////////////////////////////////////////////////////////////////////////
			void                    setLidarClassification(std::uint8_t val);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets a bit on indicating the point is deleted
			//////////////////////////////////////////////////////////////////////////
			void                    setDeleteFlag(bool onOff);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the rgba data
			//////////////////////////////////////////////////////////////////////////
			void                    setRGBA(const AgCompactColor& rgba);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the normal data
			//////////////////////////////////////////////////////////////////////////
			void                    setNormal(std::uint16_t normal);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the classification data
			//////////////////////////////////////////////////////////////////////////
			void                    setPrimitiveClassification(std::uint8_t val);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the offset from the lower left part of the bounding box
			//////////////////////////////////////////////////////////////////////////
			Urho3D::Vector3      getRawOffsetFromBoundingBox() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the classification data
			//////////////////////////////////////////////////////////////////////////
			std::uint8_t            getLidarClassification() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the delete flag
			//////////////////////////////////////////////////////////////////////////
			bool            getDeleteFlag() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the RGBA data
			//////////////////////////////////////////////////////////////////////////
			AgCompactColor     getRGBA() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the index into the normal lookup table(alMath.cpp)
			//////////////////////////////////////////////////////////////////////////
			std::uint16_t       getNormal() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns identified primitives (by K2)
			//////////////////////////////////////////////////////////////////////////
			std::uint8_t            getPrimitiveClassification() const;

			void                    testData();

		private:
			std::uint64_t                                       m_data[2];
		};
		static_assert(alignof(AgVoxelLeafNode) == 8, "alignof(AgVoxelLeafNode) is incorrect");
		static_assert(sizeof(AgVoxelLeafNode) == 16, "sizeof(AgVoxelLeafNode) is incorrect");

		namespace VoxelTreeRunTimeVars
		{
			static const Urho3D::StringHash VAR_SCANNERORIGIN("scannerOrigin");
			static const Urho3D::StringHash VAR_OCTREEFLAGS("OctreeFlags");
			static const Urho3D::StringHash VAR_TOTALPOINTS("TotalPoints");
			static const Urho3D::StringHash VAR_HASRGB("HasRGB");
			static const Urho3D::StringHash VAR_HASNORMALS("HasNormals");
			static const Urho3D::StringHash VAR_HASINTENSITY("HasIntensity");
			static const Urho3D::StringHash VAR_LIDARDATA("IsLidarData");
			static const Urho3D::StringHash VAR_MATERIAL("Material");
			static const Urho3D::StringHash VAR_POINTSIZE("PointSize");

			static const Urho3D::StringHash VAR_SVOBOUNDSMIN("svoBoundsMin");
			static const Urho3D::StringHash VAR_SVOBOUNDSMAX("svoBoundsMax");
			static const Urho3D::StringHash VAR_SCANBOUNDSMIN("scanBoundsMin");
			static const Urho3D::StringHash VAR_SCANBOUNDSMAX("scanBoundsMax");
		};

		class AgPointCloudOptions : public Urho3D::Component
		{
			URHO3D_OBJECT(AgPointCloudOptions, Urho3D::Component);

		public:
			explicit AgPointCloudOptions(Urho3D::Context* context)
				: Component(context)
				, pointSize_(1.0f)
			{
			}

			void setPointSize(float value) { pointSize_ = value; }
			float getPointSize() const { return pointSize_; }

		private:
			float pointSize_;
		};
	}
}

