#pragma once
#include "../../Container/Ptr.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Node.h"
#include <functional>
#include <assert.h>
#include <type_traits>

namespace Urho3D
{
	class Context;
	class Image;
	class Node;
	class Scene;
	class String;
	class Texture2D;
	class Viewport;

	class URHO3D_API RenderUtil
	{
	public:
		static void CaptureScreen(Context* context, Image* screenshot);
		static SharedPtr<Texture2D> CreateRenderTexture(Context * context, int width, int height);
		static SharedPtr<Viewport> CreateViewport(Context * context, Texture2D * renderTexture, Scene * scene, Node * cameraNode);
		template<typename T, typename... Ts>
		static SharedPtr<Scene> CreateScene(Context* context, T* node, Ts*... nodes);
		static void CaptureCamera(Context * context, Texture2D * renderTexture, std::function<void(void)> caller, const String & teniqueName = "");
	private:
		static void SetOverrideTechnique(Node* node, const String& techniqueName);
		template<typename... T>
		static SharedPtr<Scene> CreateScene(SharedPtr<Scene> scene, T*... node) { return scene; };
		template<typename T, typename... Ts>
		static SharedPtr<Scene> CreateScene(SharedPtr<Scene> scene, T* node, Ts*... nodes);
		template<typename T>
		static void CloneNode(T* node, T* nodeSource);
	};

	template<typename T, typename... Ts>
	inline SharedPtr<Scene> RenderUtil::CreateScene(Context* context, T* node, Ts*... nodes)
	{
		static_assert(std::is_same<T, Node>::value, "CreateScene末尾可变参数传入非Node类型指针");
		SharedPtr<Scene> scene(new Scene(context));
		context->AddRef();
		scene->CreateComponent<Octree>();
		return CreateScene(scene, node, nodes...);
	}

	template<typename T, typename ...Ts>
	inline SharedPtr<Scene> RenderUtil::CreateScene(SharedPtr<Scene> scene, T * node, Ts * ...nodes)
	{
		if (node)
		{
			auto* nodeClone = node->Clone();
			CloneNode(nodeClone, node);
			scene->AddChild(nodeClone);
		}
		return CreateScene(scene, nodes...);
	}

	template<typename T>
	inline void RenderUtil::CloneNode(T * node, T * nodeSource)
	{
		const Vector<SharedPtr<Component> >& components = nodeSource->GetComponents();
		const Vector<SharedPtr<Component> >& componentsClone = node->GetComponents();
		for (int i = 0; i < components.Size(); i++)
		{
			if (components[i]->GetTypeInfo()->IsTypeOf(StaticModel::GetTypeStatic()))
			{
				StaticModel* drawable = dynamic_cast<StaticModel*>(components[i].Get());
				StaticModel* drawableClone = dynamic_cast<StaticModel*>(componentsClone[i].Get());
				if (!drawable || !drawableClone)
					continue;
				for (int i = 0; i < drawable->GetBatches().Size(); ++i)
				{
					drawableClone->SetMaterial(i, drawable->GetBatches()[i].material_);
				}
			}
		}
		for (int i = 0; i < node->GetNumChildren(); ++i)
		{
			CloneNode(node->GetChild(i), nodeSource->GetChild(i));
		}
	}

}