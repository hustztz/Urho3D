#include "LevelSet.h"

GIS::LevelSet::LevelSet( GIS::ElevationConfig & config ) 
{
	config.GetValue(ConfigKey::TILE_ORIGIN, &tileOrigin_);
	int numLevels = 0;
	if ( config.GetValue(ConfigKey::NUM_LEVELS, &numLevels)) 
	{
		levels_.reserve(numLevels);
		config.GetValue(ConfigKey::LEVEL_ZERO_TILE_DELTA, &levelZeroTileDelta_ );
		auto lat = levelZeroTileDelta_.GetLatitude();
		auto lon = levelZeroTileDelta_.GetLongitude();
		for (int i = 0; i < numLevels; ++i)
		{
			config.SetValue(ConfigKey::LEVEL_NAME, std::to_string(i));
			config.SetValue(ConfigKey::LEVEL_NUMBER, i);
			config.SetValue(ConfigKey::TILE_DELTA, LatLon{ lat, lon });
			levels_.push_back(Level{ config });
			lat = Angle::FromDegrees(lat.GetDegrees() / 2);
			lon = Angle::FromDegrees(lon.GetDegrees() / 2);
		}
	}
	config.GetValue(ConfigKey::SECTOR, &sector_);
}

const GIS::Sector & GIS::LevelSet::GetSector() const noexcept
{
	return sector_;
}

const GIS::Level & GIS::LevelSet::GetLevel(int index) const
{
	if (index >= 0 && index < levels_.size())
		return levels_[index];
	else
		throw std::out_of_range{ "LevelSet::GetLevel" };
}

GIS::Level & GIS::LevelSet::GetLevel(int index)
{
	return levels_[index];
}

const GIS::LatLon & GIS::LevelSet::GetTileOrigin() const noexcept
{
	return tileOrigin_;
}

GIS::Level & GIS::LevelSet::GetTargetLevel(double targetSize) 
{
	auto & lastLevel = levels_[levels_.size() - 1]; 
	if( lastLevel.GetTexelSize() >= targetSize )
		return lastLevel;
	for (auto & level : levels_)
	{
		if (level.GetTexelSize() <= targetSize)
			return level;
	}
	return lastLevel;
}

uint32_t GIS::LevelSet::GetLevelCount() const noexcept
{
	return static_cast<uint32_t>( levels_.size() );
}

auto GIS::LevelSet::GetLastLevel() const -> const Level &
{
	return levels_.back();
}

auto GIS::LevelSet::GetLastLevel() -> Level &
{
	return levels_.back();
}
