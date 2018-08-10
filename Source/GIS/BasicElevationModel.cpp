#include "BasicElevationModel.h"
#include <set>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <future>
#include <fstream>

GIS::BasicElevationModel::BasicElevationModel(ElevationConfig config)
	: levelSet_( config )
{
	config.GetValue(ConfigKey::DETAIL_HINT, &detailHint_ );
	config.GetValue(ConfigKey::MODEL_STORE_PATH, &modelPath_ );
	config.GetValue(ConfigKey::BYTE_ORDER, &byteOrder_);
	config.GetValue(ConfigKey::DATA_TYPE, &dataType_ );
	config.GetValue(ConfigKey::TILE_ORIGIN, &tileOrigin_ );
	config.GetValue(ConfigKey::ELEVATION_MIN, &elevationMinMax_.first );
	config.GetValue(ConfigKey::ELEVATION_MAX, &elevationMinMax_.second);
	
	if (std::string extremeElevationFilename; config.GetValue(ConfigKey::ELEVATION_EXTREMES_FILE, &extremeElevationFilename))
	{
		LoadExtremeElevations( extremeElevationFilename );
	}
}

void GIS::BasicElevationModel::SetDetailHint(float detailHint) 
{
	detailHint_ = detailHint;
}

auto GIS::BasicElevationModel::GetElevations(const Sector & requestedSector, const LevelSet & levelSet, int targetLevelNumber) -> std::shared_ptr<Elevations>
{
	while (loadedTileTasks_.size())
	{
		if (loadedTileTasks_.front()._Is_ready()) {
			auto task = std::move( loadedTileTasks_.front() );
			loadedTileTasks_.pop();
			AddTile(task.get());
		}
	}

	Sector sector;
	requestedSector.Intersect(levelSet.GetSector(), &sector);
	auto & targetLevel = levelSet.GetLevel(targetLevelNumber);
	auto origin = levelSet.GetTileOrigin();
	auto delta = targetLevel.GetTileDelta();
	int nwRow = Tile::ComputeRow(delta.GetLatitude(), sector.GetMaxLatitude(), origin.GetLatitude());
	int nwCol = Tile::ComputeCol(delta.GetLongitude(), sector.GetMinLongitude(), origin.GetLongitude());
	int seRow = Tile::ComputeRow(delta.GetLatitude(), sector.GetMinLatitude(), origin.GetLatitude());
	int seCol = Tile::ComputeCol(delta.GetLongitude(), sector.GetMaxLongitude(), origin.GetLongitude());
	
	Elevations::Tiles tiles;

	std::unordered_set<TileKey> requested;
	bool missLevelZeroTiles = false, missTargetTile = false;
	for (int row = seRow; row <= nwRow; ++row) 
	{
		for (int col = nwCol; col <= seCol; ++col) 
		{
			TileKey key{ targetLevelNumber, row, col, targetLevel.GetCacheName() };
			auto tile = GetTileFromMemory(key);
			if (tile) {
				tiles.insert(tile);
				continue;
			}
			RequestTile(key);
			continue;

			TileKey fallbackKey, fallbackRequest;

			// Fall back
			int fallbackRow = row;
			int fallbackCol = col;
			for (int fallbackLevelNumber = key.GetLevelNumber() - 1; fallbackLevelNumber >= 0; --fallbackLevelNumber)
			{
				fallbackCol /= 2;
				fallbackRow /= 2;
				fallbackKey = TileKey(fallbackLevelNumber, fallbackRow, fallbackCol, levelSet.GetLevel(fallbackLevelNumber).GetCacheName());
				tile = GetTileFromMemory(fallbackKey);
				if (tile) {
					tiles.insert(tile);
					break;
				}
				else {
					if (fallbackLevelNumber == 0)
						missLevelZeroTiles = true;
					fallbackRequest = fallbackKey;
				}
			}
			if (fallbackRequest.GetCacheName().size()) {
				if (requested.insert(fallbackRequest).second) {
					RequestTile(fallbackKey);
				}
			}
		}

	}

	std::shared_ptr<Elevations> elevations{ new Elevations { *this } };

	if (missLevelZeroTiles || tiles.empty()) {
		elevations->SetTexelSize(FLT_MAX);
		
	}
	else {
		elevations->SetTexelSize(levelSet.GetLevel((*tiles.rbegin())->GetLevelNumber()).GetTexelSize());
	}
	elevations->SetTiles(tiles);
	return elevations;
}

