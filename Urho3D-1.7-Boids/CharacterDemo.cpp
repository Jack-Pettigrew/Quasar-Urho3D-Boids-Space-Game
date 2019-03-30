// Jack Pettigrew 15593452
// Advanced Games Programming Assessment
// Boid Space Game

// Generic Includes
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/DebugRenderer.h>

// Main Menu Includes
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/CheckBox.h>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"
#include "BoidSet.h"
#include "Rocket.h"
#include "Player.h"

// Network Includes
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>

// Debug Console
#include <Urho3D/IO/Log.h>

// Custom Client UI Event
static const StringHash E_CLIENTCREATEUI("ClientCreateUIEvent");
// Custom Client Health Evenet
static const StringHash E_CLIENTUPDATEHEALTHUI("ClientUpdateHealthEvent");
// Custom remote event we use to tell the client which object they control
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
// Identifier for the node ID parameter in the event data
static const StringHash PLAYER_ID("IDENTITY");
// Custom event on server, client has pressed button that it wants to start game
static const StringHash E_CLIENTISREADY("ClientReadyToStart");

URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

const int NUM_BOIDSET = 15;

// Game Objects
BoidSet boidSet[NUM_BOIDSET];
Player player;
ParticleEmitter* spaceEmitter;
Camera* pCamera;

float deltaTime;
bool halfBoidsFlag = false;

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context)
{
}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Start()
{
    Sample::Start();

	CreateScene();

	CreateMainMenu();

	/// Sub to Main Events
	SubscribeToEvents();
}

// Create the Main Menu
void CharacterDemo::CreateMainMenu()
{
	/// Mouse Mode
	InitMouseMode(MM_RELATIVE);

	/// Default handles
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(uiStyle);

	/// Cursor Settings
	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);

	/// UI Window + add to ROOT
	window_ = new Window(context_);
	root->AddChild(window_);

	/// UI Window set up
	window_->SetMinWidth(384);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("Main Window");
	window_->SetStyleAuto();

	/// Main Menu Buttons
	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	/// Buttons
	Button* startServerButton_ = CreateButton("Start Server", font, 24, window_, true);
	startClientButton = CreateButton("Client: Start Game", font, 24, window_, false);
	Button* connectButton_ = CreateButton("Connect", font, 24, window_, true);
	serverAddressLineEdit_ = CreateLineEdit("", 24, window_);
	Button* disconnectButton_ = CreateButton("Disconnect", font, 24, window_, true);
	Button* quitButton = CreateButton("Quit", font, 24, window_, true);
	
	/// Sub to Menu Events
	SubscribeToEvent(quitButton, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleQuit));
	//SubscribeToEvent(singleplayerButton, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleStartPlay));
	SubscribeToEvent(startClientButton, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleClientStartGame));
	SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleConnect));
	SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleDisconnect));
	SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleStartServer));
	SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientConnected));
	SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientDisconnected));

	/// Create Player
	{
		/// Align for menu
		player.Initialise(GetSubsystem<ResourceCache>(), scene_, false);
		player.pNode->SetPosition(Vector3(0, 100, 15));
		player.pNode->SetRotation(Quaternion(0, 45, 0));
		PlayerAnimation();
	}

	/// Create Particle: Space Dust
	Node* spaceParticle = scene_->CreateChild("SpaceParticleEffect");
	spaceParticle->SetPosition(player.pNode->GetPosition() + Vector3::FORWARD * 4);
	spaceEmitter = spaceParticle->CreateComponent<ParticleEmitter>();
	spaceEmitter->SetEffect(cache->GetResource<ParticleEffect>("Particle/SpaceDust.xml"));
}

