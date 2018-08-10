#pragma once

#include "Tile.h"
#include "Tilekey.h"
#include "Level.h"
#include "LevelSet.h"
#include "Global.h"
#include "HashCombine.h"
#include "Urho3D/Math/Frustum.h"
#include <Urho3D/Math/BoundingBox.h>
#include <vector>
#include <memory>

namespace GIS 
{

	class Global;

	struct RectTileTessellateParams {

		Urho3D::Vector3 eyePosition;

		float fov;

		Urho3D::Frustum frustum;

	};

	class RectangularTessellator 
	{
	public:

		using Point3 = Urho3D::Vector3;

		using Point2 = Urho3D::Vector2;

		using BBox = Urho3D::BoundingBox;

		/**
		 * 封装渲染RectTile需要的顶点数据和索引数据.
		 */
		class RenderInfo 
		{
		public:

			/**
			 * RenderInfo hash key.
			 */
			class Key 
			{
			public:

				Key(int density, const Sector & sector)
				{
					m_HashCode = 0;
					HashCombine(m_HashCode, density);
					HashCombine(m_HashCode, sector.GetMinLatitude().GetDegrees());
					HashCombine(m_HashCode, sector.GetMaxLatitude().GetDegrees());
					HashCombine(m_HashCode, sector.GetMinLongitude().GetDegrees());
					HashCombine(m_HashCode, sector.GetMaxLongitude().GetDegrees());
				}

				size_t GetHashCode() const noexcept
				{
					return m_HashCode;
				}

				bool operator ==(const Key & that) const noexcept 
				{
					return m_HashCode == that.m_HashCode;
				}

			private:

				size_t m_HashCode;

			};

			struct KeyHash {
			
				size_t operator ()(const Key & key) const noexcept 
				{
					return key.GetHashCode();
				}

			};

			RenderInfo(const Sector & sector, int density)
			{
				m_Sector = sector;
				m_Density = density;
			}

			void SetVertices(std::vector<Point3> && vertices) 
			{
				m_Vertices = std::move(vertices);
			}

			void SetNormals(std::vector<Point3> && normals)
			{
				m_Normals = std::move( normals );
			}
			
			void SetTexCoords( std::shared_ptr<std::vector<Point2>> texCoords )
			{
				m_TexCoords = texCoords;
			}

			void SetIndices( std::shared_ptr<std::vector<uint32_t>> indices)
			{
				m_Indices = indices;
			}

			const std::vector<Point3> & GetVertices() const noexcept
			{
				return m_Vertices;
			}

			const std::vector<Point2> & GetTexCoords() const noexcept
			{
				return *m_TexCoords;
			}

			const std::vector<uint32_t> & GetIndices() const noexcept
			{
				return *m_Indices;
			}

			const std::vector<Point3> & GetNormals() const noexcept
			{
				return m_Normals;
			}

			float GetDensity() const noexcept 
			{
				return m_Density;
			}

			void SetBBox(const BBox & bbox) 
			{
				m_BBox = bbox;
			}

			const BBox & GetBBox() const noexcept
			{
				return m_BBox;
			}

			Urho3D::Vector3 Interpolate(int row, int column, float xDec, float yDec) const;

		private:

			int m_Density;

			Sector m_Sector;

			std::vector<Point3> m_Vertices;

			std::vector<Point3> m_Normals;

			std::shared_ptr<std::vector<Point2>> m_TexCoords;

			std::shared_ptr<std::vector<uint32_t>> m_Indices;

			BBox m_BBox;

		};

		class RectTile {
		public:

			RectTile(const Sector & sector, uint32_t levelNumber, uint32_t density, const BBox & bbox ) 
				: sector_( sector ), levelNumber_( levelNumber ), bbox_( bbox ), density_( density )
			{
				cellSize_ = sector.GetDeltaLat().GetRadians() / density;
			}

			void SetRenderInfo(const std::shared_ptr<RenderInfo> & renderInfo)
			{
				renderInfo_ = renderInfo;
				bbox_ = renderInfo->GetBBox();
			}

