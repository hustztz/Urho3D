#pragma once

#include "Angle.h"
#include "LevelSet.h"
#include "HashCombine.h"
#include <string>

namespace GIS 
{

    class TileKey 
    {
        
    public:

		TileKey() : TileKey( 0, 0, 0, "" ) {
		}

        TileKey(int levelNumber, int row, int col, const std::string & cacheName) 
        {
            levelNumber_ = levelNumber;
            row_ = row;
            col_ = col;
            cacheName_ = cacheName;
            hash_ = ComputeHash();
        }

        TileKey(Angle latitude, Angle longitude, LevelSet levelSet, int levelNumber);

        int GetLevelNumber() const noexcept 
        {
            return levelNumber_;
        }

        int GetRow() const noexcept 
        {
            return row_;
        }

        int GetCol() const noexcept
        {
            return col_;
        }

        const std::string & GetCacheName() const noexcept 
        {
            return cacheName_;
        }

		size_t GetHashCode() const noexcept 
		{
			return hash_;
		}

		bool operator ==(const TileKey & that) const 
		{
			return levelNumber_ == that.levelNumber_ && row_ == that.row_ && col_ == that.col_ && cacheName_ == that.cacheName_;
		}

		std::string GetPathInLevel() const {
			return std::to_string(row_) + "/" + std::to_string(row_) + "_" + std::to_string(col_);
		}

    private:

        size_t ComputeHash() {
			size_t seed = 0;
			HashCombine(seed, levelNumber_);
			HashCombine(seed, row_);
			HashCombine(seed, col_);
			HashCombine(seed, cacheName_);
			return seed;
        }

    private:
        int levelNumber_;
        int row_;
        int col_;
        std::string cacheName_;
        size_t hash_;
    };

}

namespace std {

	template <>
	struct hash<GIS::TileKey> 
	{

		std::size_t operator() (const GIS::TileKey & key) const
		{
			return key.GetHashCode();
		}
	};

}