// Create Base Scene
void CharacterDemo::CreateScene()
{
	/// Resource handle + Scene Setup
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);
	scene_->CreateComponent<DebugRenderer>();

	/// Create Camera Object
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>(LOCAL);
	///cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	camera->SetFarClip(3000.0f);
	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
	pCamera = camera;

	///  Zone Object
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.1f, 0.1f, 0.1f));
	zone->SetFogStart(2000.0f);
	zone->SetFogEnd(5000.0f);
	zone->SetBoundingBox(BoundingBox(-1500.0f, 1500.0f));

	/// Skybox Object
	Node* skyNode = scene_->CreateChild("Skybox");
	Skybox* skybox = skyNode->CreateComponent<Skybox>();
	skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));
	skyNode->SetRotation(Quaternion(90, 0, 0));

	/// Base Lighting
	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.25f);
	light->SetBrightness(0.75);

	/// Planet Lighting
	Node* lightNode2 = scene_->CreateChild("DirectionalLight2");
	lightNode2->SetDirection(Vector3(-0.3f, -0.5f, -0.425f));
	Light* light2 = lightNode2->CreateComponent<Light>();
	light2->SetLightType(LIGHT_DIRECTIONAL);
	light2->SetCastShadows(false);
	light2->SetShadowBias(BiasParameters(0.00025f, 0.5f));
	light2->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light2->SetSpecularIntensity(0.25f);
	light2->SetBrightness(0.75f);

	/// Terrain Object
	Node* terrainNode = scene_->CreateChild("Terrain");
	terrainNode->SetPosition(Vector3(0.0f, -0.5f));
	Terrain* terrain = terrainNode->CreateComponent<Terrain>();
	terrain->SetSpacing(Vector3(2, 0.25, 2)); /// Spacing
	terrain->SetSmoothing(true); 	/// Interpolated vertices
	terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
	terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
	terrain->SetCastShadows(true);
	terrain->SetOccluder(true);

	RigidBody* terrainBody = terrainNode->CreateComponent<RigidBody>();
	terrainBody->SetCollisionLayer(2);
	CollisionShape* terrainShape = terrainNode->CreateComponent<CollisionShape>();
	terrainShape->SetTerrain();

	/// Mars
	Node* marsNode = scene_->CreateChild("Mars");
	marsNode->SetPosition(Vector3(500, 500, 1000));
	marsNode->SetScale(100.0f);

	StaticModel* mars = marsNode->CreateComponent<StaticModel>();
	mars->SetModel(cache->GetResource<Model>("Models/Planet.mdl"));
	mars->SetMaterial(cache->GetResource<Material>("Materials/Mars.xml"));
	mars->SetCastShadows(true);

	RigidBody* marsRigid = marsNode->CreateComponent<RigidBody>();
	marsRigid->SetCollisionLayer(2);
	marsRigid->SetMass(1.0f);
	marsRigid->SetUseGravity(false);
	marsRigid->SetPosition(marsNode->GetPosition());

	SharedPtr<ValueAnimation> planetRotation(new ValueAnimation(context_));
	planetRotation->SetKeyFrame(0.0f, Quaternion(0, 0, 0));
	planetRotation->SetKeyFrame(120.0f, Quaternion(0, 180.0f, 0));
	planetRotation->SetKeyFrame(240.0f, Quaternion(0, 360.0f, 0));
	marsNode->SetAttributeAnimation("Rotation", planetRotation, WM_LOOP);

	/// Venus
	Node* venusNode = scene_->CreateChild("Venus");
	venusNode->SetPosition(Vector3(-2500, 750, -1000));
	venusNode->SetScale(300.0f);

	StaticModel* venusModel = venusNode->CreateComponent<StaticModel>();
	venusModel->SetModel(cache->GetResource<Model>("Models/Planet.mdl"));
	venusModel->SetMaterial(cache->GetResource<Material>("Materials/Earth.xml"));
	venusModel->SetCastShadows(true);

	RigidBody* venusRigid = venusNode->CreateComponent<RigidBody>();
	venusRigid->SetCollisionLayer(2);
	venusRigid->SetMass(1.0f);
	venusRigid->SetUseGravity(false);
	venusRigid->SetPosition(venusNode->GetPosition());

	venusNode->SetAttributeAnimation("Rotation", planetRotation, WM_LOOP);

	/// Initialise other Objects
	//boidSet.Initialise(cache, scene_);
}