double GIS::BasicElevationModel::GetBestResolution() const noexcept
{
	return levelSet_.GetLastLevel().GetTexelSize();
}

void GIS::BasicElevationModel::LoadAllLevelZeroTiles()
{
	auto & levelZero = levelSet_.GetLevel(0);
	auto tileDalta = levelZero.GetTileDelta();
	auto latDelta = tileDalta.GetLatitude();
	auto lonDelta = tileDalta.GetLongitude();

	auto sector = levelSet_.GetSector();

	int rows = sector.GetDeltaLat().GetDegrees() / latDelta.GetDegrees();
	int cols = sector.GetDeltaLon().GetDegrees() / lonDelta.GetDegrees();

	for (int row = 0; row < rows; ++row) {

		for (int col = 0; col < cols; ++col) {

			RequestTile( TileKey(0, row, col, levelZero.GetCacheName()) );
		}

	}

	while (loadedTileTasks_.size())
	{
		auto task = std::move(loadedTileTasks_.front());
		task.wait();
		loadedTileTasks_.pop();
		AddTile(task.get());
	}

}

std::pair<float, float> GIS::BasicElevationModel::GetExtremeElevations(const Sector & sector) {
	if (extremeLevel_ < 0 || extremes_.size() == 0)
		return { elevationMinMax_.first, elevationMinMax_.second };
	SectorKey sectorKey{ sector };
	if (auto iter = m_ExtremesCache.find(sectorKey); iter != m_ExtremesCache.end()) {
		return iter->second;
	}
	return m_ExtremesCache[sectorKey] = ComputeExtrameElevations(sector);
}

std::pair<float, float> GIS::BasicElevationModel::GetExtremeElevations(const Angle & latitude, const Angle & longitude) const noexcept
{

	if (extremeLevel_ < 0 || extremes_.empty())
		return { elevationMinMax_ };
	LatLon delta = levelSet_.GetLevel(extremeLevel_).GetTileDelta();
	LatLon origin = levelSet_.GetTileOrigin();
	const int row = ElevationTile::ComputeRow(delta.GetLatitude(), latitude, origin.GetLatitude());
	const int col = ElevationTile::ComputeCol(delta.GetLongitude(), longitude, origin.GetLongitude());

	const int nCols = ElevationTile::ComputeCol(delta.GetLongitude(), Angle::FromDegrees( 180 ), Angle::FromDegrees( -180 ) ) + 1;

	int index = 2 * (row * nCols + col);
	double min = extremes_[index];
	double max = extremes_[index + 1];

	if (min == GetMissingDataSignal())
		min = GetMissingDataReplacement();
	if (max == GetMissingDataSignal())
		max = GetMissingDataReplacement();

	return std::make_pair( static_cast<float>( min ), static_cast<float>( max ) );
}

std::pair<float, float> GIS::BasicElevationModel::GetMaxMinElevation() const noexcept
{
	return elevationMinMax_;
}

std::vector<float> GIS::BasicElevationModel::GetElevations(const Sector & sector, std::vector<LatLon> const & latlons, float targetResolution, bool mapMissingData)
{
	std::vector<float> result;
	result.reserve(latlons.size());
	Level & level = levelSet_.GetTargetLevel(targetResolution);
	auto elevation = GetElevations(sector, levelSet_, level.GetLevelNumber());
	for (auto latlon : latlons) 
	{
		auto elev = elevation->GetElevation(latlon);
		if (elev != GetMissingDataSignal()) 
		{
			result.push_back(elev);
		}
		else {
			result.push_back(missingSignalRep_);
		}
	}
	return result;
}

