#pragma once

#include "RectangularTessellator.h"
#include <Urho3D/Graphics/Drawable.h>

namespace GIS 
{

	class GeometryTile;

	/**
	 * RectTile组件
	 */
	class DrawableTile : public Urho3D::Drawable 
	{
	public:

		DrawableTile(Urho3D::Context *context);

		/**
		 * 光线查询
		 */
		void ProcessRayQuery(const Urho3D::RayOctreeQuery& query, Urho3D::PODVector<Urho3D::RayQueryResult>& results) override final;
		
		/**
		 * 更新Batch
		 */
		void UpdateBatches(const Urho3D::FrameInfo & frame) override;

		/**
		 * 设置几何体
		 */
		void SetGeometry(Urho3D::Geometry *geometry);

		/**
		 * 设置材质
		 */
		void SetMatarial(Urho3D::Material *material);

		/**
		 * 获取Geometry
		 */
		GeometryTile *GetGeometry() const noexcept;

		/**
		 * 设置bounding box
		 */
		void SetBoundingBox(const Urho3D::BoundingBox & bbox);

		/**
		 * 注册组件
		 */
		static void RegisterObject(Urho3D::Context* context);

	protected:

		void OnWorldBoundingBoxUpdate() override final;


	private:

		Urho3D::SharedPtr<Urho3D::Geometry> geometry_;

	};

}