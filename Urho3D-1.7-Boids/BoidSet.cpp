#include "BoidSet.h"

BoidSet::BoidSet()
{
}


BoidSet::~BoidSet()
{
}

void BoidSet::Initialise(ResourceCache * pRes, Scene * pScene)
{
	for (int i = 0; i < Boids::NUM_BOIDS; i++)
	{
		boidList[i].Initialise(pRes, pScene);
	}
}

void BoidSet::Update(float tm)
{
	for (int i = 0; i < Boids::NUM_BOIDS; i++)
	{

		boidList[i].ComputeForce(&boidList[0]);
		boidList[i].Update(tm);
	}
}