bool GIS::BasicElevationModel::IsContain(const Angle & latitude, const Angle & longitude) const noexcept
{
	return levelSet_.GetSector().IsContain(latitude, longitude);
}

float GIS::BasicElevationModel::GetUnmappedElevation(const Angle & latitude, const Angle & longitude) const noexcept
{
	if( !IsContain( latitude, longitude ) )
		return GetMissingDataSignal();
	auto & lastLevel = levelSet_.GetLastLevel();
	auto tileKey = TileKey(latitude, longitude, levelSet_, lastLevel.GetLevelNumber());
	auto tile = GetTileFromMemory(tileKey);
	if (!tile) {
		int fallbackRow = tileKey.GetRow();
		int fallbackCol = tileKey.GetCol();
		for (int fallbackLevelNum = tileKey.GetLevelNumber() - 1; fallbackLevelNum >= 0; fallbackLevelNum--)
		{
			fallbackRow /= 2;
			fallbackCol /= 2;

			TileKey fallbackKey (fallbackLevelNum, fallbackRow, fallbackCol,
				levelSet_.GetLevel(fallbackLevelNum).GetCacheName());
			tile = GetTileFromMemory(fallbackKey);
			if (tile != nullptr )
				break;
		}
	}
	if (tile == nullptr)
	{
		// Request the level-zero tile since it's not in memory
		auto & firstLevel = levelSet_.GetLevel(0);
		TileKey zeroKey(latitude, longitude, levelSet_, firstLevel.GetLevelNumber());
		const_cast<BasicElevationModel*>(this)->RequestTile(zeroKey);

		// Return the best we know about the location's elevation
		return GetExtremeElevations(latitude, longitude).first;
	}
	return tile->LookUpElevation(latitude, longitude);
}

float GIS::BasicElevationModel::GetElevation(const Angle & latitude, const Angle & longitude) const noexcept
{
	auto e = GetUnmappedElevation( latitude, longitude );
	return e == GetMissingDataSignal() ? GetMissingDataReplacement() : e;
}

float GIS::BasicElevationModel::GetMissingDataReplacement() const noexcept
{
	return 0.0f;
}

std::pair<float, float> GIS::BasicElevationModel::ComputeExtrameElevations(const Sector & sector) const noexcept
{
	auto & level = levelSet_.GetLevel(extremeLevel_);
	auto tileDelta = level.GetTileDelta();
	auto tileOri = levelSet_.GetTileOrigin();
	auto maxRow = Tile::ComputeRow(tileDelta.GetLatitude(), sector.GetMaxLatitude(), tileOri.GetLatitude());
	auto minRow = Tile::ComputeRow(tileDelta.GetLatitude(), sector.GetMinLatitude(), tileOri.GetLatitude());
	auto maxCol = Tile::ComputeCol(tileDelta.GetLongitude(), sector.GetMaxLongitude(), tileOri.GetLongitude());
	auto minCol = Tile::ComputeCol(tileDelta.GetLongitude(), sector.GetMinLongitude(), tileOri.GetLongitude());
	float minElevation = FLT_MAX;
	float maxElevation = -FLT_MAX;
	uint32_t nCols = Tile::ComputeCol(tileDelta.GetLongitude(), Angle::FromDegrees(180), Angle::FromDegrees(-180)) + 1;
	for (int row = minRow; row <= maxRow; ++row) {
		for (int col = minCol; col <= maxCol; ++col) {
			int index = 2 * (row * nCols + col);
			float a = extremes_[index];
			float b = extremes_[index];
			if (abs( a - GetMissingDataSignal() ) < 0.00001f ) {
				a = missingSignalRep_;
			}
			if ((abs(b - GetMissingDataSignal()) < 0.00001F)) {
				b = missingSignalRep_;
			}
			minElevation = std::min(minElevation, std::min(a, b));
			maxElevation = std::max(maxElevation, std::max(a, b));
		}
	}
	if (abs(minElevation - FLT_MAX) < 0.00001f)
		minElevation = elevationMinMax_.first;
	if (abs(maxElevation + FLT_MAX) < 0.00001f)
		maxElevation = elevationMinMax_.second;
	return { minElevation, maxElevation };
}

