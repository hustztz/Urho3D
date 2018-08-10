#include "DrawableTile.h"
#include "GeometryTile.h"
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Core/Context.h>

namespace Urho3D {

	extern const char* GEOMETRY_CATEGORY;

}

GIS::DrawableTile::DrawableTile(Urho3D::Context *context)
	: Drawable( context, Urho3D::DRAWABLE_GEOMETRY ) 
{

	//SetGeometry(geometry);
	//vertices[3].y_ += 1;
	//SetBoundingBox(Urho3D::BoundingBox(vertices[0], vertices[3]));
}

void GIS::DrawableTile::ProcessRayQuery(const Urho3D::RayOctreeQuery & query, Urho3D::PODVector<Urho3D::RayQueryResult>& results)
{
}

void GIS::DrawableTile::UpdateBatches(const Urho3D::FrameInfo & frame)
{
	Drawable::batches_[0].worldTransform_ = Drawable::node_ ? &Drawable::node_->GetWorldTransform() : nullptr;
}

void GIS::DrawableTile::SetGeometry(Urho3D::Geometry *geometry)
{
	if (!geometry || geometry == geometry_ )
	{
		return;
	}
	geometry_ = geometry;
	Drawable::batches_.Resize(1);
	Drawable::batches_[0].worldTransform_ = Drawable::node_ ? &Drawable::node_->GetWorldTransform() : nullptr;
	Drawable::batches_[0].geometry_ = geometry;
	Drawable::batches_[0].geometryType_ = Urho3D::GeometryType::GEOM_STATIC;
}

void GIS::DrawableTile::SetMatarial(Urho3D::Material *material) 
{
	if( 0 != Drawable::batches_.Size() ) 
	{
		Drawable::batches_[0].material_ = material;
	}
}

GIS::GeometryTile *GIS::DrawableTile::GetGeometry() const noexcept
{
	return static_cast<GeometryTile*>( Drawable::batches_[0].geometry_ );
}
	
void GIS::DrawableTile::RegisterObject(Urho3D::Context *context)
{
	context->RegisterFactory<DrawableTile>(Urho3D::GEOMETRY_CATEGORY);
}

void GIS::DrawableTile::OnWorldBoundingBoxUpdate()
{
	Drawable::worldBoundingBox_ = Drawable::boundingBox_;
}

void GIS::DrawableTile::SetBoundingBox(const Urho3D::BoundingBox & bbox)
{
	Drawable::boundingBox_ = bbox;
	OnMarkedDirty(Drawable::node_);
}

