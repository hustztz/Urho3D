#include "GeometryTile.h"
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <memory>

GIS::GeometryTile::GeometryTile(Urho3D::Context *context) 
	: Geometry( context ) 
{
	SetNumVertexBuffers(3);
}

void GIS::GeometryTile::SetRectTile(RectangularTessellator::RectTile *rectTile) 
{
	m_RectTile = rectTile;
	auto renderInfo = rectTile->GetRenderInfo();
	auto & positionData = renderInfo->GetVertices();
	SetPositionData(positionData.size(), positionData.data());
	auto & normalData = renderInfo->GetNormals();
	SetNormalData(normalData.size(), normalData.data());
}

void GIS::GeometryTile::SetPositionData(uint64_t count, const void *data)
{
	auto positionBuffer = GetVertexBuffer(0);
	if (!positionBuffer) {
		Urho3D::SharedPtr<Urho3D::VertexBuffer> vertexBuffer(new Urho3D::VertexBuffer{ Geometry::context_ });
		vertexBuffer->SetShadowed(false);
		Urho3D::PODVector<Urho3D::VertexElement> element;
		element.Push(Urho3D::VertexElement{ Urho3D::VertexElementType::TYPE_VECTOR3, Urho3D::VertexElementSemantic::SEM_POSITION });
		vertexBuffer->SetSize(count, element, false);
		SetVertexBuffer(0, vertexBuffer);
		positionBuffer = GetVertexBuffer(0);
	}
	auto lockedMem = positionBuffer->Lock(0, count, true);
	memcpy(lockedMem, data, sizeof(Urho3D::Vector3) * count);
	positionBuffer->Unlock();
}

void GIS::GeometryTile::SetNormalData(uint64_t count, const void * data)
{
	auto normalBuffer = GetVertexBuffer(1);
	if (!normalBuffer) 
	{
		normalBuffer = new Urho3D::VertexBuffer{ Geometry::context_ };
		normalBuffer->SetShadowed(false);
		Urho3D::PODVector<Urho3D::VertexElement> element;
		element.Push(Urho3D::VertexElement{ Urho3D::VertexElementType::TYPE_VECTOR3, Urho3D::VertexElementSemantic::SEM_NORMAL });
		normalBuffer->SetSize(count, element, false);
		SetVertexBuffer(1, normalBuffer);
	}
	auto lockedMem = normalBuffer->Lock(0, count, true);
	memcpy(lockedMem, data, sizeof(Urho3D::Vector3) * count);
	normalBuffer->Unlock();
}

void GIS::GeometryTile::SetTexCoordsBuffer(Urho3D::VertexBuffer *texCoordsBuffer)
{
	SetVertexBuffer(2, texCoordsBuffer);
}

auto GIS::GeometryTile::GetRectTile() const noexcept -> RectTile*
{
	return m_RectTile;
}