// Subscribe to Update event for constant recalling of Update method (Similar to Unity)
void CharacterDemo::SubscribeToEvents()
{
	UnsubscribeFromEvent("SceneUpdate");
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CharacterDemo, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(CharacterDemo, HandlePhysicsPreStep));

	/// SERVER: Called when connected and finished loading into a server
	SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(CharacterDemo, HandleClientFinishedLoading));
	
	SubscribeToEvent(E_CLIENTCREATEUI, URHO3D_HANDLER(CharacterDemo, CreateClientUI));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTCREATEUI);
	SubscribeToEvent(E_CLIENTUPDATEHEALTHUI, URHO3D_HANDLER(CharacterDemo, UpdateClientHealthUI));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTUPDATEHEALTHUI);

	/// Client Ready and Object Authority
	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(CharacterDemo, HandleClientToServerReadyToStart));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);
	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(CharacterDemo, HandleServerToClientObjectID));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);

	/// Collision
	SubscribeToEvent(player.pNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandlePlayerCollision));

	for (int i = 0; i < player.missileSet.missleList->NUM_MISSILES; i++)
	{
		SubscribeToEvent(player.missileSet.missleList[i].pNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandleMissileCollision));
	}

	/// Post Render Draw
	//SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(CharacterDemo, DrawDebugRender));

}

// Handle each Update
void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	/// Handle to input
	Input* input = GetSubsystem<Input>();


	/// If Game is the SERVER
	if (GetSubsystem<Network>()->IsServerRunning())
	{

		/// Grab Update Namespace
		using namespace Update;

		/// Delta Time between frames
		deltaTime = eventData[P_TIMESTEP].GetFloat();

		/// UI focus? Exit Update
		if (GetSubsystem<UI>()->GetFocusElement())
			return;

		/// UI Cursor Showing? Don't Update Game Obejcts
		if (!GetSubsystem<UI>()->GetCursor()->IsVisible())
		{
			player.pNode->SetAnimationEnabled(false);

			/// Rotate Player with Cam Direction
			Quaternion playerRotation = player.pNode->GetRotation();

			if (!player.GetCombatMode())
				player.pNode->SetRotation(playerRotation.Slerp(cameraNode_->GetWorldRotation(), deltaTime * 4.0f));
			else
				player.pNode->SetRotation(playerRotation.Slerp(cameraNode_->GetWorldRotation(), deltaTime * 50.0f));

			// Boid Update Half and Half OPTIMISATION
			if (!halfBoidsFlag)
			{
				for (int i = 0; i < NUM_BOIDSET / 2; i++)
				{
					boidSet[i].Update(deltaTime);
				}

				halfBoidsFlag = !halfBoidsFlag;
			}
			else 
			{
				for (int i = NUM_BOIDSET / 2; i < NUM_BOIDSET; i++)
				{
					boidSet[i].Update(deltaTime);
				}

				halfBoidsFlag = !halfBoidsFlag;
			}

			player.Update(input, deltaTime, pCamera);

			missileUI->SetText("Missile: " + (String)player.missileSet.GetAvaliableMissiles());
		}
		else
		{
			/// Player Menu Animation
			player.pNode->SetAnimationEnabled(true);
		}

		/// Show/Hide menu
		if (input->GetKeyPress(KEY_M))
		{
			/// Main Menu Show Bool
			mainMenuVisible = !mainMenuVisible;

			/// Reset Player animation relative to player position
			PlayerAnimation();
		}
	}
	else if(GetSubsystem<Network>()->GetServerConnection())
	{
		/// Show/Hide menu
		if (input->GetKeyPress(KEY_M))
		{
			/// Main Menu Show Bool
			mainMenuVisible = !mainMenuVisible;

			printf("MainMenu = %i \n", mainMenuVisible);

			/// Reset Player animation relative to player position
			PlayerAnimation();
		}
	}
}

