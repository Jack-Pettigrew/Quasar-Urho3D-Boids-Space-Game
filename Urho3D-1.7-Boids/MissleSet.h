#pragma once
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include "Rocket.h"

class MissleSet
{
private:
	
	int avaliableMissiles = Missile::NUM_MISSILES;

public:

	Missile missleList[Missile::NUM_MISSILES];

	MissleSet();
	~MissleSet();

	void Initialise(ResourceCache* pRes, Scene* pScene);
	void Update(float deltaTime);
	void Shoot(Vector3 playerPosition, Vector3 playerDirection);

	// Returns number of Missiles ready to shoot
	int GetAvaliableMissiles();
};

