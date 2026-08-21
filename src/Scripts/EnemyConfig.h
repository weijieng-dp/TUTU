/**___________________________________________________________________________/
@file          EnemyConfig.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple static class used for initializing values that enemies use from a json
file.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Json.h"
#include "../CoreLib/Components.h"
#include "PlayerControllerScript.h"

class EnemyConfig {
public:
	static void Init();

	static std::string eyeAttackAnim;
	static std::string eyeAttackChargeAnim;
	static std::string eyeIdleAnim;
	static std::string eyeChaseAnim;
	static std::string girlIdleAnim;
	static std::string girlIdleAnim2;
	static std::string girlIdleAnim2Alt;
	static std::string girlChaseAnim;
	static std::string girlChaseAnimAlt;
	static std::string girlChargeAnim;
	static std::string girlChargeAnimAlt;
	static std::string oniAttack1Anim;
	static std::string oniAttack2Anim;
	static std::string oniAttack2AnimAlt;
	static std::string oniChaseAnim;
	static std::string oniChaseAnimAlt;
	static std::string guyAttackAnim;
	static std::string guyAttackAnimAlt;
	static std::string guyIdleAnim;
	static std::string guyIdleAnimAlt;
	static std::string larvaAttackAnim;
	static std::string larvaChaseAnim;

	static float eyeIdleMin;
	static float eyeIdleMax;
	static float eyeChaseMax;
	static float girlIdleMin;
	static float girlIdleMax;
	static float girlChaseMax;
	static float guyAttackCd;
	static float larvaIdleMin;
	static float larvaIdleMax;
	static float larvaChaseMax;


	static void OniInit(EnemyComponent* ec, Registry::Entity e);
	static void EyeInit(EnemyComponent* ec, Registry::Entity e);
	static void LarvaInit(EnemyComponent* ec, Registry::Entity e);
	static void GuyInit(EnemyComponent* ec, Registry::Entity e);
};


namespace es {
	enum FLAGS {
		START_ATTACK = 0x0001,
		FINISH_ATTACK = 0x0002,
		EXIT_IDLE = 0x0004,
		GIRL_LOS = 0x0008
	};

	const std::string attack{ "ATTACK" };
	const std::string idle{ "IDLE" };
	const std::string chase{ "CHASE" };

	inline Vec2 GetPlayerPos(Registry& r) {
		std::vector<EntityRegistry::Entity> idList = r.GetEntitiesWithComponent<PlayerControllerScript>(); // basically grabbing player component
		// Initializing the player's position
		if (idList.empty()) { return Vec2(); }
		else return r.GetComponent<TransformComponent>(idList[0])->translate;
	}

	inline EntityRegistry::Entity GetPlayerNum(Registry& r) {
		std::vector<EntityRegistry::Entity> idList = r.GetEntitiesWithComponent<PlayerControllerScript>(); // basically grabbing player component
		if (idList.empty()) { return 0; }
		else return idList[0];
	}

	// temporary aabb function. Collider component will in the future store the entities it collided with, negating the need of this additional check
	inline bool CheckAABB(Registry& r, EntityRegistry::Entity e1, EntityRegistry::Entity e2) {
		CollisionComponent* atcc = r.GetComponent<CollisionComponent>(e1);
		CollisionComponent* plcc = r.GetComponent<CollisionComponent>(e2);
		Vec2 atMin = atcc->worldMin; Vec2 atMax = atcc->worldMax;
		Vec2 plMin = plcc->worldMin; Vec2 plMax = plcc->worldMax;

		return atMin.x <= plMax.x && plMin.x < atMax.x &&
			atMin.y <= plMax.y && plMin.y < atMax.y;
	}
}