#pragma once

#include <cmath>

namespace GIS 
{

    const float PI = 3.14159265358979323846f;

    const float DEGREES_TO_RADIANS = PI / 180.0f;

    const float RADIANS_TO_DEGREES = 180.0f / PI;

    class Angle 
    {
    public:

        static Angle FromDegrees(float degrees) 
        {
            return { degrees };
        }

        static Angle FromRadians(float radians) 
        {
            return { radians * RADIANS_TO_DEGREES };
        }

		static Angle Average(const Angle & lhs, const Angle & rhs)
		{
			return FromDegrees((lhs.GetDegrees() + rhs.GetDegrees()) * 0.5f);
		}

        Angle() {}

        float GetDegrees() const noexcept 
        {
            return degrees_;
        }

        double GetRadians() const noexcept 
        {
            return radians_;
        }

        Angle operator +(const Angle & that) const noexcept
        {
            return FromDegrees(GetDegrees() + that.GetDegrees());
        }

        Angle operator -( const Angle & that ) const noexcept 
        {
            return FromDegrees( GetDegrees() - that.GetDegrees() );
        }

		Angle & operator +=(const Angle & that) {
			*this = FromDegrees(that.degrees_ + degrees_);
			return *this;
		}

        bool operator <( const Angle & that ) const noexcept 
        {
            return GetDegrees() < that.GetDegrees();
        }

        bool operator >( const Angle & that ) const noexcept 
        {
            return GetDegrees() > that.GetDegrees();
        }

		Angle operator /(float scalar) const {
			return Angle::FromDegrees(degrees_ / scalar);
		}

		Angle operator *(float scalar) const {
			return Angle::FromDegrees(degrees_ * scalar);
		}

		Angle MidAngle(const Angle & a2) const noexcept {
			return Angle::FromDegrees((GetDegrees() + a2.GetDegrees()) / 2.0f);
		}

		bool operator ==(const Angle & angle) const noexcept {
			return abs(degrees_ - angle.degrees_) < 0.000001;
		}

    private:

        Angle(float degrees) 
        {
            degrees_ = degrees;
            radians_ = DEGREES_TO_RADIANS * degrees;
        }

        float degrees_ = 0;
        
        double radians_ = 0;

	};

}