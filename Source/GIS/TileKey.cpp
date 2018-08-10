#include "Tile.h"
#include "Tilekey.h"

GIS::TileKey::TileKey(Angle latitude, Angle longitude, LevelSet levelSet, int levelNumber) {
	auto & l = levelSet.GetLevel(levelNumber);
	levelNumber_ = levelNumber;
	row_ = Tile::ComputeRow(l.GetTileDelta().GetLatitude(), latitude, levelSet.GetTileOrigin().GetLatitude());
	col_ = Tile::ComputeCol(l.GetTileDelta().GetLongitude(), longitude, levelSet.GetTileOrigin().GetLongitude());
	cacheName_ = l.GetCacheName();
	ComputeHash();
}
