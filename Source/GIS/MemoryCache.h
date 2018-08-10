#pragma once

#include "Tile.h"
#include "Tilekey.h"
#include "MutexMap.h"
#include <memory>
#include <unordered_map>

namespace GIS 
{

	template <typename T>
	class MemoryCache 
	{
	public:

		std::shared_ptr<T> Get(const TileKey & key) const {
			std::shared_ptr<T> item;
			map_.GetValue(key, item);
			return item;
		}

		void Put(const TileKey & key, const std::shared_ptr<T> & tile) {
			map_.SetValue( key, tile );
		}

		bool Exists(const TileKey & key) {
			return map_.Exists(key);
		}

		void Remove(const TileKey & key) {
			map_.Remove(key);
		}

	private:

		MutexMap<TileKey, std::shared_ptr<T>> map_;

	};

}