// Handle all Late Update calls
void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	/// Delta Time
	float deltaTime = eventData[Update::P_TIMESTEP].GetFloat();

	Input* input = GetSubsystem<Input>();
	UI* ui = GetSubsystem<UI>();

	/// Menu: No Camera Rotation
	if (!ui->GetCursor()->IsVisible())
	{
		/// Grab current mouse movement YAW + PITCH
		/// Clamp the pitch between -90 and 90 degrees
		IntVector2 mouseMove = input->GetMouseMove();
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);

		if(GetSubsystem<Network>()->IsServerRunning())
		{
			/// Get Rotation Direction
			Quaternion dir(player.pNode->GetRotation().x_, Vector3::UP);
			/// Multiply it by YAW and PITCH on their respective axis
			dir = dir * Quaternion(yaw_, Vector3::UP);
			dir = dir * Quaternion(pitch_, Vector3::RIGHT);

			/// Camera Position Offset
			Vector3 offset(0.0f, -2.0f, 10.0f);
			/// Position camera based PLAYER, OFFSET, and the ROTATION DIRECTION
			Vector3 cameraPos = player.pNode->GetPosition() - dir * offset;
			/// Apply to Camera Node
			cameraNode_->SetPosition(cameraPos);
			cameraNode_->SetRotation(dir);

		}
		else if(clientObjectID_)
		{
			// Move Client Camera Around
			Node* playerNode = this->scene_->GetNode(clientObjectID_);

			/// Get Rotation Direction
			Quaternion dir(playerNode->GetRotation().x_, Vector3::UP);
			/// Multiply it by YAW and PITCH on their respective axis
			dir = dir * Quaternion(yaw_, Vector3::UP);
			dir = dir * Quaternion(pitch_, Vector3::RIGHT);

			/// Camera Position Offset
			Vector3 offset(0.0f, -2.0f, 10.0f);
			/// Position camera based PLAYER, OFFSET, and the ROTATION DIRECTION
			Vector3 cameraPos = playerNode->GetPosition() - dir * offset;
			/// Apply to Camera Node
			cameraNode_->SetPosition(cameraPos);
			cameraNode_->SetRotation(dir);
		}
	}
	else
	{
		/// Retain main menu camera pos
		cameraNode_->SetPosition(player.pNode->GetPosition() + Vector3(5, 1, 0));
		cameraNode_->LookAt(player.pNode->GetPosition(), Vector3::UP, TS_WORLD);
	}

	/// Menu Visibility
	ui->GetCursor()->SetVisible(mainMenuVisible);
	window_->SetVisible(mainMenuVisible);
}

// Creates a new Button
Button * CharacterDemo::CreateButton(const String & text, Font* font, int pHeight, Urho3D::Window * whichWindow, bool enabled)
{
	/// Create Button
	Button* button = whichWindow->CreateChild<Button>();
	button->SetMinHeight(pHeight);
	button->SetStyleAuto();
	button->SetEnabled(enabled);

	/// Create Button Text
	Text* buttonText = button->CreateChild<Text>();
	buttonText->SetFont(font, 12);
	buttonText->SetAlignment(HA_CENTER, VA_CENTER);
	buttonText->SetText(text);
	window_->AddChild(button);

	return button;
}

// Creates a new LineEdit
LineEdit * CharacterDemo::CreateLineEdit(const String & text, int pHeight, Urho3D::Window * whichWindow)
{
	LineEdit* lineEdit = whichWindow->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(pHeight);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText(text);
	window_->AddChild(lineEdit);
	lineEdit->SetStyleAuto();

	return lineEdit;
}

// Handles the Quit Button
void CharacterDemo::HandleQuit(StringHash eventType, VariantMap & evenData)
{
	engine_->Exit();
}

// Handles Client Click Start Game
void CharacterDemo::HandleClientStartGame(StringHash eventType, VariantMap & evenType)
{
	printf("Client has pressed START GAME \n");

	mainMenuVisible = !mainMenuVisible;

	/// Client is still observer
	if (clientObjectID_ == 0) 
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection();

		/// CLIENT: Send to Server Player ID
		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
		}
	}
}