auto GIS::BasicElevationModel::GetTileFromMemory(const TileKey & key) const noexcept -> std::shared_ptr<ElevationTile>
{
	if (key.GetLevelNumber() == 0)
	{
		auto tileIter = levelZeroTiles_.find(key);
		return (tileIter != levelZeroTiles_.end()) ? tileIter->second : std::shared_ptr<ElevationTile>{};
	}
	else
	{
		return memoryCache_.Get(key);
	}
}

namespace 
{

	bool FileExists(const std::string & fileName) 
	{
		std::ifstream input{ fileName };
		return input.good();
	}

	template <typename T>
	std::vector<T> ReadDataFromFile(const std::string & filename)
	{
		std::vector<T> result;
		std::ifstream input{ filename, std::ios::binary };
		if (input.is_open()) 
		{
			input.seekg(0, std::ios::end);
			auto sz = input.tellg() / sizeof(T);
			result.resize( sz );
			input.seekg(0, std::ios::beg);
			input.read(reinterpret_cast<char*>(&result[0]), sz);
			input.close();
		}
		return result;
	}

	template <typename T>
	std::vector<float> ConvertElevationData(std::vector<T> & source)
	{
		std::vector<float> result;
		result.reserve(source.size());
		std::transform(source.begin(), source.end(), std::back_inserter(result),
			[](const T & sour) -> float
			{
				return static_cast<float>( sour);
			}
		);
		return result;
	}

}

void GIS::BasicElevationModel::RequestTile(const TileKey & tileKey) 
{
	if (!requestingTiles_.Insert(tileKey))
		return;

	auto requestTileTask = [this, tileKey]() -> std::shared_ptr<ElevationTile>
	{
		auto & level = levelSet_.GetLevel(tileKey.GetLevelNumber());
		auto path = modelPath_ + "/" + level.GetPath() + "/" + tileKey.GetPathInLevel() + level.GetFormatSuffix();

		if (!FileExists(path)) 
		{
			return {};
		}
		std::vector<float> elevationData;
		switch (dataType_) 
		{
		case DataType::Int16:
			elevationData = ConvertElevationData( ReadDataFromFile<int16_t>(path) );
			break;
		case DataType::Int32:
			elevationData = ConvertElevationData(ReadDataFromFile<int32_t>(path));
			break;
		case DataType::Float32:
			elevationData = ReadDataFromFile<float>(path);
			break;
		case DataType::Float64:
			elevationData = ConvertElevationData(ReadDataFromFile<double>(path));
			break;
		default:
			throw std::runtime_error{ "Not implement" };
		}
		if (byteOrder_ != ByteOrder::LittleEndian)
			throw std::runtime_error{ "Not implement" };
		auto tile = CreateTile(tileKey);
		auto tileKeyLevelNumber = tileKey.GetLevelNumber();
		auto levelNumber = tile->GetLevelNumber();
		tile->SetElevationData(std::move( elevationData) );
		return tile;
	};
	requestTileTask();
	loadedTileTasks_.push( std::async(std::launch::async | std::launch::deferred, requestTileTask ) );
}

auto GIS::BasicElevationModel::CreateTile(const TileKey & tileKey) -> std::shared_ptr<ElevationTile>
{
	auto levelNumber = tileKey.GetLevelNumber();
	auto & level = levelSet_.GetLevel(levelNumber);
	auto lat = Tile::ComputeRowLatitude( tileKey.GetRow(), level.GetTileDelta().GetLatitude(), tileOrigin_.GetLatitude() );
	auto lon = Tile::ComputeColLongitude(tileKey.GetCol(), level.GetTileDelta().GetLongitude(), tileOrigin_.GetLongitude());
	Sector sector { lat, lat + level.GetTileDelta().GetLatitude(), lon, lon + level.GetTileDelta().GetLongitude() };
	return std::make_shared<ElevationTile>( *this, sector, tileKey.GetRow(), tileKey.GetCol(), levelNumber, level.GetPath() );
}

