#include "Boids.h"

float Boids::Range_FAttract = 80.0f;
float Boids::Range_FRepel = 20.0f;
float Boids::Range_FAlign = 5.0f;
float Boids::FAttract_Vmax = 8.0f;
float Boids::FAttract_Factor = 4.0f;
float Boids::FRepel_Factor = 3.0f;
float Boids::FAlign_Factor = 3.0f;

static Node* target;

Boids::Boids() : pNode(nullptr), pRigidbody(nullptr),
pCollisionShape(nullptr), pObject(nullptr)
{
}

Boids::~Boids()
{

}

// Initialise a Boid
void Boids::Initialise(ResourceCache * pRes, Scene * pScene)
{
	// Create Boid Node
	pNode = pScene->CreateChild("Boid");
	///pNode->SetPosition(Vector3(Random(180.0f) - 90.0f, 30.0f, Random(180.0f) - 90.0f));
	pNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
	pNode->SetScale(2.0f);

	// Give each Boid a Mesh + Model
	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Ship_Type_3.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/sh3.xml"));
	pObject->SetCastShadows(false);

	// Give each Boid a Rigidbody (physics and movement)
	pRigidbody = pNode->CreateComponent<RigidBody>();
	pRigidbody->SetCollisionLayer(2);
	pRigidbody->SetMass(1.0f);
	pRigidbody->SetUseGravity(false);
	pRigidbody->SetPosition(Vector3(Random(360.0f) - 90.0f, Random(360.0f), Random(360.0f) - 90.0f));
	pRigidbody->SetTrigger(true);

	// Give each boid Collision identifier
	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetBox(Vector3(0.75f, 0.5f, 0.5f));

	// Setting initial velocity
	pRigidbody->SetLinearVelocity(Vector3(Random(-20, 20), 0, Random(-20, 20)));
}

// Compute Forces applied to each Boid to stay together along the way
void Boids::ComputeForce(Boids * pBoidList)
{
	// ------------ PLAYER FOLLOW -------------
	/// set the force member variable to zero
	force = Vector3::ZERO;

	Vector3 directiontoPlayer = Vector3::ZERO;

	directiontoPlayer = target->GetPosition() - pNode->GetPosition();

	if(directiontoPlayer.Length() < 50.0)
		force += directiontoPlayer.Normalized() * 100;

	// ------------- COHESION CODE -------------
	Vector3 CoM; /// centre of mass, accumulated total
	int n = 0; /// count number of neigbours

	/// Search Neighbourhood
	for (int i = 0; i < NUM_BOIDS; i++)
	{
		/// the current boid?
		if (this == &pBoidList[i]) continue;

		/// sep = vector position of this boid from current oid
		Vector3 sep = pRigidbody->GetPosition() - pBoidList[i].pRigidbody->GetPosition();

		float d = sep.Length(); //distance of boid

		if (d < Range_FAttract)
		{
			/// with range, so is a neighbour
			CoM += pBoidList[i].pRigidbody->GetPosition();
			n++;
		}

	}

	// Attractive force component
	if (n > 0)
	{
		CoM /= n;

		Vector3 dir = (CoM - pRigidbody->GetPosition()).Normalized();

		Vector3 vDesired = dir * FAttract_Vmax;

		force += (vDesired - pRigidbody->GetLinearVelocity())*FAttract_Factor;
	}

	/// Reset Neighbours
	n = 0;
	
	// ------------- ALLIGNMENT CODE -------------

	/// New Direction Vector
	Vector3 allignmentForce = Vector3::ZERO;

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		/// If iteration is THIS boid
		if (this == &pBoidList[i]) continue;

		/// get seperation direction
		Vector3 sep = pRigidbody->GetPosition() - pBoidList[i].pRigidbody->GetPosition();
		
		/// get magnitude from direction vector
		float d = sep.Length(); 

		/// if within range
		if (d < Range_FAlign)
		{
			/// Add Boid velocity to allignmentForce
			allignmentForce += pBoidList[i].pRigidbody->GetLinearVelocity();
			n++;
		}
	}

	// if there is a neighbour
	if (n > 0)
	{
		/// Average allignmentForce by neighbour count
		allignmentForce /= n;
 
		force += (allignmentForce - pRigidbody->GetLinearVelocity()) * FAlign_Factor;
	}

	/// Reset neighbour count
	n = 0;

	// ------------- SEPERATION CODE -------------
	Vector3 seperation = Vector3::ZERO;

	for (int i = 0; i < NUM_BOIDS; i++)
	{
		/// If iteration is THIS boid
		if (this == &pBoidList[i]) 
			continue; /// skip

		/// get seperation direction
		Vector3 sep = pRigidbody->GetPosition() - pBoidList[i].pRigidbody->GetPosition();

		/// get magnitude from direction vector
		float d = sep.Length();

		/// if within range
		if (d < Range_FRepel)
		{
			/// average direction by magnitude
			seperation += (sep / d);
			n++;
		}
	}

	// if there is a neighbour
	if (n > 0)
	{
		/// Multiply seperation force vector by repel factor
		seperation *= FRepel_Factor;

		/// Apply to overall boid force
		force += seperation;
	}
}

// Updates the Boid's movement by it's computed forces
void Boids::Update(float timeStep)
{
	/// Apply calculated force to boid
	pRigidbody->ApplyForce(force);

	// ------------- CAPPING SPEED -------------

	/// Get Velocity and Magnitude
	Vector3 vel = pRigidbody->GetLinearVelocity();
	float d = vel.Length();

	/// If speed (magnitude) is faster than...
	if (d < 10.0f)
	{
		///...increase / reduce speed
		d = 10.0f;
		pRigidbody->SetLinearVelocity(vel.Normalized()*d);
	}
	else if (d > 50.0f)
	{
		d = 50.0f;
		pRigidbody->SetLinearVelocity(vel.Normalized()*d);
	}

	// ------------- Change direction facing while moving -------------
	Quaternion endRotation = Quaternion(0, 0, 0);

	Vector3 nVelocity = vel.Normalized();

	endRotation.FromLookRotation(nVelocity, Vector3::UP);

	/// Rotate around X axis (facing forward)
	endRotation = endRotation * Quaternion(-90, -90, 0);

	/// Apply
	pRigidbody->SetRotation(endRotation);

	// ------------- CAPPING HEIGHT -------------

	Vector3 p = pRigidbody->GetPosition();

	if (p.y_ < 10.0f)
	{
		p.y_ = 10.0f;
		pRigidbody->SetPosition(p);
	}
	else if (p.y_ > 500.0f)
	{
		p.y_ = 500.0f;
		pRigidbody->SetPosition(p);
	}

}

void Boids::AddPlayer(Node* playerNode)
{
	target = playerNode;
}
