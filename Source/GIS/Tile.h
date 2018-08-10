#pragma once

#include "Angle.h"
#include "Sector.h"
#include "Level.h"
#include "TileKey.h"
#include <cassert>

namespace GIS 
{

	class Tile
	{
	public:

		const Sector & GetSector() const noexcept;

		int GetRow() const noexcept;

		int GetCol() const noexcept;

		int GetLevelNumber() const noexcept;

		const std::string & GetPath() const noexcept;

		static std::string GetPath(LevelSet & levelSet);

		static int ComputeRow(const Angle & delta, const Angle & latitude, const Angle & origin)
		{
			assert(delta.GetDegrees() > 0 && latitude.GetDegrees() >= -90.0f && latitude.GetDegrees() <= 90.0f);
			int row = (int)(latitude.GetDegrees() - origin.GetDegrees()) / delta.GetDegrees();
			if (latitude.GetDegrees() - origin.GetDegrees() == 180.0f)
				row = row - 1;
			return row;
		}

		static int ComputeCol(const Angle & delta, const Angle & longitude, const Angle & origin)
		{
			assert(delta.GetDegrees() > 0 && longitude.GetDegrees() >= -180.0f && longitude.GetDegrees() <= 180.0f);
			auto gridLongitude = longitude.GetDegrees() - origin.GetDegrees();
			if (gridLongitude < 0)
				gridLongitude += 360;
			int col = gridLongitude / delta.GetDegrees();
			if (longitude.GetDegrees() - origin.GetDegrees() == 360)
				col = col - 1;
			return col;
		}

		static Angle ComputeRowLatitude(int row, const Angle & delta, Angle origin) {
			assert(row >= 0 && delta.GetDegrees() > 0);
			return Angle::FromDegrees(row * delta.GetDegrees() + origin.GetDegrees());
		}

		static Angle ComputeColLongitude(int col, const Angle & delta, const Angle origin) {
			assert(col >= 0 && delta.GetDegrees() > 0);
			return Angle::FromDegrees(col * delta.GetDegrees() + origin.GetDegrees());
		}

	protected:

		Tile(const Sector & sector, int row, int col, int levelNumber, const std::string & levelPath);

	private:

		Sector sector_;

		int row_;

		int col_;

		int levelNumber_;

		std::string path_;

	};

}