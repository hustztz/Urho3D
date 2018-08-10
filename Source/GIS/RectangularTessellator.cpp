#include "RectangularTessellator.h"
#include "BasicElevationModel.h"
#include "Global.h"

namespace 
{

	const uint32_t DEFAULT_DENSITY = 5;
	const uint32_t DEFAULT_NUM_LAT_SUBDIVISIONS = 3;
	const uint32_t DEFAULT_NUM_LON_SUBDIVISIONS = 3;

}

std::unordered_map<int, std::shared_ptr<std::vector<uint32_t>>> GIS::RectangularTessellator::ms_IndicesMap;

std::unordered_map<int, std::shared_ptr<std::vector<Urho3D::Vector2>>> GIS::RectangularTessellator::ms_TexCoordsMap;

GIS::RectangularTessellator::RectangularTessellator(Global & global, ElevationConfig & config)
	: density_(DEFAULT_DENSITY), levelSet_( config ), global_( global ) 
{
	numLevel0LatSubdivision = DEFAULT_NUM_LAT_SUBDIVISIONS;
	numLevel0LonSubdivision = DEFAULT_NUM_LON_SUBDIVISIONS;
	m_MaxLevel = levelSet_.GetLevelCount() - 1;
}

auto GIS::RectangularTessellator::Tessellate( const RectTileTessellateParams & params ) -> const std::vector<std::shared_ptr<RectTile>>&
{
	currentTiles_.clear();
	if (!topLevelTiles_.size()) 
	{
		CreateTopLevelTiles();
	}
	for (auto tile : topLevelTiles_) 
	{
		SelectVisibleTiles(params, tile);
	}
	for (auto tile : currentTiles_) 
	{
		if (!tile->GetRenderInfo())
			CheckVertexData(*tile);
	}
	return currentTiles_;
}

void GIS::RectangularTessellator::CreateTopLevelTiles() 
{
	topLevelTiles_.clear();
	float deltaLat = 180.0f / numLevel0LatSubdivision;
	float deltaLon = 360.0f / numLevel0LonSubdivision;
	Angle lastLat = Angle::FromDegrees(-90);
	for (uint32_t row = 0; row < numLevel0LatSubdivision; ++row)
	{
		Angle lat = lastLat + Angle::FromDegrees( deltaLat );
		if (lat.GetDegrees() + 1 > 90)
			lat = Angle::FromDegrees(90);
		Angle lastLon = Angle::FromDegrees(-180);
		for (uint32_t col = 0; col < numLevel0LonSubdivision; ++col)
		{
			Angle lon = lastLon + Angle::FromDegrees(deltaLon);
			if (lon.GetDegrees() + 1 > 180)
				lon = Angle::FromDegrees(180);
			Sector tileSector{ lastLat, lat, lastLon, lon };
			topLevelTiles_.push_back(CreateTile( tileSector, 0 ));
			lastLon = lon;
		}
		lastLat = lat;
	}
}

void GIS::RectangularTessellator::SelectVisibleTiles(const RectTileTessellateParams & params, std::shared_ptr<RectTile> tile)
{
	if (params.frustum.IsInsideFast(tile->GetBBox())) 
	{
		if( tile->GetLevelNumber() < m_MaxLevel && !AtBestResolution(*tile) && NeedToSplit(params, *tile))
		{

			for (auto sector : tile->GetSector().Subdivide())
			{
				auto subTile = CreateTile(sector, tile->GetLevelNumber() + 1);
				SelectVisibleTiles(params, subTile);
			}
		}
		else {
			currentTiles_.push_back(tile);
		}
	}
}

bool GIS::RectangularTessellator::AtBestResolution(const RectTile & rectTile)
{
	auto bestResolution = global_.GetElevationModel().GetBestResolution();

	auto cellSize = rectTile.GetCellSize();

	return bestResolution >= cellSize;
}

bool GIS::RectangularTessellator::NeedToSplit(const RectTileTessellateParams & param, const RectTile & tile)
{
	double cellSizeRadians = tile.GetCellSize();
	double cellSizeMeters = global_.GetRadius() * cellSizeRadians;
	double detailScale = pow(10, -(0.3f + 1.0f));
	double fovScale = std::clamp(tanf(param.fov / 2) / tanf(PI / 8), 0.0f, 1.0f);
	auto tileCenter = tile.GetRefCenter();
	Urho3D::Vector3 tileCenterf;
	tileCenterf.x_ = tileCenter.x;
	tileCenterf.y_ = tileCenter.y;
	tileCenterf.z_ = tileCenter.z;
	double scaledDistance = tileCenterf.DistanceToPoint(param.eyePosition) * fovScale * detailScale;
	return cellSizeMeters > scaledDistance;
}

