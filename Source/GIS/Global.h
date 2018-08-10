#pragma once

#include "ElevationConfig.h"
#include "LevelSet.h"
#include "Angle.h"
#include <Urho3D/Math/BoundingBox.h>

namespace GIS 
{

	class BasicElevationModel;

	class RectangularTessellator;

	class Global 
	{
	public:

		struct Vector4D {

			double x, y, z, w;

		};

		struct Vector3D {
			double x, y, z;
		};

	public:

		Global();

		/**
		 * ��ʼ����������
		 */
		void InitializeEarth();

		/**
		 * ��ȡ���θ߶�ģ��
		 */
		BasicElevationModel & GetElevationModel();

		/**
		 * ��ȡ��������ϸ�ֶ���
		 */
		RectangularTessellator & GetTessellator();

		/**
		 * ���ݾ�γ�Ⱥͺ������ݼ���ѿ�������
		 */
		Vector4D ComputePointFromPosition(const Angle & angle, const Angle & logitude, float elevation);

		/**
		 * @overload
		 */
		Vector4D ComputePointFromPosition(const LatLon & latlon, float elevation);

		/**
		 * ��ȡ����ĳ���뾶
		 */
		float GetRadius() const noexcept;

		/**
		 * ����ĳ����γ�ȷ�Χ��bounding box
		 */
		Urho3D::BoundingBox ComputeBoundingBox(float verticalExaggeration, const Sector & sector);

		/**
		 * @overload
		 */
		Urho3D::BoundingBox ComputeBoundingBox(float verticalExaggeration, const Sector & sector, float minElevation, float maxElevation);

		/**
		 * ����Զ�ü���
		 */
		float ComputeFarClipDistance(const Urho3D::Vector3 & position);

		/**
		 * ������������
		 */
		Position CartesianToGeodetic(const Urho3D::Vector3 & cart);

		/**
		 * ��ȡ��������λ��
		 */
		bool GetPointOnTerrain( const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & point );

		/**
		 * ����ָ��λ�ö�Ӧ�ĸ߶�
		 */
		float ComputeElevationAboveSurface(const Position & position);

		/**
		 * ����ˮƽ����
		 */
		float ComputeHorizonDistance(float elevation);

		/**
		 * ������ü���
		 */
		float ComputePerspectiveNearDistance(float farDist, float farResolution, uint32_t depthBits);

	private:

		/**
		 * ��������ת��Ϊ�ѿ�������
		 */
		Vector4D GeodeticToCartesian(const Angle & latitude, const Angle & longitude, float elevation);

	private:

		ElevationConfig config_;

		LevelSet levelSet_;

		std::shared_ptr<BasicElevationModel> elevationModel_;

		std::shared_ptr<RectangularTessellator> tessellator_;

		double equatorialRadius_;

		double polarRadius_;

		double es_;

		double em_;

	};

}