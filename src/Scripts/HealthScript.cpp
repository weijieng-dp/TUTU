/*!
@file       HealthScript.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Script to represent health on an entity, handles damage taking and provides
interface for callbacks to handle damage taking

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "HealthScript.h"
#include "PlayerControllerScript.h"
void HealthScript::OnStart(Registry & )
{
};


void HealthScript::OnUpdate(Registry &, float , bool )
{
};

void HealthScript::OnFixedUpdate(Registry&, float, bool)
{
}
void HealthScript::TakeDamage(int damage)
{
	PlayerControllerScript* PlayerController = CEO::Get<Registry>()->GetComponent<PlayerControllerScript>(entity);
	if (PlayerController)
	{
		if (PlayerController->timeSinceDamage < PlayerController->iFrame) {
			return;
		}
		else
		{
			PlayerController->timeSinceDamage = 0;
		}
	}

	currHealth -= damage;
	if (onHitCallback)
		onHitCallback(damage);

	if (currHealth <= 0 && onDeathCallback)
		onDeathCallback();
}
;
