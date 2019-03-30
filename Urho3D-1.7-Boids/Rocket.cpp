#include "Rocket.h"

const float Missile::TIMER_START = 2.0f;

Missile::Missile() : pNode(nullptr), pRigidbody(nullptr),
pCollisionShape(nullptr), pObject(nullptr)
{
}


Missile::~Missile()
{
}

void Missile::RestartTimer()
{
	timer = TIMER_START;

}

void Missile::Initialise(ResourceCache * pRes, Scene * pScene)
{
	timer = TIMER_START;

	// Create Missle Node
	pNode = pScene->CreateChild("Missle");
	pNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
	pNode->SetRotation(Quaternion(0.0f, 0.0f, 0.0f));
	pNode->SetScale(1.0f);

	// Give each Boid a Mesh + Model
	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
	pObject->SetCastShadows(true);

	// Give each Boid a Rigidbody (physics and movement)
	pRigidbody = pNode->CreateComponent<RigidBody>();
	pRigidbody->SetCollisionLayer(2);
	pRigidbody->SetMass(1.0f);
	pRigidbody->SetUseGravity(false);
	pRigidbody->SetAngularFactor(Vector3(1.5f, 1.5f, 1.5f));
	//pRigidbody->SetTrigger(true);


	// Give each boid Collision identifier
	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetBox(Vector3::ONE);

	pObject->SetEnabled(false);
}

void Missile::Update(float timeStep)
{

}