#include "TileNodeManager.h"
#include "DrawableTile.h"
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Shape/Box.h>
#include <Urho3D/Graphics/StaticModel.h>

GIS::TileNodeManager::TileNodeManager(Global & global, Scene *scene)
	: m_Global( &global ), m_Scene( scene ) 
{
	m_CameraNode = m_Scene->GetChild("camera");
	m_Camera = m_CameraNode->GetComponent<Camera>();
	m_Scene->CreateChild("Tile");
}

void GIS::TileNodeManager::Tessellate()
{
	if (!m_IsEnableTessellate)
		return;
	RectTileTessellateParams params;
	params.eyePosition = m_CameraNode->GetPosition();
	params.fov = m_Camera->GetFov();
	params.frustum = m_Camera->GetFrustum();
	auto & rectTiles = m_Global->GetTessellator().Tessellate(params);
	decltype(m_NodesMap) newNodeMap;
	auto tileNode = m_Scene->GetChild("Tile");

	auto cache = m_Scene->GetContext()->GetSubsystem<Urho3D::ResourceCache>();
	auto boxMode = cache->GetResource<Urho3D::Model>("Models/Box.mdl");
	auto boxMaterial = cache->GetResource<Urho3D::Material>("Materials/Stone.xml");
	boxMaterial->SetFillMode(Urho3D::FILL_WIREFRAME);

	if (rectTiles.size()) {
		auto & front = rectTiles.front();
		auto & frontRenderInfo = front->GetRenderInfo();
		if (!m_IndexBuffer) 
		{
			m_IndexBuffer = new Urho3D::IndexBuffer{ m_Scene->GetContext() };
			auto & indices = frontRenderInfo->GetIndices();
			m_IndexBuffer->SetSize(indices.size(), true);
			auto lockedMem = m_IndexBuffer->Lock(0, indices.size(), true);
			memcpy(lockedMem, indices.data(), indices.size() * sizeof(uint32_t));
			m_IndexBuffer->Unlock();
		}
		if (!m_TexCoordsBuffer) 
		{
			m_TexCoordsBuffer = new VertexBuffer{ m_Scene->GetContext() };
			auto & texCoords = frontRenderInfo->GetTexCoords();
			Urho3D::PODVector<Urho3D::VertexElement> element;
			element.Push(Urho3D::VertexElement{ Urho3D::VertexElementType::TYPE_VECTOR2, Urho3D::VertexElementSemantic::SEM_TEXCOORD });
			m_TexCoordsBuffer->SetSize(texCoords.size(), element);
			auto lockedMem = m_TexCoordsBuffer->Lock(0, texCoords.size());
			memcpy(lockedMem, texCoords.data(), texCoords.size() * sizeof(Urho3D::Vector2));
			m_TexCoordsBuffer->Unlock();
		}
		for (auto & rectTile : rectTiles)
		{
			SectorKey key{ rectTile->GetSector() };
			if (auto nodeIter = m_NodesMap.find(key); nodeIter != m_NodesMap.end())
			{
				auto node = nodeIter->second;
				newNodeMap[key] = node;
				m_NodesMap.erase(key);
			}
			else
			{
				SharedPtr<Node> newNode;
				DrawableTile *drawableTile = nullptr;
				GeometryTile *geometry = nullptr;
				if (m_FreeNodes.size())
				{
					newNode = m_FreeNodes.front();
					m_FreeNodes.pop();
					drawableTile = newNode->GetComponent<DrawableTile>();
					geometry = drawableTile->GetGeometry();
				}
				else {
					newNode = tileNode->CreateChild();
					drawableTile = newNode->CreateComponent<DrawableTile>();
					geometry = new GeometryTile{ m_Scene->GetContext() };
				}
				geometry->SetRectTile(rectTile.get());
				geometry->SetIndexBuffer(m_IndexBuffer);
				geometry->SetDrawRange( Urho3D::PrimitiveType::TRIANGLE_STRIP, 0, m_IndexBuffer->GetIndexCount());
				geometry->SetTexCoordsBuffer(m_TexCoordsBuffer);
				drawableTile->SetBoundingBox(rectTile->GetBBox());
				
				drawableTile->SetGeometry(geometry);
				tileNode->AddChild(newNode);
				newNodeMap[key] = newNode;
			}
		}

	}

	for(auto & [key, node] : m_NodesMap) 
	{
		tileNode->RemoveChild(node);
		m_FreeNodes.push(node);
	}
	m_NodesMap = std::move(newNodeMap);
}

void GIS::TileNodeManager::SetTessellateEnable(bool isEnable)
{
	m_IsEnableTessellate = isEnable;
}
