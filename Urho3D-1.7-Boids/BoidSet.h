#pragma once
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include "Boids.h"

class BoidSet
{
public:
	
	Boids boidList[Boids::NUM_BOIDS];

	BoidSet();
	~BoidSet();

	void Initialise(ResourceCache* pRes, Scene* pScene);
	void Update(float tm);
};

