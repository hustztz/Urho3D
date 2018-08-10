#pragma once

#include "LatLon.h"
#include "ElevationConfig.h"
#include <string>

namespace GIS
{

	class Level
	{
	public:

		Level() = default;

		Level(ElevationConfig & config);

		int GetLevelNumber() const noexcept
		{
			return levelNumber_;
		}

		LatLon GetTileDelta() const noexcept
		{
			return tileDelta_;
		}

		const std::string & GetLevelName() const noexcept
		{
			return levelName_;
		}

		int GetTileWidth() const noexcept
		{
			return tileWidth_;
		}

		int GetTileHeight() const noexcept
		{
			return tileHeight_;
		}

		const std::string & GetCacheName() const noexcept 
		{
			return dataCacheName_;
		}

		double GetTexelSize() const noexcept 
		{
			return texelSize_;
		}

		const std::string & GetPath() const noexcept 
		{
			return path_;
		}

		const std::string & GetFormatSuffix() const noexcept
		{
			return formatSuffix_;
		}

	private:

		int levelNumber_;

		std::string levelName_;

		LatLon tileDelta_;

		int tileWidth_;

		int tileHeight_;

		std::string dataCacheName_;

		double texelSize_ = 0;

		std::string path_;

		std::string formatSuffix_;

	};

}