#pragma once

#include "Tile.h"
#include "LevelSet.h"
#include "Level.h"
#include "ElevationConfig.h"
#include "LevelSet.h"
#include "TileKey.h"
#include "MemoryCache.h"
#include "MutexMap.h"
#include "MutexSet.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <queue>
#include <future>
#include <iostream>

namespace GIS 
{

    class BasicElevationModel 
    {

		using ByteOrder = ElevationConfig::ByteOrder;

		using DataType = ElevationConfig::DataType;

        class ElevationTile : public Tile 
		{
        public:

			ElevationTile( BasicElevationModel & model, const Sector & sector, int row, int col, int levelNumber, const std::string & levelPath)
				: model_( model ), Tile(sector, row, col, levelNumber, levelPath) {
			}

			void SetElevationData(std::vector<float> && data)
			{
				elevationData_ = std::move(data);
			}

			const auto & GetElevationData() const noexcept 
			{
				return elevationData_;
			}

			float LookUpElevation(const Angle & latitude, const Angle & longitude) const noexcept;

		private:

			BasicElevationModel & model_;

			std::vector<float> elevationData_;

		};

		class Elevations 
		{

		public:

			struct Less 
			{
				bool operator ()(const std::shared_ptr<ElevationTile> & lhs, const std::shared_ptr<ElevationTile> & rhs) const 
				{
					return(lhs->GetLevelNumber() > rhs->GetLevelNumber());
				}
			};

			using Tiles = std::set<std::shared_ptr<ElevationTile>, Less>;

		public:

			Elevations(BasicElevationModel & model)
				: model_(model)
			{
			}

			void SetTiles(Tiles && tiles) 
			{
				tiles_ = std::move(tiles);
			}

			void SetTiles(const Tiles & tiles) 
			{
				tiles_ = tiles;
			}

			void SetTexelSize(double texelSize) 
			{
				texelSize_ = texelSize;
			}

			double GetTexelSize() const noexcept 
			{
				return texelSize_;
			}

			float GetElevation(const LatLon & latLon) const noexcept 
			{
				if (auto tile = GetContainTile(latLon)) {

					auto & elevations = tile->GetElevationData();

					auto sector = tile->GetSector();
					auto & level = model_.GetLevelSet().GetLevel(tile->GetLevelNumber());
					int tileWidth = level.GetTileWidth();
					int tileHeight = level.GetTileHeight();
					float sectorDeltaLat = sector.GetDeltaLat().GetRadians();
					float sectorDeltaLon = sector.GetDeltaLon().GetRadians();
					float dLat = sector.GetMaxLatitude().GetRadians() - latLon.GetLatitude().GetRadians();
					float dLon = latLon.GetLongitude().GetRadians() - sector.GetMinLongitude().GetRadians();
					float sLat = dLat / sectorDeltaLat;
					float sLon = dLon / sectorDeltaLon;

					int j = static_cast<int>( ( tileHeight - 1) * sLat );
					int i = static_cast<int>((tileWidth - 1) * sLon);
					int k = j * tileWidth + i;

					// 双线性差值
					float eLeft = elevations[k];
					//return eLeft;
					float eRight = i < (tileWidth - 1) ? elevations[k + 1] : eLeft;

					auto missingSignal = model_.GetMissingDataSignal();
					if (abs(missingSignal - eLeft) < 0.00001 || abs(missingSignal - eRight) < 0.00001)
					{
						return missingSignal;
					}
					float dw = sectorDeltaLon / (tileWidth - 1);
					float dh = sectorDeltaLat / (tileHeight - 1);
					float ssLon = (dLon - i * dw) / dw;
					float ssLat = (dLat - j * dh) / dh;
					float eTop = eLeft + ssLon * (eRight - eLeft);
					if (j < tileHeight - 1 && i < tileWidth - 1)
					{
						eLeft = elevations[k + tileWidth];
						eRight = elevations[k + tileWidth + 1];
					}
					float eBot = eLeft + ssLon * (eRight - eLeft);
					return eTop + ssLat * (eBot - eTop);
				}
				return 0;
			}

		private:

			std::shared_ptr<ElevationTile> GetContainTile( const LatLon & latLon ) const noexcept
			{
				for (auto tile : tiles_)
				{
					if (tile->GetSector().IsContain(latLon.GetLatitude(), latLon.GetLongitude()))
						return tile;
				}
				return {};
			}

		private:

			double texelSize_;

			Tiles tiles_;

			BasicElevationModel & model_;

		};

	public:

        BasicElevationModel(ElevationConfig config);

		void SetDetailHint(float detailHint);

		std::shared_ptr<Elevations> GetElevations(const Sector & requestedSector, const LevelSet & levelSet, int targetLevelNumber);

		const LevelSet & GetLevelSet() const noexcept
		{
			return levelSet_;
		}

		float GetMissingDataSignal() const noexcept
		{
			return missingDataSignal_;
		}

		/**
		 * 获取最高地形分辨率
		 */
		double GetBestResolution() const noexcept;

		/**
		 * 加载所有0层的高度数据
		 */
		void LoadAllLevelZeroTiles();

		/**
		 * 获取某个区域内的高度极值
		 */
		std::pair<float, float> GetExtremeElevations(const Sector & sector);

		/**
		 * @overload
		 */
		std::pair<float, float> GetExtremeElevations(const Angle & latitude, const Angle & longitude) const noexcept;

		/** 
		 * 获取最大最小的高度数据
		 * @return std::pair<float, float> 第一个元素是最小高度
		 */
		std::pair<float, float> GetMaxMinElevation() const noexcept;

		/**
		 * 获取一组高度数据
		 */
		std::vector<float> GetElevations(const Sector & sector, std::vector<LatLon> const & latlons, float targetResolution, bool mapMissingData);

		/**
		 * 判断某个经纬度是否在当前模型范围内
		 */
		bool IsContain(const Angle & latitude, const Angle & longitude) const noexcept;

		/**
		 * 获取某个经纬度的高度值
		 */
		float GetElevation(const Angle & latitude, const Angle & longitude) const noexcept;

		float GetMissingDataReplacement() const noexcept;

	private:

		std::pair<float, float> ComputeExtrameElevations(const Sector & sector) const noexcept;

		std::shared_ptr<ElevationTile> GetTileFromMemory(const TileKey & key) const noexcept;

		void RequestTile( const TileKey & tileKey );

		std::shared_ptr<ElevationTile> CreateTile(const TileKey & tileKey);

		void AddTile(const std::shared_ptr<ElevationTile> & tile);

		void LoadExtremeElevations( const std::string & filename );

		float GetUnmappedElevation(const Angle & latitude, const Angle & longitude) const noexcept;

    private:

        std::unordered_map<TileKey, std::shared_ptr<ElevationTile>> levelZeroTiles_;

		MemoryCache<ElevationTile> memoryCache_;

		LevelSet levelSet_;

		float detailHint_ = 0.0f;

		float missingDataSignal_ = 0;

		float missingSignalRep_ = 0;

		MutexSet<TileKey> requestingTiles_;

		std::string modelPath_;

		ElevationConfig::ByteOrder byteOrder_;

		ElevationConfig::DataType dataType_;

		LatLon tileOrigin_;

		std::queue<std::future<std::shared_ptr<ElevationTile>>> loadedTileTasks_;

		std::pair<float, float> elevationMinMax_;

		std::vector<float> extremes_;

		int32_t extremeLevel_ = -1;

		/**
		 * 对高度极值进行缓存
		 */
		std::unordered_map<SectorKey, std::pair<float, float>> m_ExtremesCache;

    };

}