/**
 * 创建新的RectTile，设置RectTile的bounding box、ref center.
 */
auto GIS::RectangularTessellator::CreateTile(const Sector & sector, int level) -> std::shared_ptr<RectTile> 
{
	auto tile = std::make_shared<RectTile>(sector, level, density_, BBox {} );
	if (auto iter = m_BBoxMap.find(sector); iter != m_BBoxMap.end()) {
		tile->SetBoundingBox(iter->second);
	}
	else {
		CheckVertexData( *tile );
		m_BBoxMap[SectorKey{ sector }] = tile->GetBBox();
	}
	auto centroid = sector.GetCentroid();
	auto refCenter = global_.ComputePointFromPosition(centroid.GetLatitude(), centroid.GetLongitude(), 0);
	tile->SetRefCenter(refCenter);
	return tile;
}

void GIS::RectangularTessellator::CheckVertexData(RectTile & tile) 
{
	if (tile.GetRenderInfo() == nullptr) 
	{
		RenderInfo::Key key{ tile.GetDensity(), tile.GetSector() };
		auto result = m_RenderInfoMap.find(key);
		if (result == m_RenderInfoMap.end()) {
			CreateTileVertexData(tile);
		}
		else {
			tile.SetRenderInfo(result->second);
		}
	}
}

/**
 * 构造RectTile的顶点数据
 */
void GIS::RectangularTessellator::CreateTileVertexData(RectTile & tile) 
{

	int density = tile.GetDensity();

	int numVertices = (density + 3) * (density + 3);

	std::vector<Point3> verts;
	verts.reserve(numVertices);
	std::vector<Point3> norms;
	norms.reserve(numVertices);

	auto tileLocations = ComputeLocation( tile );

	auto elevationData = global_.GetElevationModel().GetElevations(tile.GetSector(), tileLocations, tile.GetResolution(), true);

	auto centroid = tile.GetSector().GetCentroid();
	auto refCenter = global_.ComputePointFromPosition(centroid.GetLatitude(), centroid.GetLongitude(), 0);

	int ie = 0;
	int iv = 0;
	auto latlonIter = tileLocations.begin();
	BBox bbox;
	for (int j = 0; j <= density + 2; ++j) 
	{
		for (int i = 0; i <= density + 2; ++i)
		{
			const auto & latlon = *latlonIter;
			float elevation = elevationData[ie++];
			if (j == 0 || i == 0 || i == density + 2 || j == density + 2)
			elevation = 0;
			auto p = global_.ComputePointFromPosition(latlon.GetLatitude(), latlon.GetLongitude(), elevation );
			Urho3D::Vector3 pos;
			pos.x_ = p.x;
			pos.y_ = p.y;
			pos.z_ = p.z;
			bbox.Merge(pos);
			verts.push_back(pos);
			latlonIter++;
			norms.push_back( pos.Normalized() );
		}
	}
	
	std::shared_ptr<RenderInfo> ri{ new RenderInfo { tile.GetSector(), tile.GetDensity() } };
	ri->SetVertices(std::move(verts));
	ri->SetNormals(std::move(norms));
	tile.SetBoundingBox(bbox);
	ri->SetBBox(bbox);
	m_RenderInfoMap[RenderInfo::Key{ tile.GetDensity(), tile.GetSector() }] = ri;
	if (auto fr = ms_IndicesMap.find(tile.GetDensity()); fr != ms_IndicesMap.end())
	{
		ri->SetIndices(fr->second);
	}
	else 
	{
		std::shared_ptr<std::vector<uint32_t>> mem { new std::vector<uint32_t>( ComputeIndices( tile.GetDensity() ) ) };
		ri->SetIndices(mem);
		ms_IndicesMap[tile.GetDensity()] = mem;
	}

	if (auto fr = ms_TexCoordsMap.find(tile.GetDensity()); fr != ms_TexCoordsMap.end())
	{
		ri->SetTexCoords(fr->second);
	}
	else 
	{
		auto mem = std::make_shared<std::vector<Urho3D::Vector2>>(ComputeTextureTexCoords(tile.GetDensity()));
		ri->SetTexCoords(mem);
		ms_TexCoordsMap[tile.GetDensity()] = mem;
	}
	tile.SetRenderInfo(ri);
	tile.SetRefCenter(refCenter);
}