// SERVER: Handles Start Server Button
void CharacterDemo::HandleStartServer(StringHash evenType, VariantMap& evenData)
{
	Log::WriteRaw("SERVER: Server has started! \n");

	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);

	/// Hide Menu
	mainMenuVisible = !mainMenuVisible;

	/// Initialise boids on first pressf
	if (!boidSpawnTrigger)
	{
		for (int i = 0; i < NUM_BOIDSET; i++)
		{
			boidSet[i].Initialise(GetSubsystem<ResourceCache>(), scene_);
		}

		/// Add player to Boid follow list
		Boids::AddPlayer(player.pNode);
		
		boidSpawnTrigger = true;
	}

	// Create Health UI
	healthUI = CreateTextUI("Health: " + (String)player.GetPlayerHealth(), 20, HA_LEFT, VA_BOTTOM);
	missileUI = CreateTextUI("Missiles: " + (String)player.missileSet.GetAvaliableMissiles(), 20, HA_RIGHT, VA_BOTTOM);

	//OpenConsoleWindow();
}

// CLIENT: Handles Connect Button
void CharacterDemo::HandleConnect(StringHash evenType, VariantMap& evenData)
{
	/// Clears scene, Prepares it for receiving
	CreateClientScene();

	/// Network Handle 
	Network* network = GetSubsystem<Network>();

	/// Get inputted address
	String address = serverAddressLineEdit_->GetText().Trimmed();
	
	/// Empty Line Handling
	if (address.Empty())
		address = "localhost";

	/// Specify scene to use as a client for replication
	network->Connect(address, SERVER_PORT, scene_);
}

// SERVER: Handles Client Connecting
void CharacterDemo::HandleClientConnected(StringHash evenType, VariantMap & evenData)
{
	/// Console Log
	printf("(Connected) A client has connected!");

	/// Network to Client
	using namespace ClientConnected;

	/// New Connection -> Store connected player Ptr
	Connection* newConnection = static_cast<Connection*>(evenData[P_CONNECTION].GetPtr());

	/// Tells Client to Load Server Scene
	newConnection->SetScene(scene_);

	// or send to ALL clients
	///network->BroadcastRemoteEvent(E_CLIENTCUSTOMEVENT, true, remoteEventData);

}

// CLIENT + SERVER: Handles Disconnect Button
void CharacterDemo::HandleDisconnect(StringHash evenType, VariantMap& evenData)
{
	printf("HandleDisconnect has been pressed. \n");

	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// Running as Client
	if (serverConnection)
	{
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		clientObjectID_ = 0;

		Start();
	}
	// Running as a server, stop it
	else if (network->IsServerRunning())
	{
		network->StopServer();
		scene_->Clear(true, false);

		Start();
	}
}

// SERVER: Handles Client Disconnecting
void CharacterDemo::HandleClientDisconnected(StringHash evenType, VariantMap & evenData)
{
	/// Console Log
	printf("(Disconnected) A client has disconnected!");
}

// CLIENT: Clears and Prepare's client's scene for networked scene
void CharacterDemo::CreateClientScene()
{
	startClientButton->SetEnabled(true);
}

// Create the server's scene
void CharacterDemo::CreateServerScene()
{
}

// CLIENT: Collect Client pressed Controls
Controls CharacterDemo::FromClientToServerControls()
{
	Input* input = GetSubsystem<Input>();
	Controls controls;

	/// Check which button is pressed
	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(CTRL_SHOOT, input->GetKeyDown(KEY_SPACE));
	controls.Set(CTRL_SHIFT, input->GetKeyDown(KEY_SHIFT));
	controls.Set(CTRL_AIM, input->GetMouseButtonDown(4));

	/// mouse pitch + yaw to server
	controls.yaw_ = yaw_;
	controls.pitch_ = pitch_;

	return controls;
}

