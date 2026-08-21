
#include "LarvaProjectileScript.h"
#include "PlayerControllerScript.h"


void LarvaProjectileScript::OnStart(Registry&)
{
}


void LarvaProjectileScript::OnUpdate(Registry&, float, bool)
{

}

void LarvaProjectileScript::OnFixedUpdate(Registry& r, float, bool)
{
	PhysicsComponent* pc = GetComponent<PhysicsComponent>(r);
	pc->velocity *= 0.97f;
}

