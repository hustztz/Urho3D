#pragma once

#include "RectangularTessellator.h"
#include <Urho3D/Graphics/Geometry.h>

namespace GIS 
{

	/**
	 * 对应RectangleTessellator的RectTile
	 */
	class GeometryTile : public Urho3D::Geometry 
	{
	public:

		using RectTile = RectangularTessellator::RectTile;

	public:

		GeometryTile(Urho3D::Context *context);

		/**
		 * 初始化GeometryTile的几何体数据
		 */
		void SetRectTile( RectTile *rectTile );

		/**
		 * 设置Texcoords buffer
		 */
		void SetTexCoordsBuffer(Urho3D::VertexBuffer *texCoordsBuffer);

		/**
		 * 获取RectTile
		 */
		RectTile *GetRectTile() const noexcept;

	private:

		/**
		 * 设置位置数据
		 */
		void SetPositionData(uint64_t count, const void *data);

		/**
		 * 设置法向数据
		 */
		void SetNormalData(uint64_t count, const void *data);

	private:

		RectTile *m_RectTile = nullptr;

	};

}