std::vector<GIS::LatLon> GIS::RectangularTessellator::ComputeLocation(const RectTile & rectTile)
{
	std::vector<LatLon> latlons;
	int density = rectTile.GetDensity();
	int numVertices = (density + 3) * (density + 3);
	latlons.reserve(numVertices);

	Angle latMax = rectTile.GetSector().GetMaxLatitude();
	Angle dLat = rectTile.GetSector().GetDeltaLat() / density;
	Angle lat = rectTile.GetSector().GetMinLatitude();

	Angle lonMin = rectTile.GetSector().GetMinLongitude();
	Angle lonMax = rectTile.GetSector().GetMaxLongitude();
	Angle dLon = rectTile.GetSector().GetDeltaLon() / density;

	
	for (int j = 0; j <= density + 2; j++)
	{
		Angle lon = lonMin;
		for (int i = 0; i <= density + 2; i++)
		{
			latlons.push_back(LatLon(lat, lon));

			if (i > density)
				lon = lonMax;
			else if (i != 0)
				lon = lon + (dLon);

			if (lon.GetDegrees() < -180)
				lon = Angle::FromDegrees(-180);
			else if (lon.GetDegrees() > 180)
				lon = Angle::FromDegrees(180);
		}

		if (j > density)
			lat = latMax;
		else if (j != 0)
			lat = lat + dLat;
	}
	return latlons;
}

std::vector<Urho3D::Vector2> GIS::RectangularTessellator::ComputeTextureTexCoords(int density)
{
	const float one = 0.999999f;

	int coordCount = (density + 3) * (density + 3);
	std::vector<Urho3D::Vector2> buffer;
	buffer.resize( coordCount );

	float *p = reinterpret_cast<float*>( buffer.data() );

	float delta = 1.0f / density;
	int k = 2 * (density + 3);

	for (int j = 0; j < density; j++)
	{
		double v = j * delta;

		// skirt column; duplicate first column
		p[k++] = 0;
		p[k++] = (float)v;

		// interior columns
		for (int i = 0; i < density; i++)
		{
			p[k++] = (float)(i * delta); // u
			p[k++] = (float)v;
		}

		// last interior column; force u to 1.
		p[k++] = one;//1d);
		p[k++] = (float)v;

		// skirt column; duplicate previous column
		p[k++] = one;//1d);
		p[k++] = (float)v;
	}

	// Last interior row
	//noinspection UnnecessaryLocalVariable
	float v = one;//1d;
	p[k++] = 0; // skirt column
	p[k++] = v;

	for (int i = 0; i < density; i++)
	{
		p[k++] = (float)(i * delta); // u
		p[k++] = v;
	}
	p[k++] = one;//1d); // last interior column
	p[k++] = v;

	p[k++] = one;//1d); // skirt column
	p[k++] = v;

	// last skirt row
	int kk = k - 2 * (density + 3);
	for (int i = 0; i < density + 3; i++)
	{
		p[k++] = p[kk++];
		p[k++] = p[kk++];
	}

	// first skirt row
	k = 0;
	kk = 2 * (density + 3);
	for (int i = 0; i < density + 3; i++)
	{
		p[k++] = p[kk++];
		p[k++] = p[kk++];
	}

	return buffer;
}

std::vector<uint32_t> GIS::RectangularTessellator::ComputeIndices(int density) 
{
	int sideSize = density + 2;
	int indexCount = 2 * sideSize * sideSize + 4 * sideSize - 2;
	std::vector<uint32_t> buffer;
	int k = 0;
	for (int i = 0; i < sideSize; i++)
	{
		buffer.push_back(k);
		if (i > 0)
		{
			buffer.push_back(++k);
			buffer.push_back(k);
		}

		if (i % 2 == 0) // even
		{
			buffer.push_back(++k);
			for (int j = 0; j < sideSize; j++)
			{
				k += sideSize;
				buffer.push_back(k);
				buffer.push_back(++k);
			}
		}
		else // odd
		{
			buffer.push_back(--k);
			for (int j = 0; j < sideSize; j++)
			{
				k -= sideSize;
				buffer.push_back(k);
				buffer.push_back(--k);
			}
		}
	}
	return buffer;
}