// SERVER: Recieve Client Controls and apply to Server Game
void CharacterDemo::ProcessClientControls()
{
	/// Get Network Subsystem
	Network* network = GetSubsystem<Network>();

	/// Get all currently connected players
	const Vector< SharedPtr<Connection> >& connections = network->GetClientConnections();

	/// Iterate through each client in Current Connections
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		/// Get current iterated client
		Connection* connection = connections[i];

		/// Get the client's controlled Player Node from HashMap
		///Node* clientPlayer = serverObjects_[connection];
		Player* clientPlayer = clientObjects_[connection];

		/// If client has no Node -> SKIP
		if (!clientPlayer) continue;

		// ... otherwise, process Controls!

		/// Get Controls sent by iterated Client connection
		const Controls& controls = connection->GetControls();

		/// Handle Client Input
		HandleClientObject(clientPlayer, controls);
	}
}

// Handles Controls for Client
void CharacterDemo::HandleClientObject(Player* clientPlayer, Controls controls)
{
	// Set Client Player Controls
	clientPlayer->SetClientControls(controls);

	// Set Client's Camera Rotation
	Quaternion clientRotation(controls.pitch_, controls.yaw_, 0.0f);
	Quaternion clientDirection = clientPlayer->pNode->GetRotation();
	clientPlayer->pNode->SetRotation(clientDirection.Slerp(clientRotation, deltaTime * 50.0f));

	// Update Client's Player
	clientPlayer->Update(GetSubsystem<Input>(), deltaTime, pCamera);
}

// SERVER: Calls ProcessClientControls() 
// CLIENT: Runs through controls before process (Before the main Physics step)
void CharacterDemo::HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	/// CLIENT ONLY: Collect Controls
	if (serverConnection)
	{
		serverConnection->SetPosition(cameraNode_->GetPosition()); /// set camera pos
		serverConnection->SetControls(FromClientToServerControls()); /// send controls to server

		// Ask SERVER to perform an event!
		///VariantMap remoteEventData;
		///remoteEventData["aValueRemoteValue"] = 0;
		///serverConnection->SendRemoteEvent(E_CLIENTSCUSTOMEVENT, true, remoteEventData);
	}

	/// SERVER ONLY: Read Controls, Apply them if needed
	else if (network->IsServerRunning())
		ProcessClientControls(); /// Apply Controls to SERVER
}

// CLIENT: Handle Sever-Client HashMap ID
void CharacterDemo::HandleServerToClientObjectID(StringHash eventType, VariantMap & eventData)
{
	clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	printf("Client ID : %i \n", clientObjectID_);
}

// Server: Recieve Client Ready-to-Start Event to SERVER
void CharacterDemo::HandleClientToServerReadyToStart(StringHash eventType, VariantMap & eventData)
{
	/// Print to console
	printf("Event sent by the Client and running on Server: Client is ready to start the game \n");

	/// Get current connection
	using namespace ClientConnected;
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

	/// Create new Player Object for the Client to Control
	Player* clientPlayer = new Player();
	clientPlayer->Initialise(GetSubsystem<ResourceCache>(), scene_, true);

	/// Assign it to HashMap
	clientObjects_[newConnection] = clientPlayer;

	/// Finally send the object's node ID using a remote event
	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = clientPlayer->pNode->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);

	/// Ask Client to create UI
	VariantMap remoteClientData;
	remoteClientData["ClientHealth"] = clientPlayer->GetPlayerHealth();
	remoteClientData["MissileCount"] = clientPlayer->missileSet.GetAvaliableMissiles();
	newConnection->SendRemoteEvent(E_CLIENTCREATEUI, true, remoteClientData);

	SubscribeToEvent(clientPlayer->pNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandleClientCollision));

	for (unsigned int i = 0; i < Missile::NUM_MISSILES; i++)
	{
		SubscribeToEvent(clientPlayer->missileSet.missleList[i].pNode, E_NODECOLLISIONSTART, URHO3D_HANDLER(CharacterDemo, HandleClientMissileCollision));
	}
}

// EVENT: Handles client finishing loading 
void CharacterDemo::HandleClientFinishedLoading(StringHash eventType, VariantMap & eventData)
{
	printf("Client has finished loading up the scene from the server \n");
}

