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
#include <Urho3D/IO/Log.h>

#include <vector>

#include "Player.h"

namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
}

// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;

class Boids
{
	// Scaling and Ranges of each Force applied
	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;

	static float FAttract_Factor;

	static float FRepel_Factor;
	static float FAlign_Factor;
	static float FAttract_Vmax;

public:
	// Controls force of the boid 
	Vector3 force;

	static int const NUM_BOIDS = 20;

	// GameObject components
	Node* pNode;
	RigidBody* pRigidbody;
	CollisionShape* pCollisionShape;
	StaticModel* pObject;

	Boids();

	~Boids();

	// Sets up Boid components.
	void Initialise(ResourceCache* pRes, Scene* pScene);

	// Called each frame, calculates acting Force from beighbours boids
	void ComputeForce(Boids* boid);

	// Update function called each frame, taking in time since last call
	void Update(float timeStep);

	static void AddPlayer(Node* playerNode);

};

