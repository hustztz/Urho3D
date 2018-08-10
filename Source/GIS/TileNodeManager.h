#pragma once

#include "RectangularTessellator.h"
#include "Global.h"
#include "GeometryTile.h"
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Camera.h>
#include <queue>
#include <unordered_map>

namespace GIS 
{

	using Urho3D::Node;

	using Urho3D::Scene;

	using Urho3D::SharedPtr;

	using Urho3D::VertexBuffer;

	using Urho3D::Camera;

	class TileNodeManager 
	{
	public:

		TileNodeManager( Global & global, Scene *scene );

		void Tessellate();
		
		void SetTessellateEnable(bool isEnable);

	private:

		Global *m_Global;

		Scene *m_Scene;

		std::unordered_map<SectorKey, SharedPtr<Node>> m_NodesMap;

		std::queue<SharedPtr<Node>> m_FreeNodes;

		Node *m_CameraNode = nullptr;

		Camera *m_Camera = nullptr;

		SharedPtr<Urho3D::IndexBuffer> m_IndexBuffer;

		SharedPtr<Urho3D::VertexBuffer> m_TexCoordsBuffer;

		uint32_t m_Density = 0;

		bool m_IsEnableTessellate = true;

	};

}