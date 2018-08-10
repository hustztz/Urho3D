#pragma once

#include "Angle.h"

namespace GIS 
{

    class LatLon 
    {
    public:

        static LatLon FromRadians(float latitude, float longitude)
        {
            LatLon latlon;
            latlon.latitude_ = Angle::FromRadians(latitude);
            latlon.longitude_ = Angle::FromRadians(longitude);
            return latlon;
        }

        static LatLon FromDegrees(float latitude, float longitude) 
        {
            LatLon latlon;
            latlon.latitude_ = Angle::FromDegrees(latitude);
            latlon.longitude_ = Angle::FromDegrees(longitude);
            return latlon;
        }

		LatLon() {
		}

		LatLon(const Angle & latitude, const Angle & longitude)
		{
			latitude_ = latitude;
			longitude_ = longitude;
		}

        const Angle & GetLatitude() const noexcept 
		{
            return latitude_;
        }

        const Angle & GetLongitude() const noexcept 
		{
            return longitude_;
        }

    private:

        Angle latitude_;

        Angle longitude_;


    };

	/**
	 * 经纬度-海拔的坐标位置
	 */
	class Position : public LatLon {
	public:

		Position()
			: elevation_( 0 ) {
		}

		Position(const Angle & latitude, const Angle & longitude, float elevation)
			: LatLon( latitude, longitude ), elevation_( elevation ) {}

		/**
		 * 获取海拔高度
		 */
		float GetElevation() const noexcept
		{
			return elevation_;
		}

	private:

		float elevation_;

	};

}