#include "GISApp.h"
#include "DrawableTile.h"
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Shape/Coordinate.h>
#include <Urho3D/Shape/Box.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <iostream>

int main()
{
	Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context()); 
	Urho3D::SharedPtr<GIS::GISApp> application(new GIS::GISApp(context)); 
	return application->Run();
}

GIS::GISApp::GISApp(Context *context) : Sample( context )
{
	DrawableTile::RegisterObject(context);
}

void GIS::GISApp::Start()
{
	Sample::Start();
	CreateScene();
	SetupViewport();
	SubscribeToEvents();
}

void GIS::GISApp::CreateScene() 
{
	auto* cache = GetSubsystem<ResourceCache>();

	scene_ = new Scene(context_);

	auto tree = scene_->CreateComponent<Octree>();
	scene_->CreateComponent<DebugRenderer>();

	Node* zoneNode = scene_->CreateChild("Zone");
	auto* zone = zoneNode->CreateComponent<Zone>();
	// Set same volume as the Octree, set a close bluish fog and some ambient light
	zone->SetBoundingBox(BoundingBox(-1000000000, 1000000000));
	zone->EnableFog(false);

	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
	auto* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_POINT);

	auto* coodinate = scene_->CreateComponent<Coordinate>();
	coodinate->SetSample(50);

	Node* skyNode = scene_->CreateChild("Sky");
	skyNode->SetScale(500.0f);
	auto* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

	cameraNode_ = scene_->CreateChild("Camera");
	cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0, 0.0f, -10.0f));
	cameraNode_->SetPosition(Vector3(0, 0.0f, -40000000.0f));
	cameraNode_->GetComponent<Camera>()->SetFarClip(10000000000);
	cameraNode_->GetComponent<Camera>()->SetNearClip(10000);
	lightNode->SetPosition(cameraNode_->GetPosition());
	m_TileNodeManager.reset(new TileNodeManager{ global_, scene_ });
	m_TileNodeManager->Tessellate();
}

void GIS::GISApp::SetupViewport() 
{
	auto* renderer = GetSubsystem<Renderer>();
	SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
	renderer->SetViewport(0, viewport);
}

void GIS::GISApp::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GISApp, HandleUpdate));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GISApp, HandlePostRenderUpdate));
}


void GIS::GISApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

	// Take the frame time step, which is stored as a float
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	// Move the camera, scale movement with time step
	MoveCamera(timeStep);
}

void GIS::GISApp::HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData) {
	context_->GetSubsystem<Renderer>()->DrawDebugGeometry(true);
}

void GIS::GISApp::MoveCamera(float timeStep)
{// Do not move if the UI has a focused element (the console)
	if (GetSubsystem<UI>()->GetFocusElement())
		return;

	auto* input = GetSubsystem<Input>();

	// Movement speed as world units per second
	const float MOVE_SPEED = 9000000.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.1f;

	// Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
	IntVector2 mouseMove = input->GetMouseMove();
#ifdef URHO3D_RIGHT_HANDED
	yaw_ -= MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ -= MOUSE_SENSITIVITY * mouseMove.y_;
#else
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
#endif
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	// Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
	cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

	// Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
	// Use the Translate() function (default local space) to move relative to the node's orientation.
	bool move = false;
	if (input->GetKeyDown(KEY_W)) {
		cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
		move = true;
	}
	if (input->GetKeyDown(KEY_S)) {
		cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
		move = true;
	}
	if (input->GetKeyDown(KEY_A)) {
		cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
		move = true;
	}
	if (input->GetKeyDown(KEY_D)) {
		cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
		move = true;
	}
	bool isNeedUpdate = false;
	if (input->GetKeyPress(KEY_UP)) {
		++m_Show;
		isNeedUpdate = true;
	}
	if (input->GetKeyPress(KEY_DOWN)) {
		m_Show = m_Show ? m_Show - 1 : 0;
		isNeedUpdate = true;
	}
	if (input->GetKeyPress(KEY_LEFT)) {
		m_TileNodeManager->SetTessellateEnable(false);
	}
	if (input->GetKeyPress(KEY_RIGHT)) {
		m_TileNodeManager->SetTessellateEnable(true);
	}
	if (!m_Debug && (isNeedUpdate ||move || mouseMove.x_ || mouseMove.y_ ) ) {
		Node* lightNode = scene_->GetChild("DirectionalLight");
		auto light = lightNode->GetComponent<Light>();
		float farDist = global_.ComputeFarClipDistance(cameraNode_->GetPosition());
		cameraNode_->GetComponent<Camera>()->SetFarClip(farDist);
		light->SetRange(farDist + farDist * 0.4f);
		lightNode->SetPosition(cameraNode_->GetPosition());
		float nearDist = global_.ComputePerspectiveNearDistance(farDist, 3.0f, 24);
		cameraNode_->GetComponent<Camera>()->SetNearClip(nearDist);
		m_TileNodeManager->Tessellate();
	}
}