			const BBox & GetBBox() const noexcept
			{
				return bbox_;
			}

			const std::shared_ptr<RenderInfo> & GetRenderInfo() const noexcept 
			{
				return renderInfo_;
			}

			int GetDensity() const noexcept
			{
				return density_;
			}

			const Sector & GetSector() const noexcept
			{
				return sector_;
			}

			float GetResolution() const noexcept
			{
				return sector_.GetDeltaLat().GetRadians() / density_;
			}

			uint32_t GetLevelNumber() const noexcept
			{
				return levelNumber_;
			}

			/**
			 * 设置RectTile的参考中心
			 */
			void SetRefCenter(const Global::Vector4D & refCenter)
			{
				refCenter_ = refCenter;
			}

			const Global::Vector4D & GetRefCenter() const noexcept 
			{
				return refCenter_;
			}

			double GetCellSize() const noexcept 
			{
				return cellSize_;
			}

			void SetBoundingBox(const BBox & bbox)
			{
				bbox_ = bbox;
			}

			bool GetSurfacePoint(const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & result );

		private:

			Sector sector_;

			uint32_t levelNumber_ = 0;

			BBox bbox_;

			uint32_t density_ = 0;

			double cellSize_ = 0;

			Global::Vector4D refCenter_;

			std::shared_ptr<RenderInfo> renderInfo_;
		};


	public:

		RectangularTessellator(Global & global, ElevationConfig & config);

		/**
		 * 细分区块
		 * @return 当前可见的区块数组
		 */
		auto Tessellate( const RectTileTessellateParams & params )-> const std::vector<std::shared_ptr<RectTile>> &;

		/**
		 * 获取地球表面的位置
		 */
		bool GetPointOnTerrain(const Angle & latitude, const Angle & longitude, Urho3D::Vector3 & point);

	private:

		void CreateTopLevelTiles();

		void SelectVisibleTiles(const RectTileTessellateParams & params, std::shared_ptr<RectTile> tile );

		std::shared_ptr<RectTile> CreateTile(const Sector & sector, int level);

		void CheckVertexData( RectTile & tile );

		void CreateTileVertexData(RectTile & tile);

		/**
		 * 计算当前RectTile的所有顶点对应的经纬度
		 */
		std::vector<LatLon> ComputeLocation(const RectTile & rectTile);

		/**
		 * 计算RectTile纹理坐标数据
		 *
		 * @param[in] density RectTile的密度
		 * @return 索引数据
		 */
		std::vector<Urho3D::Vector2> ComputeTextureTexCoords(int density);

		/**
		 * 计算RectTile的下标索引数据
		 *
		 * @param[in] density RectTile的密度
		 * @return 索引数据
		 */
		std::vector<uint32_t> ComputeIndices(int density);

		/**
		 * 判断当前Tile是否达到最大分辨率
		 */
		bool AtBestResolution(const RectTile & rectTile);

		/**
		 * 根据距离判断当前Tile是否需要细分
		 */
		bool NeedToSplit(const RectTileTessellateParams & param, const RectTile & tile);

	private:

		std::vector<std::shared_ptr<RectTile>> currentTiles_;

		std::vector<std::shared_ptr<RectTile>> topLevelTiles_;

		uint32_t density_;

		LevelSet levelSet_;

		uint32_t numLevel0LatSubdivision = 0;

		uint32_t numLevel0LonSubdivision = 0;

		float verticalExaggeration_ = 1.0f;

		Global & global_;

		std::unordered_map<RenderInfo::Key, std::shared_ptr<RenderInfo>, RenderInfo::KeyHash> m_RenderInfoMap;

		uint32_t m_MaxLevel = 0;

		std::unordered_map<SectorKey, Urho3D::BoundingBox> m_BBoxMap;

		static std::unordered_map<int, std::shared_ptr<std::vector<uint32_t>>> ms_IndicesMap;

		static std::unordered_map<int, std::shared_ptr<std::vector<Urho3D::Vector2>>> ms_TexCoordsMap;

	};

}