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
		 * 初始化地形数据
		 */
		void InitializeEarth();

		/**
		 * 获取地形高度模型
		 */
		BasicElevationModel & GetElevationModel();

		/**
		 * 获取地形区块细分对象
		 */
		RectangularTessellator & GetTessellator();

		/**
		 * 根据经纬度和海拔数据计算笛卡尔坐标
		 */
		Vector4D ComputePointFromPosition(const Angle & angle, const Angle & logitude, float elevation);

		/**
		 * @overload
		 */
		Vector4D ComputePointFromPosition(const LatLon & latlon, float elevation);

		/**
		 * 获取地球的赤道半径
		 */
		float GetRadius() const noexcept;

		/**
		 * 计算某个经纬度范围的bounding box
		 */
		Urho3D::BoundingBox ComputeBoundingBox(float verticalExaggeration, const Sector & sector);

		/**
		 * @overload
		 */
		Urho3D::BoundingBox ComputeBoundingBox(float verticalExaggeration, const Sector & sector, float minElevation, float maxElevation);

		/**
		 * 计算远裁剪面
		 */
		float ComputeFarClipDistance(const Urho3D::Vector3 & position);

		/**
		 * 计算球面坐标
		 */
		Position CartesianToGeodetic(const Urho3D::Vector3 & cart);

		/**
		 * 获取地球表面的位置
		 */
		bool GetPointOnTerrain( const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & point );

		/**
		 * 计算指定位置对应的高度
		 */
		float ComputeElevationAboveSurface(const Position & position);

		/**
		 * 计算水平距离
		 */
		float ComputeHorizonDistance(float elevation);

		/**
		 * 计算近裁剪面
		 */
		float ComputePerspectiveNearDistance(float farDist, float farResolution, uint32_t depthBits);

	private:

		/**
		 * 球面坐标转换为笛卡尔坐标
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