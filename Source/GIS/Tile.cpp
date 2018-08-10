#include "Tile.h"

const GIS::Sector & GIS::Tile::GetSector() const noexcept
{
	return sector_;
}

int GIS::Tile::GetRow() const noexcept
{
	return row_;
}

int GIS::Tile::GetCol() const noexcept
{
	return col_;
}

int GIS::Tile::GetLevelNumber() const noexcept
{
	return levelNumber_;
}

const std::string & GIS::Tile::GetPath() const noexcept
{
	return path_;
}

GIS::Tile::Tile(const Sector & sector, int row, int col, int levelNumber, const std::string & levelPath)
{
	sector_ = sector;
	row_ = row;
	col_ = col;
	levelNumber_ = levelNumber;
	path_ = levelPath + "/" + std::to_string( row ) + "/" + std::to_string( row ) + "_" + std::to_string( col );
}