void GIS::BasicElevationModel::AddTile(const std::shared_ptr<ElevationTile>& tile)
{
	if (!tile)
		return;
	auto levelNumber = tile->GetLevelNumber();
	TileKey k{ levelNumber, tile->GetRow(), tile->GetCol(), levelSet_.GetLevel(levelNumber).GetCacheName() };
	if (0 == levelNumber ) 
	{
		levelZeroTiles_[k] = tile;
	}
	else
	{
		memoryCache_.Put(k, tile);
	}
	requestingTiles_.Remove(k);
}

namespace {

	template <typename T>
	void EndSwap(T *data) {
		static_assert(std::is_pod<T>::value, "T must be a POD type");
		auto byteData = reinterpret_cast<uint8_t*>(data);
		std::reverse(byteData, byteData + sizeof(T));
	}

}

void GIS::BasicElevationModel::LoadExtremeElevations(const std::string & filename) {
	std::ifstream input{ filename, std::ios::binary };
	if (!input.is_open())
		return;
	input.seekg(0, std::ios::end);
	std::size_t elementCount = input.tellg();
	extremes_.clear();
	extremes_.reserve(elementCount);
	input.seekg(0, std::ios::beg);
	auto nameComponent = filename.substr( 0, filename.find_last_of('.') );
	auto levelNumberString = nameComponent.substr( nameComponent.find_last_of('_') + 1 );
	try {
		extremeLevel_ = std::stoi(levelNumberString);
		do {
			uint16_t elevation;
			if (input.read(reinterpret_cast<char*>(&elevation), sizeof(elevation))) {
				EndSwap(&elevation);
				extremes_.push_back(elevation);
			}
			else {
				break;
			}
		} while (true);
	}
	catch (...) {
		return;
	}
}

float GIS::BasicElevationModel::ElevationTile::LookUpElevation(const Angle & latitude, const Angle & longitude) const noexcept
{
	auto & level = model_.levelSet_.GetLevel(GetLevelNumber());
	int tileHeight = level.GetTileHeight();
	int tileWidth = level.GetTileWidth();
	double sectorDeltaLat = GetSector().GetDeltaLat().GetRadians();
	double sectorDeltaLon = GetSector().GetDeltaLon().GetRadians();
	double dLat = GetSector().GetMaxLatitude().GetRadians() - latitude.GetRadians();
	double dLon = longitude.GetRadians() - GetSector().GetMinLongitude().GetRadians();
	double sLat = dLat / sectorDeltaLat;
	double sLon = dLon / sectorDeltaLon;

	int j = (int)((tileHeight - 1) * sLat);
	int i = (int)((tileWidth - 1) * sLon);
	int k = j * tileWidth + i;

	double eLeft = elevationData_[k];
	double eRight = i < (tileWidth - 1) ? elevationData_[k + 1] : eLeft;

	if (model_.GetMissingDataSignal() == eLeft || model_.GetMissingDataSignal() == eRight)
		return model_.GetMissingDataSignal();

	double dw = sectorDeltaLon / (tileWidth - 1);
	double dh = sectorDeltaLat / (tileHeight - 1);
	double ssLon = (dLon - i * dw) / dw;
	double ssLat = (dLat - j * dh) / dh;

	double eTop = eLeft + ssLon * (eRight - eLeft);
	if (j < tileHeight - 1 && i < tileWidth - 1)
	{
		eLeft = elevationData_[k + tileWidth];
		eRight = elevationData_[k + tileWidth + 1];

		if ( model_.GetMissingDataSignal() == eLeft || model_.GetMissingDataSignal() == eRight)
			return model_.GetMissingDataSignal();
	}
	double eBot = eLeft + ssLon * (eRight - eLeft);
	return eTop + ssLat * (eBot - eTop);
}
