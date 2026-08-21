/**___________________________________________________________________________/
@file          EyeCollisionScript.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Enemy scripting file. Currently contains 
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/

#include "EyeCollisionScript.h"
#include "FlashingVFX.h"
#include "HealthScript.h"
#include "../CoreLib/AudioManager.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/Camera.h"

void EyeCollisionScript::OnTriggerEnter(const Collider& other)
{
	Registry* registry = CEO::Get<Registry>();

	LayerComponent* otherLayer = registry->GetComponent<LayerComponent>(other.entity);
	if ((static_cast<unsigned long long>(1) << otherLayer->layer) & CEO::Get<LayerManager>()->GetLayerMaskByName("Environment"))
	{
		CollisionComponent* cc = registry->GetComponent<CollisionComponent>(entity);
		cc->isTrigger = false; // Exit isTrigger if is trying to jump through wall
	}
}
