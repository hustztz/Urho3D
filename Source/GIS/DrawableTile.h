#pragma once

#include "RectangularTessellator.h"
#include <Urho3D/Graphics/Drawable.h>

namespace GIS 
{

	class GeometryTile;

	/**
	 * RectTile���
	 */
	class DrawableTile : public Urho3D::Drawable 
	{
	public:

		DrawableTile(Urho3D::Context *context);

		/**
		 * ���߲�ѯ
		 */
		void ProcessRayQuery(const Urho3D::RayOctreeQuery& query, Urho3D::PODVector<Urho3D::RayQueryResult>& results) override final;
		
		/**
		 * ����Batch
		 */
		void UpdateBatches(const Urho3D::FrameInfo & frame) override;

		/**
		 * ���ü�����
		 */
		void SetGeometry(Urho3D::Geometry *geometry);

		/**
		 * ���ò���
		 */
		void SetMatarial(Urho3D::Material *material);

		/**
		 * ��ȡGeometry
		 */
		GeometryTile *GetGeometry() const noexcept;

		/**
		 * ����bounding box
		 */
		void SetBoundingBox(const Urho3D::BoundingBox & bbox);

		/**
		 * ע�����
		 */
		static void RegisterObject(Urho3D::Context* context);

	protected:

		void OnWorldBoundingBoxUpdate() override final;


	private:

		Urho3D::SharedPtr<Urho3D::Geometry> geometry_;

	};

}