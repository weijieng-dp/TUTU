/**___________________________________________________________________________/
@file          GirlStates.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

StateMachine for the girl enemy

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "../CoreLib/CEO.h"
#include "../CoreLib/Pathfind.h"
#include "../CoreLib/Camera.h"
#include "WeightedRNG.h"

namespace {
	constexpr float pi = 3.14159265358979f;
	bool LineOfSight(const Vec2& startPos, const Vec2& endPos) {
		Pathfind& path = *CEO::Instance().GetManager<Pathfind>();
		Vec2 dir = (endPos - startPos).Normalised();
		Vec2 dirX = dir / dir.x * 320.f * (dir.x < 0 ? -1.f : 1.f);
		Vec2 dirY = dir / dir.y * 320.f * (dir.y < 0 ? -1.f : 1.f);
		Vec2 point = startPos;

		
		int xFlip = dir.x < 0 ? -1 : 1;
		// Stepping in the X direction to find for a collidable block. Stops when point.x is past endPos.
		// First going to make point.x line up with the axis.
		if (xFlip < 0) {	// If moving to the left
			float offset = std::fmodf(point.x, 320);
			if (offset < 0) point += dir / dir.x * (320 + offset) * -1;
			else point += dir / dir.x * offset * -1;
		}
		else {
			float offset = std::fmodf(point.x, 320);
			if (offset < 0) point -= dir / dir.x * offset;
			else point += dir / dir.x * (320 - offset);
		}

		while (xFlip * point.x < xFlip * endPos.x) {
			if (path.IsCollidable(point)) return false;
			point += dirX;
		}

		point = startPos;
		int yFlip = dir.y < 0 ? -1 : 1;
		// Now stepping Y
		if (yFlip < 0) {	// If moving down
			float offset = std::fmodf(point.y, 320);
			if (offset < 0) point += dir / dir.y * (320 + offset) * -1;
			else point += dir / dir.y * offset * -1;
		}
		else {
			float offset = std::fmodf(point.y, 320);
			if (offset < 0) point -= dir / dir.y * offset;
			else point += dir / dir.y * (320 - offset);
		}

		while (yFlip * point.y < yFlip * endPos.y) {
			if (path.IsCollidable(point)) return false;
			point += dirY;
		}

		return true;
	}
}

GirlStateMachine::GirlStateMachine() {
	StateList[es::attack] = std::make_shared<GirlAttackState>();
	StateList[es::idle] = std::make_shared<GirlIdleState>();

	StartState = StateList[es::idle];
	CurrState = StateList[es::idle];
	CurrStateName = es::idle;
}


GirlAttackState::GirlAttackState() {
	transitions.push_back(std::make_shared<AttackIdleTransition>());
}

void GirlAttackState::OnStart(Registry& r, EntityRegistry::Entity e) {

	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	NameComponent* nc = r.GetComponent<NameComponent>(e);
	std::string an = nc->name == "Girl" ? EnemyConfig::girlChargeAnim : EnemyConfig::girlChargeAnimAlt;

	// Settings values to the default
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(an);
	ac->isPlaying = true;
	ec->timeCount = 0;
	ec->enemyStruct.girlStruct.reachTarget = false;
	ec->enemyStruct.girlStruct.ratio = 0.5f;
	ec->enemyStruct.girlStruct.acceleration = 1.f;
	ec->flags = 0;
	pc->velocity = Vec2();

	Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tc->translate);

	Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

	float maxDistance = size.Length() * 1.5f;   // allow beyond screen
	float dist = distanceInView.Length();

	float normalized = dist / maxDistance;
	normalized = std::clamp(normalized, 0.0f, 1.0f);

	// Smooth falloff (quadratic)
	float volume = 1.0f - (normalized);

	float mapValue = distanceInView.x / (size.x);
	mapValue = std::clamp(mapValue, -0.7f, 0.7f);
	std::string rand = std::to_string(WeightedRNG::getRand("Girl") + 1);

	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Enemy\\Girl\\GirlCharge" + rand + ".wav").Play(volume, 1, mapValue);

	std::cout << "enter attack\n";
}

void GirlAttackState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	
	// Note: the variable name "reachTarget" has not changed, but it's purpose has.
	// I'm now using a different method than before to determine when the enemy has
	// "passed" the player. Instead of doing distance checks to a target location,
	// it will now be whenever the angle of direction to player vs current movement
	// direction is greater than 90 degrees. Makes alot more sense than whatever I
	// was trying before.

	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	NameComponent* nc = r.GetComponent<NameComponent>(e);
	std::string an = nc->name == "Girl" ? EnemyConfig::girlChaseAnim : EnemyConfig::girlChaseAnimAlt;


	// This part is managing the startup animation. The rest of the code won't run until 
	// startup is done. If statement checks if the charge up animation is done or not
	if (!((ec->flags & es::START_ATTACK) || ac->isPlaying)) {
		ac->currAnim = &CEO::Get<ResourceManager>()->GetAnimation(an);
		ac->isPlaying = true;
		ec->flags |= es::START_ATTACK;
		ec->targetPos = CEO::Instance().GetManager<Pathfind>()->GetPlayerPos();
		pc->velocity = (ec->targetPos - tc->translate).Normalised() * ec->velocity;
	}
	else {
		if (!(ec->flags & es::START_ATTACK)) {
			// Setting the velocity to 0 for the knockback effect
			pc->velocity = Vec2();
			return; // Terminate early if still "charging up"
		}
	}

	Pathfind& path = *CEO::Instance().GetManager<Pathfind>();
	auto& girlStuff = ec->enemyStruct.girlStruct;

	// Timer tracking, used for various purposes.
	// Tracks how long the enemy has been chasing the player to be used for 
	// acceleration, then tracks how long the enemy has been running past
	// the player to be use for deceleration and rapid turning + retargeting
	ec->timeCount += dt;

	// LOS check
	if (!LineOfSight(tc->translate,path.GetPlayerPos())) {
		ec->flags |= es::GIRL_LOS;	// Adding to the flag if she gains line of sight
	}
	else if (ec->flags & es::GIRL_LOS) {
		ec->flags ^= es::GIRL_LOS;	// And this is removing when she loses line of sight
	}

	// The acceleration is: 1 -> 12 in 2s
	if (!girlStuff.reachTarget) {
		girlStuff.acceleration = std::clamp(girlStuff.acceleration + dt * 5.5f, 1.f, 12.f);
		//girlStuff.acceleration = 5.f;
	}
	else {
		girlStuff.acceleration = std::clamp(girlStuff.acceleration - dt * 11.f, 1.f, 12.f); // She starts decelerating in 1s
	}

	Vec2 playerDir = path.GetPlayerPos() - tc->translate;
	Vec2 girlDir = pc->velocity;

	float turnAngle = Vec2Angle(playerDir, girlDir);
	float negative = Vec2DotProduct(playerDir.Normalised(), { -girlDir.Normalised().y,girlDir.Normalised().x }) > 0 ? 1.f : -1.f;
	turnAngle *= negative;
	// Checking if angle > 90 degrees in either direction
	if (!girlStuff.reachTarget && 
		(turnAngle > pi * 0.5f || turnAngle < -pi * 0.5f )) {
		girlStuff.reachTarget = true;
		ec->timeCount = 0;
	}

	
	if (ec->flags & es::GIRL_LOS) {
		// If she lost line of sight, she just continues in a straight line
		if (girlStuff.reachTarget) {
			// Swap to idle state after she "passes" the player
			ec->flags = es::FINISH_ATTACK;
			return;
		}
	}
	else {
		if (girlStuff.reachTarget) {
			// At this point she starts decelerating, and she gets a much better
			// turn rate instead.
			girlStuff.ratio = std::clamp(girlStuff.ratio + dt * 0.1f, 0.5f, 1.0f);
			playerDir.Normalise(); girlDir.Normalise();

			// If the angle is small enough, it means she is "looking" at the player.
			if (float angle = Vec2Angle(girlDir, playerDir);
				angle < 0.1f && angle > -0.1f) {
				// Reset the state to the start of attack state.
				OnStart(r, e);
				return;
			}

			pc->velocity += ec->velocity * girlStuff.ratio * playerDir;
			pc->velocity.Normalise();
			pc->velocity *= ec->velocity * girlStuff.acceleration;
			return;
		}
		else {
			// This is her standard chase AI before she "passes" the player.
			// Cap her rotation speed to 90 degs/s, aka pi / 2
			turnAngle = std::clamp(turnAngle, -pi / 2.f, pi / 2.f) * dt;

			Mat3 rot; Mat3RotRad(rot, turnAngle);
			Vec2 vel = rot * pc->velocity.Normalised();
			pc->velocity = vel * ec->velocity * girlStuff.acceleration;
		}
	}
}

void GirlAttackState::OnExit(Registry& r, EntityRegistry::Entity e) {

	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	ec->enemyStruct.girlStruct.acceleration = 1;
	ec->enemyStruct.girlStruct.reachTarget = false;
	ec->flags = 0;
}


GirlIdleState::GirlIdleState() {
	transitions.push_back(std::make_shared<IdleAttackTransition>());
}	

void GirlIdleState::OnStart(Registry& r, EntityRegistry::Entity e) {
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	NameComponent* nc = r.GetComponent<NameComponent>(e);
	std::string an = nc->name == "Girl" ? EnemyConfig::girlIdleAnim2 : EnemyConfig::girlIdleAnim2Alt;

	ec->maxTime = 1;
	ec->timeCount = 0;
	ec->targetPos = tc->translate;
	pc->velocity = Vec2();
	ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(an);
}

void GirlIdleState::OnUpdate(Registry& r, EntityRegistry::Entity e, float dt) {
	
	// The idle behavior is: the girl enemy selects a random tile in the general direction of the player,
	// then moves towards it. After reaching said tile, the girl will then staystill on it for a short moment,
	// before finding for a new target tile location.
	// It exits the idle state when the player is in range, and the girl has line of sight with the player.

	PhysicsComponent* pc = r.GetComponent<PhysicsComponent>(e);
	EnemyComponent* ec = r.GetComponent<EnemyComponent>(e);
	TransformComponent* tc = r.GetComponent<TransformComponent>(e);
	AnimatorComponent* ac = r.GetComponent<AnimatorComponent>(e);

	Pathfind& path = *CEO::Instance().GetManager<Pathfind>();

	ec->timeCount += dt;
	
	// Transition to attack if the player is in range and the enemy has line of sight.
	if (((tc->translate - path.GetPlayerPos()).LengthSquared() <= ec->attackRange * ec->attackRange) &&
		LineOfSight(tc->translate, path.GetPlayerPos())) {
		ec->lockFacing = false;
		ec->flags |= es::EXIT_IDLE; // Trigger transition to attack
		return;
	}

	// Checks if the enemy has reached the destination tile.
	if ((ec->targetPos - tc->translate).Length() < 10.f) {
		// Resetting timecount for only the first time this is called.
		if (pc->velocity.x != 0 && pc->velocity.y != 0) ec->timeCount = 0;

		pc->velocity = Vec2();

		// This is if it has reached the location, and has to afk before it can find for a new tile to path towards
		if (ec->timeCount < ec->maxTime) {
			ec->lockFacing = true;
			NameComponent* nc = r.GetComponent<NameComponent>(e);
			std::string an = nc->name == "Girl" ? EnemyConfig::girlIdleAnim2 : EnemyConfig::girlIdleAnim2Alt;
			ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(an);
			return;
		}

		std::pair<Vec2,Vec2> ret = path.MoveTo(tc->translate, path.GetPlayerPos());

		int loopcount{};
		do {
			if (loopcount > 10) break;
			ret.second.Normalise();
			Mat3 rot;

			// Alot of randomness here. This is basically selecting a random tile that is in the approximate direction towards the player.
			// The while checks if the selected tile is collidable or not. If it is, find a new tile.

			float degrees = static_cast<float>(rand()) / RAND_MAX * 90 - 45.f;
			float mag = static_cast<float>(rand()) / RAND_MAX * (EnemyConfig::girlIdleMax - EnemyConfig::girlIdleMin) + EnemyConfig::girlIdleMin + 1.5f;
			float idle = static_cast<float>(rand()) / RAND_MAX * (EnemyConfig::girlIdleMax - EnemyConfig::girlIdleMin) + EnemyConfig::girlIdleMin;
			Mat3RotDeg(rot, degrees);
			ec->targetPos = rot * ret.second * ec->velocity * mag + tc->translate;
			Vec2 vel = (ec->targetPos - tc->translate); vel.Normalise();
			pc->velocity = vel * ec->velocity ;
			ec->maxTime = idle;
			ec->timeCount = 0;
			NameComponent* nc = r.GetComponent<NameComponent>(e);
			std::string an = nc->name == "Girl" ? EnemyConfig::girlIdleAnim2 : EnemyConfig::girlIdleAnim2Alt;
			ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(an);
			loopcount++;

		} while (path.IsCollidable(ec->targetPos));
	}
	else {
		Vec2 vel = (ec->targetPos - tc->translate); vel.Normalise();
		pc->velocity = vel * ec->velocity;

		// If the enemy spent too long path finding towards the tile but never gets considered as "reaching" it, then 
		// this failsafe triggers.
		if (ec->timeCount >= 5.f) {
			ec->timeCount = 0;
			ec->targetPos = tc->translate;
		}
	}
}

void GirlIdleState::OnExit(Registry&, EntityRegistry::Entity) {}