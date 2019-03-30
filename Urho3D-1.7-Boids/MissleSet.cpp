#include "MissleSet.h"



MissleSet::MissleSet()
{
}

MissleSet::~MissleSet()
{
}

void MissleSet::Initialise(ResourceCache * pRes, Scene * pScene)
{
	for (int i = 0; i < Missile::NUM_MISSILES; i++)
	{
		missleList[i].Initialise(pRes, pScene);
	}
}

void MissleSet::Update(float deltaTime)
{
	for (int i = 0; i < Missile::NUM_MISSILES; i++)
	{
		missleList[i].Update(deltaTime);

		if (missleList[i].pObject->IsEnabled())
		{
			missleList[i].timer -= deltaTime;

			if (missleList[i].timer <= 0)
			{
				missleList[i].pObject->SetEnabled(false);
				missleList[i].respawned = true;
				avaliableMissiles++;
				return;
			}
		}
	}
}

void MissleSet::Shoot(Vector3 playerPosition, Vector3 playerDirection)
{
	for (int i = 0; i < Missile::NUM_MISSILES; i++)
	{
		if (missleList[i].respawned)
		{
			missleList[i].RestartTimer();
	
			missleList[i].pRigidbody->SetPosition(playerPosition + Vector3(playerDirection * 3));
			missleList[i].pRigidbody->SetLinearVelocity(playerDirection * 100.0f);
			missleList[i].pObject->SetEnabled(true);

			missleList[i].respawned = false;
			avaliableMissiles--;

			return;
		}
	}
	
}

int MissleSet::GetAvaliableMissiles()
{
	return avaliableMissiles;
}
