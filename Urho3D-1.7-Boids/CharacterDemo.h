#pragma once

#include "Sample.h"
#include "Player.h"

namespace Urho3D
{
	class Node;
	class Scene;
	class Window;
}

class CharacterDemo : public Sample
{
    URHO3D_OBJECT(CharacterDemo, Sample);

public:
    // Construct.
    CharacterDemo(Context* context);
    // Destruct.
    ~CharacterDemo();

    // Setup after engine initialization and before running the main loop.
    virtual void Start();

	/// a urho3d window:
	SharedPtr<Window> window_;
	SharedPtr<Window> mpWindow_;

	// Mouse Sensitivity
	const float MOUSE_SENSITIVITY = 0.1f;

	// Menu Visibility
	bool mainMenuVisible = true;

	// Initial Boid Spawn
	bool boidSpawnTrigger = false;

	// Connect Address
	LineEdit* serverAddressLineEdit_;
	Button* startClientButton;

	// UI Pointers
	Text* healthUI;
	Text* missileUI;

	// Networking Port
	static const unsigned short SERVER_PORT = 2345;

	// ID for each connected client
	unsigned clientObjectID_ = 0;

	// HashMap for recording connected Players and their Objects
	HashMap<Connection*, Player*> clientObjects_;
	/// HashMap<Connection*, WeakPtr<Node>> serverObjects_;

protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    virtual String GetScreenJoystickPatchString() const { return
        "<patch>"
        "    <add sel=\"/element\">"
        "        <element type=\"Button\">"
        "            <attribute name=\"Name\" value=\"Button3\" />"
        "            <attribute name=\"Position\" value=\"-120 -120\" />"
        "            <attribute name=\"Size\" value=\"96 96\" />"
        "            <attribute name=\"Horiz Alignment\" value=\"Right\" />"
        "            <attribute name=\"Vert Alignment\" value=\"Bottom\" />"
        "            <attribute name=\"Texture\" value=\"Texture2D;Textures/TouchInput.png\" />"
        "            <attribute name=\"Image Rect\" value=\"96 0 192 96\" />"
        "            <attribute name=\"Hover Image Offset\" value=\"0 0\" />"
        "            <attribute name=\"Pressed Image Offset\" value=\"0 0\" />"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"Label\" />"
        "                <attribute name=\"Horiz Alignment\" value=\"Center\" />"
        "                <attribute name=\"Vert Alignment\" value=\"Center\" />"
        "                <attribute name=\"Color\" value=\"0 0 0 1\" />"
        "                <attribute name=\"Text\" value=\"Gyroscope\" />"
        "            </element>"
        "            <element type=\"Text\">"
        "                <attribute name=\"Name\" value=\"KeyBinding\" />"
        "                <attribute name=\"Text\" value=\"G\" />"
        "            </element>"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">1st/3rd</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"F\" />"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Jump</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"SPACE\" />"
        "        </element>"
        "    </add>"
        "</patch>";
    }

private:
    // Create static scene content.
    void CreateScene();
    // Subscribe to necessary events.
    void SubscribeToEvents();
    // Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    // Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

	void CreateMainMenu();
	Button* CreateButton(const String& text, Font* font, int pHeight, Urho3D::Window*
		whichWindow, bool enabled);
	LineEdit* CreateLineEdit(const String& text, int pHeight, Urho3D::Window*
		whichWindow);
	void HandleQuit(StringHash eventType, VariantMap& evenData);
	///void HandleStartPlay(StringHash eventType, VariantMap& evenData);
	
	/// Server/Client Methods
	void HandleClientStartGame(StringHash eventType, VariantMap& eventData);
	void HandleConnect(StringHash evenType, VariantMap & evenData);
	void HandleDisconnect(StringHash evenType, VariantMap & evenData);
	void HandleStartServer(StringHash evenType, VariantMap & evenData);
	void HandleClientConnected(StringHash evenType, VariantMap & evenData);
	void HandleClientDisconnected(StringHash evenType, VariantMap & evenData);
	Controls FromClientToServerControls();
	void ProcessClientControls();
	void HandleClientObject(Player* clientPlayer, Controls controls);
	void HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData);
	void HandleClientFinishedLoading(StringHash eventType, VariantMap& eventData);

	/// Client Gameplay Events

	//  Creates Client's UI
	void CreateClientUI(StringHash eventType, VariantMap& eventData);
	// Updates Client's Health UI
	void UpdateClientHealthUI(StringHash eventType, VariantMap & eventData);
	// Updates Client's Missile UI
	void UpdateClientMissileUI(StringHash eventType, VariantMap & eventData);
	// Handles Client Collision
	void HandleClientCollision(StringHash eventType, VariantMap & eventData);
	// Handles Client Missile Collision
	void HandleClientMissileCollision(StringHash eventType, VariantMap & eventData);
	
	// Handles Player Collision
	void HandlePlayerCollision(StringHash eventType, VariantMap & eventData);
	void HandleMissileCollision(StringHash eventType, VariantMap& eventData);
	void HandleServerToClientObjectID(StringHash eventType, VariantMap& eventData);
	void HandleClientToServerReadyToStart(StringHash eventType, VariantMap& eventData);
	void CreateClientScene();
	void CreateServerScene();

	// Calls Player Animation to run
	void PlayerAnimation();
	
	// Draws Debug Geometry
	void DrawDebugRender(StringHash eventType, VariantMap& eventData);

	/// UI Methods
	Text* CreateTextUI(String text, int fontSize, HorizontalAlignment ha, VerticalAlignment va);
	
};