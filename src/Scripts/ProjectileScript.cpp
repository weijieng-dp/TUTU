/*!
@file       ProjectileScript.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Script to handle projectile behavior including movement, lifespan management,
and damage dealing on collision.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "ProjectileScript.h"
#include "HealthScript.h"
#include "ShootScript.h"
#include "EnemyStatusEffectScript.h"
#include "../CoreLib/EventsDispatcher.h"
#include "../CoreLib/StatsManager.h"
#include "GameobjectPoolScript.h"
#include "../CoreLib/Camera.h"

void ProjectileScript::OnStart(Registry & )
{
};


void ProjectileScript::OnUpdate(Registry &, float , bool ){};

void ProjectileScript::OnFixedUpdate(Registry& registry, float fixeddt, bool)
{
	timeAlive += fixeddt;
	if (timeAlive > lifespan)
	{
		TransformComponent* tC = GetComponent<TransformComponent>(*CEO::Get<Registry>());

		Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tC->translate);

		Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

		float maxDistance = size.Length() * 1.5f;   // allow beyond screen
		float dist = distanceInView.Length();

		float normalized = dist / maxDistance;
		normalized = std::clamp(normalized, 0.0f, 1.0f);

		// Smooth falloff (quadratic)
		float volume = 1.0f - (normalized);

		float mapValue = distanceInView.x / (size.x);
		mapValue = std::clamp(mapValue, -0.7f, 0.7f);

		int rand = (std::rand() % 2) + 1;
		std::string sound = "SFX\\Player\\BulletHit" + std::to_string(rand) + ".wav";
		if (GetComponent<NameComponent>(registry)->name != "LarvaProjectile")
		{
			CEO::Instance().GetManager<ResourceManager>()->GetAudio(sound).Play(volume, 1, mapValue);
		}
		if (particlePool.GetEntityID() != 0 && !prefabName.empty())
		{
			GameObject particle = particlePool.GetComponent<GameobjectPoolScript>()->GetGameobject(prefabName);
			particle.SetActive(true);
			particle.GetComponent<TransformComponent>()->translate = registry.GetComponent<TransformComponent>(entity)->translate;
			particle.GetComponent<ParticleEmitterComponent>()->enabled = true;
		}
		if (canSpawnSplit) {
			SpawnSplit(registry);
		}
		ReturnToPool(registry);
	}
	else if (CEO::Get<StatsManager>()->homing.GetNetValue() && GetComponent<NameComponent>(registry)->name == "Projectile") {
		if (TransformComponent* comp = GetComponent<TransformComponent>(registry)) {
			Registry::Entity minEnt{ 0 };
			float minDist{};
			Vec2 projPos = comp->translate;

			Vec2 _direction{};

			for (Registry::Entity ent : registry.GetEntitiesWithComponent<EnemyComponent>()) {
				if (registry.GetComponent<ActiveComponent>(ent)->isActiveSelf) {
					Vec2 enemyPos = registry.GetComponent<TransformComponent>(ent)->translate;
					float sqDist = (enemyPos- projPos).LengthSquared();
					if (!minDist || minDist > sqDist) {
						minDist = sqDist;
						minEnt = ent;
						_direction = enemyPos-projPos;
					}
				}
			}
			if (minEnt) {
				PhysicsComponent* physComp = GetComponent<PhysicsComponent>(registry);
				if (physComp) {
					StatsManager* stats = CEO::Get<StatsManager>();
					float projMaxSpeed = stats->projectileSpeed.Get() + stats->movementSpeed.Get();

					Vec2 velocityNorm = physComp->velocity;
					velocityNorm.Normalise();
					_direction.Normalise();
					//if (fabs(_direction.x) < fabs(_direction.y)) {
					//	physComp->netForce.x = 3*(_direction.x < 0 ? -1 : 1) * (projMaxSpeed);
					//	physComp->netForce.y = 0;
					//}
					//else {
					//	physComp->netForce.x = 0;
					//	physComp->netForce.y = 3*(_direction.y < 0 ? -1 : 1) * (projMaxSpeed);
					//}
					float dotProd = Vec2DotProduct(velocityNorm, _direction);
					if (dotProd > 0 && dotProd< 1 - 0.001) {
						//physComp->velocity.x = _direction.x * (projMaxSpeed);
						//physComp->velocity.y = _direction.y * (projMaxSpeed);
						physComp->netForce.x = 3 * _direction.x * (projMaxSpeed);
						physComp->netForce.y = 3 * _direction.y * (projMaxSpeed);
					}
					//physComp->netForce.x = 3 * _direction.x * (projMaxSpeed);
					//physComp->netForce.y = 3 * _direction.y * (projMaxSpeed);

					physComp->velocity.x = std::clamp(physComp->velocity.x, -projMaxSpeed, projMaxSpeed);
					physComp->velocity.y = std::clamp(physComp->velocity.y, -projMaxSpeed, projMaxSpeed);
				}
			}
		}

	}
};

void ProjectileScript::OnTriggerEnter(const Collider& other)
{
	if (lastHit && lastHit == other.entity) return;
	else lastHit = other.entity;
	Registry* registry = CEO::Get<Registry>();

	// spawn particle on collision
	if (particlePool.GetEntityID() != 0 && !prefabName.empty())
	{
		GameObject particle = particlePool.GetComponent<GameobjectPoolScript>()->GetGameobject(prefabName);
		particle.SetActive(true);
		particle.GetComponent<TransformComponent>()->translate = registry->GetComponent<TransformComponent>(entity)->translate;
		particle.GetComponent<ParticleEmitterComponent>()->enabled = true;
	}

	TransformComponent* tC = GetComponent<TransformComponent>(*CEO::Get<Registry>());

	Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tC->translate);

	Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

	float maxDistance = size.Length() * 1.5f;   // allow beyond screen
	float dist = distanceInView.Length();

	float normalized = dist / maxDistance;
	normalized = std::clamp(normalized, 0.0f, 1.0f);

	// Smooth falloff (quadratic)
	float volume = 1.0f - (normalized);

	float mapValue = distanceInView.x / (size.x);
	mapValue = std::clamp(mapValue, -0.7f, 0.7f);

	int rand = (std::rand() % 2) + 1;
	std::string sound = "SFX\\Player\\BulletHit" + std::to_string(rand) + ".wav";
	CEO::Instance().GetManager<ResourceManager>()->GetAudio(sound).Play(volume, 1, mapValue);


	if (registry->HasComponent<HealthScript>(other.entity))
	{
		registry->GetComponent<HealthScript>(other.entity)->TakeDamage(damage);
	}
	else
	{
		HierarchyComponnent* phc = registry->GetComponent<HierarchyComponnent>(other.entity);

		HealthScript* playerHealth{ nullptr };
		if (phc->parent)
		{
			playerHealth = registry->GetComponent<HealthScript>(phc->parent);
			HierarchyComponnent* phc2 = registry->GetComponent<HierarchyComponnent>(phc->parent);
			if (phc2->parent)
			{
				playerHealth = registry->GetComponent<HealthScript>(phc2->parent);
			}

		}
		else
		{
			playerHealth = registry->GetComponent<HealthScript>(other.entity);
		}


		if (playerHealth)
		{
			playerHealth->TakeDamage(damage);
		}
	}

	StatsManager* stats = CEO::Get<StatsManager>();

	ProjectileInteractions(registry, other);

	if (stats->bounce.GetNetValue() && !(registry->HasComponent<EnemyComponent>(other.entity) && stats->pierce.GetNetValue())) {
		PhysicsComponent* physComp = GetComponent<PhysicsComponent>(*registry);
		if (physComp) {
			Vec2 dir = physComp->velocity;
			if (other.GetComponent<CollisionComponent>()->shape == Shape::Circle) {
				physComp->velocity = -dir;
			}
			else if (other.GetComponent<CollisionComponent>()->shape == Shape::Box) {
				Vec2 n{};
				CollisionComponent* collider = GetComponent<CollisionComponent>(*registry);
				CollisionComponent* colliderOther = other.GetComponent<CollisionComponent>();
				if (tC->translate.x + collider->radius >= colliderOther->worldMax.x)	n = { -1,  0 };  // right wall faces left
				if (tC->translate.x - collider->radius <= colliderOther->worldMin.x)    n = { 1,  0 };  // left wall faces right
				if (tC->translate.y + collider->radius >= colliderOther->worldMax.y)	n = { 0, -1 };  // floor faces up
				if (tC->translate.y - collider->radius <= colliderOther->worldMin.y)    n = { 0,  1 };  // ceiling faces down
				n.Normalise();
				physComp->velocity = Vec2Reflect(dir, n);
			}
		}
	}
	else if (!stats->pierce.GetNetValue() || !registry->HasComponent<EnemyComponent>(other.entity)) {
		ReturnToPool(*registry);
	}
};

void ProjectileScript::OnTriggerExit(const Collider&) {
	lastHit = 0;
}

void ProjectileScript::ProjectileInteractions(Registry* registry, Collider const& other) {
	StatsManager* stats = CEO::Get<StatsManager>();
	//--------------------
	// Enemy interactions
	//--------------------
	if (EnemyStatusEffectScript* comp = registry->GetComponent<EnemyStatusEffectScript>(other.entity);
		comp) {
		if (stats->poison.GetNetValue()) {
			double poisonTickCarry = comp->poison.tickTimeElapsed;   // preserve sub-second cadence across re-hits
			comp->poison = stats->poison.GenerateEffectInstance();
			comp->poison.tickTimeElapsed = poisonTickCarry;
		}
		if (stats->slow.GetNetValue()) {
			comp->slow = stats->slow.GenerateEffectInstance();
		}
		if (stats->freeze.GetNetValue()) {
			if (comp->freeze.timeElapsed < comp->freeze.duration) {
				comp->freeze.buildUp++;
				comp->freeze.timeElapsed = 0;
			}
			else {
				comp->freeze = stats->freeze.GenerateEffectInstance();
				comp->freeze.buildUp++;
			}
		}
		if (stats->knockback.GetNetValue()) {
			PhysicsComponent* physComp = GetComponent<PhysicsComponent>(*registry);
			if (physComp) {
				Vec2 vel = physComp->velocity;
				vel.Normalise();
				comp->knockback = stats->knockback.GenerateEffectInstance(vel);
			}
		}
	}

	if (canSpawnSplit) {
		if (registry->HasComponent<EnemyComponent>(other.entity)) {
			SpawnSplit(*registry, other.entity);
		}
		else {
			SpawnSplit(*registry);
		}
	}
}

void ProjectileScript::ReturnToPool(Registry& r) {
	r.GetComponent<ActiveComponent>(this->entity)->isActiveSelf = false;
	canSpawnSplit = true;
	lastHit = 0;
	if (auraChild.IsValid()) {
		auraChild.GetComponent<ActiveComponent>()->isActiveSelf = false;
	}
	if (poisonTrail.IsValid()) {
		poisonTrail.GetComponent<ParticleEmitterComponent>()->enabled = false;
	}
	if (freezeTrail.IsValid()) {
		freezeTrail.GetComponent<ParticleEmitterComponent>()->enabled = false;
	}
	if (slowTrail.IsValid()) {
		slowTrail.GetComponent<ParticleEmitterComponent>()->enabled = false;
	}
}

void ProjectileScript::SpawnSplit(Registry& r, Registry::Entity lastHitEntity) {
	StatsManager* stats = CEO::Get<StatsManager>();
	if (ShootScript* shooter = GetComponent<ShootScript>(r);
		shooter && stats->projectileSplit.Get()
		) {
		if (PhysicsComponent* physComp = GetComponent<PhysicsComponent>(r)) {
			Vec2 dir = physComp->velocity.Normalised();

			Mat3 rotation;
			float offset = PI / (stats->projectileSplit.Get() / 2 + stats->projectileSplit.Get() % 2 + 1);
			for (int i{}; i < stats->projectileSplit.Get(); i++) {
				if (i % 2) {
					rotation = rotation.Rotation(-offset * (i / 2 + 1));
				}
				else {
					rotation = rotation.Rotation(offset * (i / 2 + 1));
				}
				ProjectileScript* proj = shooter->Shoot(
					r,
					dir,
					stats->projectileSpeed.Get() + stats->movementSpeed.Get(),
					stats->projectileLifetime.Get(),
					static_cast<int>(stats->projectileDamage.Get()),
					stats->projectileSize.Get()/2,
					rotation);
				proj->canSpawnSplit = false;
				proj->lastHit = lastHitEntity;
			}
		}
	}
}
