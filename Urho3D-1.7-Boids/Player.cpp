#include "Player.h"
#include "Character.h"

Player::Player() : pNode(nullptr), pRigidbody(nullptr),
	pCollisionShape(nullptr), pModel(nullptr)
{
}


Player::~Player()
{
}

void Player::Initialise(ResourceCache * cache, Scene * scene, bool clientPlayer)
{
	/// Player Node
	pNode = scene->CreateChild("Player");
	pNode->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
	pNode->SetRotation(Quaternion::IDENTITY);
	pNode->SetScale(0.5f);

	/// Mesh and Model
	pModel = pNode->CreateComponent<StaticModel>();
	pModel->SetModel(cache->GetResource<Model>("Models/SpaceShip.mdl"));
	pModel->SetMaterial(cache->GetResource<Material>("Materials/SpaceShipMaterial.xml"));
	pModel->SetCastShadows(true);

	/// Give Rigidbody
	pRigidbody = pNode->CreateComponent<RigidBody>();
	pRigidbody->SetCollisionLayer(2);
	pRigidbody->SetMass(1.0f);
	pRigidbody->SetUseGravity(false);
	pRigidbody->SetPosition(pNode->GetPosition());
	pRigidbody->SetTrigger(true);

	/// Give Collision Box
	pCollisionShape = pNode->CreateComponent<CollisionShape>();
	pCollisionShape->SetBox(Vector3(5.0f, 1.5f, 5.0f));

	/// Particle Effect Offsets
	Vector3 offsetLeft(0.60f, 9.50f, 1.50f), offsetRight(-0.60f, 9.50f, 1.50f);
	Vector3 particleLeftPos(pRigidbody->GetPosition() - offsetLeft), particleRightPos(pRigidbody->GetPosition() - offsetRight);

	/// Particle Effects
	Node* leftParticle = pNode->CreateChild("ParticleLeftEffect");
	leftParticle->SetPosition(particleLeftPos);
	ParticleEmitter* emitter1 = leftParticle->CreateComponent<ParticleEmitter>();
	emitter1->SetEffect(cache->GetResource<ParticleEffect>("Particle/Fire.xml"));

	Node* rightParticle = pNode->CreateChild("ParticleRightEffect");
	rightParticle->SetPosition(particleRightPos);
	ParticleEmitter* emitter2 = rightParticle->CreateComponent<ParticleEmitter>();
	emitter2->SetEffect(cache->GetResource<ParticleEffect>("Particle/Fire.xml"));

	/// Set default speed
	speed = moveSpeed;

	missileSet.Initialise(cache, scene);

	this->clientPlayer = clientPlayer;
}

void Player::Update(Input* input, float deltaTime, Camera* pCamera)
{
	/// Prevent Collision enforced movement
	pRigidbody->SetLinearVelocity(Vector3::ZERO);
	pRigidbody->SetAngularVelocity(Vector3::ZERO);

	/// Player Input
	switch (clientPlayer)
	{
		// Server
	case false:
		/// Player Speed + Combat
		if (input->GetKeyDown(KEY_SHIFT))
		{
			speed = boostSpeed;
			/// SET TO NEW FOV
			if (!pCamera->GetFov() != boostFOV)
			{
				pCamera->SetFov(this->lerp(pCamera->GetFov(), boostFOV, deltaTime * fovSpeed));
				pNode->Translate(Vector3::FORWARD * boostSpeed * deltaTime);
			}
		}
		else if (input->GetMouseButtonDown(4))
		{
			speed = combatSpeed;
			combatMode = true;

			if(!pCamera->GetFov() != moveFOV)
				pCamera->SetFov(this->lerp(pCamera->GetFov(), moveFOV, deltaTime * fovSpeed));

		}
		else /// RESET DEFAULT SPEED & FOV
		{
			speed = moveSpeed;
			combatMode = false;

			if (!pCamera->GetFov() != moveFOV)
				pCamera->SetFov(this->lerp(pCamera->GetFov(), moveFOV, deltaTime * fovSpeed));
		}

		/// Player Movement
		if(input->GetKeyDown(KEY_W))
			pNode->Translate(Vector3::FORWARD * speed * deltaTime);
		if (input->GetKeyDown(KEY_S) && speed != boostSpeed)
			pNode->Translate(Vector3::BACK * speed * deltaTime);
		if (input->GetKeyDown(KEY_A) && speed != boostSpeed)
			pNode->Translate(Vector3::LEFT * speed * deltaTime);
		if (input->GetKeyDown(KEY_D) && speed != boostSpeed)
			pNode->Translate(Vector3::RIGHT * speed * deltaTime);

		/// Shoot Missile
		if (input->GetKeyPress(KEY_SPACE))
		{
			missileSet.Shoot(pNode->GetPosition(), pNode->GetDirection());
		}

		missileSet.Update(deltaTime);
		
		break;

		// Client
	case true:
		/// Player Speed + Combat
		if (controls.buttons_ & CTRL_SHIFT)
		{
			speed = boostSpeed;

		}
		else if (controls.buttons_ & CTRL_AIM)
		{
			speed = combatSpeed;
			combatMode = true;


		}
		else /// RESET DEFAULT SPEED & FOV
		{
			speed = moveSpeed;
			combatMode = false;

		}

		/// Player Movement
		if (controls.buttons_ & CTRL_FORWARD)
			pNode->Translate(Vector3::FORWARD * speed * deltaTime);
		if ((controls.buttons_ & CTRL_BACK) && speed != boostSpeed)
			pNode->Translate(Vector3::BACK * speed * deltaTime);
		if ((controls.buttons_ & CTRL_LEFT) && speed != boostSpeed)
			pNode->Translate(Vector3::LEFT * speed * deltaTime);
		if ((controls.buttons_ & CTRL_RIGHT) && speed != boostSpeed)
			pNode->Translate(Vector3::RIGHT * speed * deltaTime);

		/// Shoot Missile
		if (controls.buttons_ & CTRL_SHOOT)
		{
			missileSet.Shoot(pNode->GetPosition(), pNode->GetDirection());
		}

		missileSet.Update(deltaTime);

		break;
	}

}

bool Player::GetCombatMode()
{
	return combatMode;
}

bool Player::Damage()
{
	health -= 10;

	if (health <= 0)
		return true;
	else
		return false;
}

void Player::SetClientControls(Controls clientControls)
{
	controls = clientControls;
}

int Player::GetPlayerHealth()
{
	return health;
}
