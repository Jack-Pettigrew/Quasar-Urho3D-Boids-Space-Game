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

namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
}

using namespace Urho3D;

class Missile
{
public:
	static const float TIMER_START;
	float timer;
	bool respawned = true;

	static int const NUM_MISSILES = 5;

	/// GameObject components
	Node* pNode;
	RigidBody* pRigidbody;
	CollisionShape* pCollisionShape;
	StaticModel* pObject;

	Missile();
	~Missile();

	void RestartTimer();

	// Sets up Boid components.
	void Initialise(ResourceCache* pRes, Scene* pScene);

	// Update function called each frame, taking in time since last call
	void Update(float timeStep);
};

