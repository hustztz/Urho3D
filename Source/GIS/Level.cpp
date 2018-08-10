#include "Level.h"
#include <sstream>
#include <iostream>
#include <iomanip>

GIS::Level::Level(GIS::ElevationConfig & config) {
	config.GetValue( ConfigKey::LEVEL_NUMBER, &levelNumber_);
	config.GetValue( ConfigKey::LEVEL_NAME, &levelName_);
	config.GetValue( ConfigKey::TILE_WIDTH, &tileWidth_);
	config.GetValue( ConfigKey::TILE_HEIGHT, &tileHeight_);
	config.GetValue( ConfigKey::TILE_DELTA, &tileDelta_);
	config.GetValue( ConfigKey::DATA_CACHE_NAME, &dataCacheName_);
	config.GetValue( ConfigKey::FORMAT_SUFFIX, &formatSuffix_);
	texelSize_ = static_cast<double>( tileDelta_.GetLatitude().GetRadians() ) / tileHeight_;
	path_ = dataCacheName_ + "/" + levelName_;
}