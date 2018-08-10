#include "Global.h"
#include "BasicElevationModel.h"
#include "RectangularTessellator.h"
#include <thread>
#include <chrono>

namespace {

	const double WGS84_EQUATORIAL_RADIUS = 6378137.0; // ellipsoid equatorial getRadius, in meters
	const double WGS84_POLAR_RADIUS = 6356752.3; // ellipsoid polar getRadius, in meters
	const double WGS84_ES = 0.00669437999013; // eccentricity squared, semi-major axis
	const double ELEVATION_MIN = -11000.0; // Depth of Marianas trench
	const double ELEVATION_MAX = 8500.0; // Height of Mt. Everest.

}

GIS::Global::Global() : 
	config_( "config/Earth/EarthMergedElevationModel.xml" ),
	levelSet_( config_ ),
	equatorialRadius_( WGS84_EQUATORIAL_RADIUS ),
	polarRadius_( WGS84_POLAR_RADIUS ),
	es_( WGS84_ES ) 
{
	InitializeEarth();
}

void GIS::Global::InitializeEarth() 
{
	elevationModel_.reset(new BasicElevationModel{ config_ });
	elevationModel_->LoadAllLevelZeroTiles();
	tessellator_.reset(new RectangularTessellator{ *this, config_ });
}

auto GIS::Global::GetElevationModel() -> BasicElevationModel &
{
	return *elevationModel_;
}

auto GIS::Global::GetTessellator() -> RectangularTessellator &
{
	return *tessellator_;
}

auto GIS::Global::ComputePointFromPosition(const Angle & angle, const Angle & logitude, float elevation) -> Vector4D 
{
	return GeodeticToCartesian( angle, logitude, elevation );
}

auto GIS::Global::ComputePointFromPosition(const LatLon & latlon, float elevation) -> Vector4D
{
	return ComputePointFromPosition( latlon.GetLatitude(), latlon.GetLongitude(), elevation );
}

float GIS::Global::GetRadius() const noexcept
{
	return equatorialRadius_;
}

Urho3D::BoundingBox GIS::Global::ComputeBoundingBox(float verticalExaggeration, const Sector & sector)
{
	auto maxMinElevations = GetElevationModel().GetExtremeElevations(sector);
	return ComputeBoundingBox(verticalExaggeration, sector, maxMinElevations.first, maxMinElevations.second);
}

namespace {

	void Merge(Urho3D::BoundingBox & bbox, const GIS::Global::Vector4D & pos) {
		bbox.Merge(Urho3D::Vector3(pos.x, pos.y, pos.z));
	}

}