bool GIS::RectangularTessellator::GetPointOnTerrain(const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & point)
{
	for (auto tile : currentTiles_) {
		if (tile->GetSector().IsContain(latitude, longitude)) {
			if (tile->GetSurfacePoint(latitude, longitude, point))
				return true;
		}
	}
	return false;
}

namespace 
{
	Urho3D::Vector3 interpolate(
			const Urho3D::Vector3 & bL, 
			const Urho3D::Vector3 & bR, 
			const Urho3D::Vector3 & tR, 
			const Urho3D::Vector3 & tL, 
			double xDec, 
			double yDec)
	{
		double pos = xDec + yDec;
		if (pos == 1)
		{
			// on the diagonal - what's more, we don't need to do any "oneMinusT" calculation
			return Urho3D::Vector3(
				tL.x_ * yDec + bR.x_ * xDec,
				tL.y_ * yDec + bR.y_ * xDec,
				tL.z_ * yDec + bR.z_ * xDec);
		}
		else if (pos > 1)
		{
			// in the "top right" half

			// vectors pointing from top right towards the point we want (can be thought of as "negative" vectors)
			Urho3D::Vector3 horizontalVector = (tL - tR) * (1 - xDec);
			Urho3D::Vector3 verticalVector = (bR - tR) * (1 - yDec);

			return tR + horizontalVector + verticalVector;
		}
		else
		{
			// pos < 1 - in the "bottom left" half

			// vectors pointing from the bottom left towards the point we want
			Urho3D::Vector3 horizontalVector = (bR - bL) * (xDec);
			Urho3D::Vector3 verticalVector = (tL - bL) * (yDec);

			return bL + horizontalVector + verticalVector;
		}
	}
}
Urho3D::Vector3 GIS::RectangularTessellator::RenderInfo::Interpolate(int row, int column, float xDec, float yDec) const
{
	row++;
	column++;

	int numVerticesPerEdge = m_Density + 3;

	int bottomLeft = row * numVerticesPerEdge + column;

	bottomLeft *= 3;

	int numVertsTimesThree = numVerticesPerEdge;

	//        double[] a = new double[6];
	//        ri.vertices.get(a);
	//        Vec4 bL = new Vec4(a[0], a[1], a[2]);
	//        Vec4 bR = new Vec4(a[3], a[4], a[5]);
	auto bL = Urho3D::Vector3(m_Vertices[bottomLeft]);
	auto bR = Urho3D::Vector3(m_Vertices[bottomLeft + 1]);

	bottomLeft += numVertsTimesThree;

	//        ri.vertices.get(a);
	//        Vec4 tL = new Vec4(a[0], a[1], a[2]);
	//        Vec4 tR = new Vec4(a[3], a[4], a[5]);
	auto tL = Urho3D::Vector3(m_Vertices[bottomLeft]);
	auto tR = Urho3D::Vector3(m_Vertices[bottomLeft + 1]);
	return interpolate(bL, bR, tR, tL, xDec, yDec);
}

namespace {

	float createPosition(int start, float decimal, int density)
	{
		float l = ((float)start) / (float)density;
		float r = ((float)(start + 1)) / (float)density;
		return (decimal - l) / (r - l);
	}

}

bool GIS::RectangularTessellator::RectTile::GetSurfacePoint(const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & result)
{
	if (!sector_.IsContain(latitude, longitude))
		return false;
	double lat = latitude.GetDegrees();
	double lon = longitude.GetDegrees();

	double bottom = sector_.GetMinLatitude().GetDegrees();
	double top = sector_.GetMaxLatitude().GetDegrees();
	double left = sector_.GetMinLongitude().GetDegrees();
	double right = sector_.GetMaxLongitude().GetDegrees();

	double leftDecimal = (lon - left) / (right - left);
	double bottomDecimal = (lat - bottom) / (top - bottom);

	int row = (int)(bottomDecimal * (density_));
	int column = (int)(leftDecimal * (density_));

	double l = createPosition(column, leftDecimal, renderInfo_->GetDensity() );
	double h = createPosition(row, bottomDecimal, renderInfo_->GetDensity());

	result = renderInfo_->Interpolate( row, column, l, h );
	//result = result.add3(tile.ri.referenceCenter);

	return true;

}