// Event: Create Client UI
void CharacterDemo::CreateClientUI(StringHash eventType, VariantMap & eventData)
{
	healthUI = CreateTextUI("Health: " + (String)eventData["ClientHealth"].GetInt(), 20, HA_LEFT, VA_BOTTOM);
	missileUI = CreateTextUI("Missile: " + (String)eventData["MissileCount"].GetInt(), 20, HA_RIGHT, VA_BOTTOM);
}

void CharacterDemo::UpdateClientHealthUI(StringHash eventType, VariantMap & eventData)
{

}

void CharacterDemo::UpdateClientMissileUI(StringHash eventType, VariantMap & eventData)
{

}

// Player Hover Animation
void CharacterDemo::PlayerAnimation()
{
	/// Object Animation 'group'
	SharedPtr<ObjectAnimation> hoveringAnimation(new ObjectAnimation(context_));

	/// Hovering
	SharedPtr<ValueAnimation> hoverAnimation(new ValueAnimation(context_));
	hoverAnimation->SetInterpolationMethod(IM_SPLINE);
	hoverAnimation->SetKeyFrame(0.0f, player.pNode->GetPosition());
	hoverAnimation->SetKeyFrame(3.0f, player.pNode->GetPosition() + Vector3(0, 0.1, 0));
	hoverAnimation->SetKeyFrame(6.0f, player.pNode->GetPosition() + Vector3(0, 0.1, 0));
	hoverAnimation->SetKeyFrame(9.0f, player.pNode->GetPosition());
	hoveringAnimation->AddAttributeAnimation("Position", hoverAnimation);

	/// Tilting
	SharedPtr<ValueAnimation> tiltingAnimation(new ValueAnimation(context_));
	tiltingAnimation->SetInterpolationMethod(IM_SPLINE);
	tiltingAnimation->SetKeyFrame(0.0f, player.pNode->GetRotation());
	tiltingAnimation->SetKeyFrame(3.0f, player.pNode->GetRotation() + Quaternion(0, 0, 35));
	tiltingAnimation->SetKeyFrame(6.0f, player.pNode->GetRotation() + Quaternion(0, 0, -35));
	tiltingAnimation->SetKeyFrame(9.0f, player.pNode->GetRotation());
	tiltingAnimation->SetKeyFrame(12.0f, player.pNode->GetRotation());
	hoveringAnimation->AddAttributeAnimation("Rotation", tiltingAnimation);

	/// Assign ObjectAnimation to Player
	player.pNode->SetObjectAnimation(hoveringAnimation);
}

void CharacterDemo::DrawDebugRender(StringHash eventType, VariantMap & eventData)
{
	scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
}

void CharacterDemo::HandlePlayerCollision(StringHash eventType, VariantMap & eventData)
{
	using namespace NodeCollision;

	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Boid")
	{
		if (player.Damage())
			engine_->Exit();

		healthUI->SetText("Health: " + (String)player.GetPlayerHealth());
	}
}

void CharacterDemo::HandleClientCollision(StringHash eventType, VariantMap & eventData)
{
	using namespace NodeCollision;

	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Boid")
	{

	}
}

void CharacterDemo::HandleClientMissileCollision(StringHash eventType, VariantMap & eventData)
{
	using namespace NodeCollision;

	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Boid")
	{
		otherNode->SetEnabled(false);
	}
}

void CharacterDemo::HandleMissileCollision(StringHash eventType, VariantMap & eventData)
{
	using namespace NodeCollision;

	Node* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());

	if (otherNode->GetName() == "Boid")
	{
		otherNode->SetEnabled(false);
	}
}

Text * CharacterDemo::CreateTextUI(String text, int fontSize, HorizontalAlignment ha, VerticalAlignment va)
{
	Text* textPtr = GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
	textPtr->SetText(text);
	textPtr->SetFont(GetSubsystem<ResourceCache>()->GetResource<Font>("Fonts/Anonymous Pro.ttf"), fontSize);

	textPtr->SetAlignment(ha, va);

	return textPtr;
}