#pragma once
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
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
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include<Urho3D/Scene/ObjectAnimation.h>
#include<Urho3D/Scene/ValueAnimation.h>
#include<Urho3D/Input/Controls.h>

#include "MissleSet.h"
#include <algorithm>

namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
}

using namespace Urho3D;

class Player
{
private:
	const float moveFOV = 45.0f;
	const float boostFOV = 90.0f;
	const float fovSpeed = 10.0f;

	const float boostSpeed = 60.0f;
	const float moveSpeed = 20.0f;
	const float combatSpeed = 5.0f;
	float speed = 0.0f;
	   
	bool combatMode = false;

	int health = 100;

	bool clientPlayer = false;

public:
	/// GameObject components
	Node* pNode;
	RigidBody* pRigidbody;
	CollisionShape* pCollisionShape;
	StaticModel* pModel;
	Camera* camera;

	// Client's Controls
	Controls controls;

	/// Missile Group
	MissleSet missileSet;

	Player();
	~Player();

	void Initialise(ResourceCache * cache, Scene * scene, bool clientPlayer);

	void Update(Input* input, float deltaTime, Camera* pCamera);

	/// As referenced from: https://stackoverflow.com/questions/4353525/floating-point-linear-interpolation
	float lerp(float firstFloat, float secondFloat, float time)
	{
		return firstFloat + time * (secondFloat - firstFloat);
	};

	// Return Combat Mode flag
	bool GetCombatMode();

	// Handles Damage and Returns Player Death
	bool Damage();

	// Set's Player Controls as Client's sent Controls
	void SetClientControls(Controls clientControls);

	int GetPlayerHealth();

};

