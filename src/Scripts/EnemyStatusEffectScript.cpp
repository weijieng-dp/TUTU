#include "EnemyStatusEffectScript.h"
#include "GameobjectPoolScript.h"
void EnemyStatusEffectScript::OnStart(Registry & r)
{
	enemyComp = this->GetComponent<EnemyComponent>(r);
	hp = this->GetComponent<HealthScript>(r);
	physics = this->GetComponent<PhysicsComponent>(r);

	if (enemyComp) {
		initialMS = static_cast<double>(enemyComp->velocity);
	}
	vfxScript = this->GetComponent<FlashingVFX>(r);
	if (vfxScript) {
		originalColor = vfxScript->colorToBlink;
		originalFlashDuration = vfxScript->blinkTotalTime;
	}

	auto goopVec = r.GetEntitiesWithComponent<GameobjectPoolScript>();
	if (goopVec.empty()) {
		// Create one if it does not exist yet
		GameObject go = CreateGameobject();
		go.AddComponent<GameobjectPoolScript>();
		goopVec.push_back(go.GetEntityID());
	}

	objPool = goopVec[0];
	objPool.GetComponent<GameobjectPoolScript>()->AddPrefab(freezeParticle.GetPrefabName());

	// Add a poison particle emitter to entity
	poisonParticle = poisonParticle.Instantiate();
	poisonParticle.SetParent(entity);
	// set emitter radius to half width
	poisonParticle.GetComponent<ParticleEmitterComponent>()->radius = r.GetComponent<TransformComponent>(entity)->scale.x * 0.5f;
};


void EnemyStatusEffectScript::OnUpdate(Registry &r, float dt, bool)
{
	if (enemyComp) {
		if (poison.duration) {
			if (vfxScript && poison.timeElapsed > poison.duration) {
				poison = PoisonEffect{ 0,0 };
				if (vfxScript) {
					vfxScript->colorToBlink = originalColor;
					vfxScript->blinkTotalTime = originalFlashDuration;
					poisonParticle.GetComponent<ParticleEmitterComponent>()->enabled = false;
				}
			}
			else {
				if (vfxScript && poison.timeElapsed == 0) {
					vfxScript->colorToBlink = poisonColor;
					vfxScript->blinkTotalTime = poison.duration;
					vfxScript->StartFlashing();
					poisonParticle.GetComponent<ParticleEmitterComponent>()->enabled = true;
				}
				if (hp) {
					hp->currHealth = static_cast<int>(poison.UpdateCallback(dt, static_cast<double>(hp->currHealth)));
				}
			}
		}
		if (slow.duration) {
			if (slow.timeElapsed > slow.duration) {
				slow = SlowEffect{ 0,0 };
				enemyComp->velocity = static_cast<float>(initialMS);
				if (vfxScript) {
					vfxScript->colorToBlink = originalColor;
					vfxScript->blinkTotalTime = originalFlashDuration;
				}
			}
			else {
				if (vfxScript && slow.timeElapsed == 0) {
					vfxScript->colorToBlink = slowColor;
					vfxScript->blinkTotalTime = slow.duration;
					vfxScript->StartFlashing();
				}
				enemyComp->velocity = static_cast<float>(slow.UpdateCallback(dt, initialMS));
			}
		}
		if (freeze.duration) {
			if (vfxScript && freeze.timeElapsed > freeze.duration) {
				enemyComp->velocity = static_cast<float>(initialMS);
				if (vfxScript) {
					vfxScript->colorToBlink = originalColor;
					vfxScript->blinkTotalTime = originalFlashDuration;
				}
			}
			else {
				if (vfxScript && freeze.timeElapsed == 0) {
					vfxScript->colorToBlink = freezeColor;
					vfxScript->blinkTotalTime = freeze.duration;
					vfxScript->StartFlashing();
				}
				if (hp) {
					int newHp = static_cast<int>(freeze.UpdateCallback(dt, static_cast<double>(hp->currHealth)));
					if(hp->currHealth != newHp) {
						hp->currHealth = newHp;
						GameObject particle = objPool.GetComponent<GameobjectPoolScript>()->GetGameobject(freezeParticle.GetPrefabName());
						particle.SetActive(true);
						particle.GetComponent<TransformComponent>()->translate = r.GetComponent<TransformComponent>(entity)->translate;
						particle.GetComponent<ParticleEmitterComponent>()->enabled = true;
					}
				}
			}
		}
		if (knockback.duration) {
			if (physics) {
				if (knockback.UpdateCallback(dt, 0)) {
					physics->netForce.x = knockback.direction.x * knockback.effectiveness;
					physics->netForce.y = knockback.direction.y * knockback.effectiveness;
				}
				else {
					physics->netForce.x = 0;
					physics->netForce.y = 0;
					physics->velocity.x = 0;
					physics->velocity.y = 0;
					knockback.duration = 0;
				};
			}
		}
	}
};

void EnemyStatusEffectScript::OnFixedUpdate(Registry&, float, bool)
{
};