Urho3D::BoundingBox GIS::Global::ComputeBoundingBox(float verticalExaggeration, const Sector & sector, float minElevation, float maxElevation)
{
	Urho3D::BoundingBox bbox;

	double minHeight = minElevation * verticalExaggeration;
	double maxHeight = maxElevation * verticalExaggeration;

	if (minHeight == maxHeight)
		maxHeight = minHeight + 10; // Ensure the top and bottom heights are not equal.

	//std::vector<Vector4D> points;
	
	//for (LatLon ll : sector.GetLatLons() )
	//{
	//	Merge( bbox, ComputePointFromPosition(ll, minHeight));
	//	Merge(bbox, ComputePointFromPosition(ll, maxHeight));
	//}

	Merge(bbox, ComputePointFromPosition(sector.GetMinLatitude(), sector.GetMinLongitude(), minHeight));
	Merge(bbox, ComputePointFromPosition(sector.GetMinLatitude(), sector.GetMinLongitude(), maxHeight));

	Merge(bbox, ComputePointFromPosition(sector.GetMaxLatitude(), sector.GetMinLongitude(), minHeight));
	Merge(bbox, ComputePointFromPosition(sector.GetMaxLatitude(), sector.GetMinLongitude(), maxHeight));

	Merge(bbox, ComputePointFromPosition(sector.GetMinLatitude(), sector.GetMaxLongitude(), minHeight));
	Merge(bbox, ComputePointFromPosition(sector.GetMinLatitude(), sector.GetMaxLongitude(), maxHeight));

	Merge(bbox, ComputePointFromPosition(sector.GetMaxLatitude(), sector.GetMaxLongitude(), minHeight));
	Merge(bbox, ComputePointFromPosition(sector.GetMaxLatitude(), sector.GetMaxLongitude(), maxHeight));

	// A point at the centroid captures the maximum vertical dimension.
	LatLon centroid = sector.GetCentroid();
	Merge(bbox, ComputePointFromPosition(centroid, maxHeight));

	// If the sector spans the equator, then the curvature of all four edges need to be taken into account. The
	// extreme points along the top and bottom edges are located at their mid-points, and the extreme points along
	// the left and right edges are on the equator. Add points with the longitude of the sector's centroid but with
	// the sector's min and max latitude, and add points with the sector's min and max longitude but with latitude
	// at the equator. See WWJINT-225.
	if (sector.GetMinLatitude().GetDegrees() < 0 && sector.GetMaxLatitude().GetDegrees() > 0)
	{
		Merge( bbox, ComputePointFromPosition(LatLon(sector.GetMinLatitude(), centroid.GetLongitude()),
			maxHeight));
		Merge( bbox, ComputePointFromPosition(LatLon(sector.GetMaxLatitude(), centroid.GetLongitude()),
			maxHeight));
		Merge( bbox, ComputePointFromPosition(LatLon(Angle::FromDegrees( 0 ), sector.GetMinLongitude()), maxHeight));
		Merge( bbox, ComputePointFromPosition(LatLon(Angle::FromDegrees(0), sector.GetMaxLongitude()), maxHeight));
	}
	// If the sector is located entirely in the southern hemisphere, then the curvature of its top edge needs to be
	// taken into account. The extreme point along the top edge is located at its mid-point. Add a point with the
	// longitude of the sector's centroid but with the sector's max latitude. See WWJINT-225.
	else if (sector.GetMinLatitude().GetDegrees() < 0)
	{
		Merge( bbox, ComputePointFromPosition(LatLon(sector.GetMaxLatitude(), centroid.GetLongitude()),
			maxHeight));
	}
	// If the sector is located entirely in the northern hemisphere, then the curvature of its bottom edge needs to
	// be taken into account. The extreme point along the bottom edge is located at its mid-point. Add a point with
	// the longitude of the sector's centroid but with the sector's min latitude. See WWJINT-225.
	else
	{
		Merge( bbox, ComputePointFromPosition(LatLon(sector.GetMinLatitude(), centroid.GetLongitude()),
			maxHeight));
	}

	// If the sector spans 360 degrees of longitude then is a band around the entire globe. (If one edge is a pole
	// then the sector looks like a circle around the pole.) Add points at the min and max latitudes and longitudes
	// 0, 180, 90, and -90 to capture full extent of the band.
	if (sector.GetDeltaLon().GetDegrees() >= 360)
	{
		Angle minLat = sector.GetMinLatitude();
		Merge( bbox, ComputePointFromPosition(minLat, Angle::FromDegrees( 0 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(minLat, Angle::FromDegrees( 90 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(minLat, Angle::FromDegrees( -90 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(minLat, Angle::FromDegrees( 180 ), maxHeight));

		Angle maxLat = sector.GetMaxLatitude();
		Merge( bbox, ComputePointFromPosition(maxLat, Angle::FromDegrees( 0 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(maxLat, Angle::FromDegrees( 90 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(maxLat, Angle::FromDegrees( -90 ), maxHeight));
		Merge( bbox, ComputePointFromPosition(maxLat, Angle::FromDegrees( 180 ), maxHeight));
	}
	else if (sector.GetDeltaLon().GetDegrees() > 180)
	{
		// Need to compute more points to ensure the box encompasses the full sector.
		Angle cLon = sector.GetCentroid().GetLongitude();
		Angle cLat = sector.GetCentroid().GetLatitude();

		// centroid latitude, longitude midway between min longitude and centroid longitude
		Angle lon = sector.GetMinLongitude().MidAngle(cLon);
		Merge( bbox, ComputePointFromPosition(cLat, lon, maxHeight));

		// centroid latitude, longitude midway between centroid longitude and max longitude
		lon = cLon.MidAngle(sector.GetMaxLongitude());
		Merge( bbox, ComputePointFromPosition(cLat, lon, maxHeight));

		// centroid latitude, longitude at min longitude and max longitude
		Merge( bbox, ComputePointFromPosition(cLat, sector.GetMinLongitude(), maxHeight));
		Merge( bbox, ComputePointFromPosition(cLat, sector.GetMaxLongitude(), maxHeight));
	}
	return bbox;
}

float GIS::Global::ComputeFarClipDistance(const Urho3D::Vector3 & eyePoint )
{

	auto eyePosition = CartesianToGeodetic(eyePoint);

	return ComputeHorizonDistance(eyePosition.GetElevation());

	//auto elevationAboveSurface = ComputeElevationAboveSurface(eyePosition);

	//return ComputeHorizonDistance(std::max(elevationAboveSurface, eyePosition.GetElevation()));
}

GIS::Position GIS::Global::CartesianToGeodetic(const Urho3D::Vector3 & cart)
{
	double X = cart.z_;
	double Y = cart.x_;
	double Z = cart.y_;
	double XXpYY = X * X + Y * Y;
	double sqrtXXpYY = sqrt(XXpYY);

	double a = equatorialRadius_;
	double ra2 = 1 / (a * a);
	double e2 = es_;
	double e4 = e2 * e2;

	// Step 1
	double p = XXpYY * ra2;
	double q = Z * Z * (1 - e2) * ra2;
	double r = (p + q - e4) / 6;

	double h;
	double phi;

	double evoluteBorderTest = 8 * r * r * r + e4 * p * q;
	if (evoluteBorderTest > 0 || q != 0)
	{
		double u;

		if (evoluteBorderTest > 0)
		{
			// Step 2: general case
			double rad1 = sqrt(evoluteBorderTest);
			double rad2 = sqrt(e4 * p * q);

			// 10*e2 is my arbitrary decision of what Vermeille means by "near... the cusps of the evolute".
			if (evoluteBorderTest > 10 * e2)
			{
				double rad3 = cbrt((rad1 + rad2) * (rad1 + rad2));
				u = r + 0.5 * rad3 + 2 * r * r / rad3;
			}
			else
			{
				u = r + 0.5 * cbrt((rad1 + rad2) * (rad1 + rad2)) + 0.5 * cbrt(
					(rad1 - rad2) * (rad1 - rad2));
			}
		}
		else
		{
			// Step 3: near evolute
			double rad1 = sqrt(-evoluteBorderTest);
			double rad2 = sqrt(-8 * r * r * r);
			double rad3 = sqrt(e4 * p * q);
			double atan = 2 * atan2(rad3, rad1 + rad2) / 3;

			u = -4 * r * sin(atan) * cos(PI / 6 + atan);
		}

		double v = sqrt(u * u + e4 * q);
		double w = e2 * (u + v - q) / (2 * v);
		double k = (u + v) / (sqrt(w * w + u + v) + w);
		double D = k * sqrtXXpYY / (k + e2);
		double sqrtDDpZZ = sqrt(D * D + Z * Z);

		h = (k + e2 - 1) * sqrtDDpZZ / k;
		phi = 2 * atan2(Z, sqrtDDpZZ + D);
	}
	else
	{
		// Step 4: singular disk
		double rad1 = sqrt(1 - e2);
		double rad2 = sqrt(e2 - p);
		double e = sqrt(e2);

		h = -a * rad1 * rad2 / e;
		phi = rad2 / (e * rad2 + rad1 * sqrt(p));
	}

	// Compute lambda
	double lambda;
	double s2 = sqrt(2);
	if ((s2 - 1) * Y < sqrtXXpYY + X)
	{
		// case 1 - -135deg < lambda < 135deg
		lambda = 2 * atan2(Y, sqrtXXpYY + X);
	}
	else if (sqrtXXpYY + Y < (s2 + 1) * X)
	{
		// case 2 - -225deg < lambda < 45deg
		lambda = -PI * 0.5 + 2 * atan2(X, sqrtXXpYY - Y);
	}
	else
	{
		// if (sqrtXXpYY-Y<(s2=1)*X) {  // is the test, if needed, but it's not
		// case 3: - -45deg < lambda < 225deg
		lambda = PI * 0.5 - 2 * atan2(X, sqrtXXpYY + Y);
	}
	return Position{ Angle::FromRadians(phi), Angle::FromRadians(lambda), static_cast<float>( h ) };
}

bool GIS::Global::GetPointOnTerrain(const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & point)
{
	return tessellator_->GetPointOnTerrain(latitude, longitude, point);
}

float GIS::Global::ComputeElevationAboveSurface(const Position & position)
{
	Urho3D::Vector3 pointOnGlobal;
	Position surfacePosition;
	if (GetPointOnTerrain(position.GetLatitude(), position.GetLongitude(), pointOnGlobal)) {
		surfacePosition = CartesianToGeodetic(pointOnGlobal);
	}
	else {
		surfacePosition = Position {
			position.GetLatitude(), position.GetLongitude(),
			elevationModel_->GetElevation(position.GetLatitude(), position.GetLatitude() ) };
	}
	return position.GetElevation() - surfacePosition.GetElevation();
}

float GIS::Global::ComputeHorizonDistance(float elevation)
{
	if (elevation <= 0)
		return 0;
	return sqrtf(elevation * (2 * equatorialRadius_ + elevation));
}

float GIS::Global::ComputePerspectiveNearDistance(float farDist, float farResolution, uint32_t depthBits)
{
	float maxDepthValue = (1L << depthBits) - 1L;
	return farDist / (maxDepthValue / (1 - farResolution / farDist) - maxDepthValue + 1);
}

auto GIS::Global::GeodeticToCartesian(const Angle & latitude, const Angle & longitude, float elevation) -> Vector4D
{
	float cosLat = cos(latitude.GetRadians());
	float sinLat = sin(latitude.GetRadians());
	float cosLon = cos(longitude.GetRadians());
	float sinLon = sin(longitude.GetRadians());

	double rpm = equatorialRadius_ / sqrtf(1.0f - es_ * sinLat * sinLat);

	double x = (rpm + elevation ) * cosLat * sinLon;
	double y = (rpm * (1.0 - es_) + elevation ) * sinLat;
	double z = (rpm + elevation) * cosLat * cosLon;
	return Vector4D { x, y, z, 1.0f };
}
