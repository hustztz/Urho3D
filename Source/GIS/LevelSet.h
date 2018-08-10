#pragma once

#include "ElevationConfig.h"
#include "Level.h"
#include "Sector.h"
#include "LatLon.h"
#include <vector>

namespace GIS
{

	class LevelSet 
	{
	public:

		LevelSet(ElevationConfig & config);

		const Sector & GetSector() const noexcept;

		const Level & GetLevel( int index ) const;

		Level & GetLevel(int index);

		const LatLon & GetTileOrigin() const noexcept;

		Level & GetTargetLevel(double targetSize);

		uint32_t GetLevelCount() const noexcept;

		const Level & GetLastLevel() const;

		Level & GetLastLevel();

	private:

		std::vector<Level> levels_;

		LatLon tileOrigin_;

		LatLon levelZeroTileDelta_;

		Sector sector_;

	};

}