/**___________________________________________________________________________/
@file          ShootScript.cpp
@author        t.junjie@digipen.edu
@co-author     yukang.ou@digipen.edu
@date          9/29/2025

Script to manage shooting.
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "ShootScript.h"
#include "../CoreLib/LayerManager.h"
#include "../CoreLib/UpdateStackManager.h"
#include "../CoreLib/StatsManager.h"
#include "../CoreLib/EventsDispatcher.h"
#include "ProjectileScript.h"
#include "GameobjectPoolScript.h"
void ShootScript::OnStart(Registry& r)
{

	auto goopVec = r.GetEntitiesWithComponent<GameobjectPoolScript>();

	if (goopVec.empty()) {
		// Create one if it does not exist yet
		GameObject go = CreateGameobject();
		go.AddComponent<GameobjectPoolScript>();
		goopVec.push_back(go.GetEntityID());
	}

	GameobjectPoolScript* goop = r.GetComponent<GameobjectPoolScript>(goopVec[0]);
	goop->AddPrefab(particlePrefab);
	goop->AddPrefab(projectilePrefab);

	gameObjectPool = goopVec[0];
	//CEO::Get<EventsDispatcher>()->Subscribe<SpawnParticleEvent>([this](SpawnParticleEvent event) {SpawnParticle(event); });
}

void ShootScript::OnUpdate(Registry& , float dt, bool) {
	timeSinceLastShot += dt;
}
void ShootScript::OnFixedUpdate(Registry& , float , bool) {
}

//void ShootScript::SpawnParticle(SpawnParticleEvent event)
//{
	//GameObject particle = event.particlePool.GetComponent<GameobjectPoolScript>()->GetGameobject();
	//particle.SetActive(true);
	//particle.GetComponent<TransformComponent>()->translate = event.spawnPos;
	//particle.GetComponent<ParticleEmitterComponent>()->enabled = true;
//}

bool ShootScript::TryShoot(Registry& registry, Vec2 shotDir) {
	StatsManager* stats = CEO::Get<StatsManager>();
	// not ready to shoot yet
	float shottime = 1.0f / stats->attackSpeed.Get();
	int burst = stats->burst.Get();

	if (timeSinceLastShot >= shottime)
		projectilesShotThisBurst = 0;

	if (projectilesShotThisBurst < burst) {
		shottime = 1.0f/ burstAttackSpeed;
	}
	if (timeSinceLastShot < shottime)
		return false;
	else {
		timeSinceLastShot = 0;
		if (projectilesShotThisBurst < burst)
			projectilesShotThisBurst++;
	}

	
	shotDir.Normalise();

    //stats->projectileMultishot.AddMultiplier(0.5f);
    //stats->projectileSpread.AddOffset(5.f);

	int multishotCount = stats->projectileMultishot.Get();
	float spreadAngleOrigin = multishotCount > 1 ? ToRad(-stats->projectileSpread.Get() / 2.f) : 0; // start at negative half angle else start at dir
	float spreadAngleStep = multishotCount > 1 ? ToRad(stats->projectileSpread.Get() / (multishotCount - 1)) : 0;			  // angle offset for each subsequent projectile shot
	for (int i = 0; i < multishotCount; ++i)
	{
		Shoot(registry, 
				shotDir,
				stats->projectileSpeed.Get() + stats->movementSpeed.Get(),
				stats->projectileLifetime.Get(),
				static_cast<int>(stats->projectileDamage.Get()),
				stats->projectileSize.Get(),
				Mat3::Rotation(spreadAngleOrigin + spreadAngleStep * i));
	}
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Player\\Shoot.wav", "SFX").Play();
	return true;
}

ProjectileScript* ShootScript::Shoot(Registry& registry,
						Vec2 shotDir,
						float velocity,
						float lifetime,
						int damage,
						float size,
						Mat3 rotation)
{
	shotDir = rotation.TransformVector(shotDir);
	GameObject object = gameObjectPool.GetComponent<GameobjectPoolScript>()->GetGameobject(projectilePrefab.GetPrefabName());
	object.SetActive(true);
	PhysicsComponent* physics = object.GetComponent<PhysicsComponent>();
	TransformComponent* shooterTransform{ GetComponent<TransformComponent>(registry) };

	ProjectileScript* projectile = object.GetComponent<ProjectileScript>();
	physics->velocity =  shotDir * velocity;
	projectile->lifespan = lifetime;
	projectile->timeAlive = 0.f;
	projectile->damage = damage;
	projectile->particlePool = gameObjectPool;
	projectile->prefabName = particlePrefab.GetPrefabName();
	object.GetComponent<TransformComponent>()->scale = { size, size };

	if (CEO::Get<StatsManager>()->aura.GetNetValue() && projectile->auraChild.IsValid()) {
		projectile->auraChild.GetComponent<ActiveComponent>()->isActiveSelf = true;
	}
	if (CEO::Get<StatsManager>()->poison.GetNetValue() && projectile->poisonTrail.IsValid()) {
		projectile->poisonTrail.GetComponent<ParticleEmitterComponent>()->enabled= true;
	}
	if (CEO::Get<StatsManager>()->freeze.GetNetValue() && projectile->freezeTrail.IsValid()) {
		projectile->freezeTrail.GetComponent<ParticleEmitterComponent>()->enabled = true;
	}
	if (CEO::Get<StatsManager>()->slow.GetNetValue() && projectile->slowTrail.IsValid()) {
		projectile->slowTrail.GetComponent<ParticleEmitterComponent>()->enabled = true;
	}
	TransformComponent* projectileTransform{ object.GetComponent<TransformComponent>() };
	//Vec2 offset{ shotDir * fabs(shooterTransform->scale.x / 2.f) };
	projectileTransform->translate = shooterTransform->translate /*+ offset*/;
	return projectile;
}