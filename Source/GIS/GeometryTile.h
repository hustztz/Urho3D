#pragma once

#include "RectangularTessellator.h"
#include <Urho3D/Graphics/Geometry.h>

namespace GIS 
{

	/**
	 * ��ӦRectangleTessellator��RectTile
	 */
	class GeometryTile : public Urho3D::Geometry 
	{
	public:

		using RectTile = RectangularTessellator::RectTile;

	public:

		GeometryTile(Urho3D::Context *context);

		/**
		 * ��ʼ��GeometryTile�ļ���������
		 */
		void SetRectTile( RectTile *rectTile );

		/**
		 * ����Texcoords buffer
		 */
		void SetTexCoordsBuffer(Urho3D::VertexBuffer *texCoordsBuffer);

		/**
		 * ��ȡRectTile
		 */
		RectTile *GetRectTile() const noexcept;

	private:

		/**
		 * ����λ������
		 */
		void SetPositionData(uint64_t count, const void *data);

		/**
		 * ���÷�������
		 */
		void SetNormalData(uint64_t count, const void *data);

	private:

		RectTile *m_RectTile = nullptr;

	};

}