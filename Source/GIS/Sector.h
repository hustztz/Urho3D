#pragma once 

#include "LatLon.h"
#include "Angle.h"
#include "HashCombine.h"
#include <vector>

namespace GIS 
{

    class Sector 
    {
    public:

        static Sector FromDegrees( float minLatitude, float maxLatitude, float minLongitude, float maxLongitude ) 
        {
			Sector sector;
            sector.minLatitude_ = Angle::FromDegrees( minLatitude );
            sector.maxLatitude_ = Angle::FromDegrees( maxLatitude );
            sector.minLongitude_ = Angle::FromDegrees( minLongitude );
            sector.maxLongitude_ = Angle::FromDegrees( maxLongitude );
            sector.deltaLat_ = Angle::FromDegrees( maxLatitude - minLatitude );
            sector.deltaLon_ = Angle::FromDegrees( maxLongitude - minLongitude );
			return sector;
        }

		static Sector FromRadians(float minLatitude, float maxLatitude, float minLongitude, float maxLongitude)
		{
			Sector sector;
			sector.minLatitude_ = Angle::FromRadians(minLatitude);
			sector.maxLatitude_ = Angle::FromRadians(maxLatitude);
			sector.minLongitude_ = Angle::FromRadians(minLongitude);
			sector.maxLongitude_ = Angle::FromRadians(maxLongitude);
			sector.deltaLat_ = Angle::FromRadians(maxLatitude - minLatitude);
			sector.deltaLon_ = Angle::FromRadians(maxLongitude - minLongitude);
			return sector;
		}

        Sector() = default;

		Sector( Angle minLatitude, Angle maxLatitude, Angle minLongitude, Angle maxLongitude )
		{
			minLatitude_ = minLatitude;
			maxLatitude_ = maxLatitude;
			minLongitude_ = minLongitude;
			maxLongitude_ = maxLongitude;
			deltaLat_ = maxLatitude - minLatitude;
			deltaLon_ = maxLongitude - minLongitude;
		}

        const Angle & GetMinLatitude() const noexcept 
        {
            return minLatitude_;
        }

        const Angle & GetMaxLatitude() const noexcept 
        {
            return maxLatitude_;
        }

        const Angle & GetMinLongitude() const noexcept 
        {
            return minLongitude_;
        }

        const Angle & GetMaxLongitude() const noexcept 
        {
            return maxLongitude_;
        }

        const Angle & GetDeltaLat() const noexcept 
        {
            return deltaLat_;
        }

        const Angle & GetDeltaLon() const noexcept 
        {
            return deltaLon_;
        }


		std::vector<Sector> Subdivide() const noexcept 
		{
			std::vector<Sector> sectors(4);
			auto midLat = Angle::Average(minLatitude_, maxLatitude_);
			auto midLon = Angle::Average(minLongitude_, maxLongitude_);
			sectors[0] = FromDegrees(minLatitude_.GetDegrees(), midLat.GetDegrees(), minLongitude_.GetDegrees(), midLon.GetDegrees());
			sectors[1] = FromDegrees(minLatitude_.GetDegrees(), midLat.GetDegrees(), midLon.GetDegrees(),		 maxLongitude_.GetDegrees());
			sectors[2] = FromDegrees(midLat.GetDegrees(), maxLatitude_.GetDegrees(), minLongitude_.GetDegrees(), midLon.GetDegrees());
			sectors[3] = FromDegrees(midLat.GetDegrees(), maxLatitude_.GetDegrees(), midLon.GetDegrees(), maxLongitude_.GetDegrees());
			return sectors;
		}

		bool Intersect(const Sector & that, Sector *result ) const noexcept 
		{
			Angle minLat, maxLat;
			minLat = minLatitude_ > that.minLatitude_ ? minLatitude_ : that.minLatitude_;
			maxLat = maxLatitude_ < that.maxLatitude_ ? maxLatitude_ : that.maxLatitude_;
			if (minLat > maxLat)
				return false;
			Angle minLon, maxLon;
			minLon = minLongitude_ > that.minLongitude_ ? minLongitude_ : that.minLongitude_;
			maxLon = maxLongitude_ < that.maxLongitude_ ? maxLongitude_ : that.maxLongitude_;
			if (minLon > maxLon)
				return false;
			*result = Sector{ minLat, maxLat, minLon, maxLon };
			return true;
		}

		bool IsContain(const Angle & latitude, const Angle & longitude ) const noexcept
		{
			return IsContainDegrees(latitude.GetDegrees(), longitude.GetDegrees());
		}

		bool IsContainDegrees(double degreeLatitude, double degreeLongitude) const noexcept 
		{
			return degreeLatitude >= minLatitude_.GetDegrees() && degreeLatitude <= maxLatitude_.GetDegrees()
				&& degreeLongitude >= minLongitude_.GetDegrees() && degreeLongitude <= maxLongitude_.GetDegrees();
		}

		LatLon GetCentroid() const noexcept {
			auto lat = Angle::FromDegrees(0.5f * (GetMaxLatitude().GetDegrees() + GetMinLatitude().GetDegrees()));
			auto lon = Angle::FromDegrees(0.5f * (GetMaxLongitude().GetDegrees() + GetMinLongitude().GetDegrees()));
			return { lat, lon };
		}

		std::vector<LatLon> GetLatLons() const noexcept {
			return { { minLatitude_, minLongitude_ }, { minLatitude_, maxLongitude_ }, { maxLatitude_, minLongitude_ }, { maxLatitude_, maxLongitude_ } };
		}

    private:

        Angle minLatitude_; 

        Angle maxLatitude_;

        Angle minLongitude_;

        Angle maxLongitude_;

        Angle deltaLat_;

        Angle deltaLon_;

    };

	class SectorKey {
	public:

		SectorKey(const Sector & sector) {
			m_HashCode = 0;
			HashCombine(m_HashCode, sector.GetMinLatitude().GetDegrees());
			HashCombine(m_HashCode, sector.GetMaxLatitude().GetDegrees());
			HashCombine(m_HashCode, sector.GetMinLongitude().GetDegrees());
			HashCombine(m_HashCode, sector.GetMaxLongitude().GetDegrees());
		}

		size_t GetHashCode() const noexcept {
			return m_HashCode;
		}

		bool operator ==(const SectorKey & that) const noexcept {
			return m_HashCode == that.m_HashCode;
		}

	private:

		size_t m_HashCode;

	};

}

namespace std {

	template <>
	struct hash<GIS::SectorKey> {

		size_t operator ()(const GIS::SectorKey & sector) const noexcept {
			return sector.GetHashCode();
		}

	};

}

