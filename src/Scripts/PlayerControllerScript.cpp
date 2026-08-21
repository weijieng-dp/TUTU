/*!
@file       PlayerController.cpp
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Implements the PlayerController script, which handles player input
			and applies position updates to the TransformComponent each frame.
			This script reads keyboard inputs and moves the player accordingly.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#include "PlayerControllerScript.h"
#include "../CoreLib/InputManager.h"
#include "memory.h"
#include "FlashingVFX.h"
#include "HealthbarScript.h"
#include "../CoreLib/ResourceManager.h"
#include "../CoreLib/StatsManager.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/Camera.h"
#include "../CoreLib/HierarchyManager.h"
#include "../CoreLib/EventsDispatcher.h"
#include "../CoreLib/Pathfind.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/Android/Haptics.h"
#include <cmath>
#include "FlashingVFX.h"
#include "ShootScript.h"
#include "HealthScript.h"
#include "CameraControllerScript.h"
#include "DialogueScript.h"
#include "../CoreLib/PersistentDataManager.h"
#include "../CoreLib/MapManager.h"



void PlayerControllerScript::SetPlayerHealth()
{
	HealthScript* health = GetComponent<HealthScript>(*CEO::Get<Registry>());
	health->maxHealth = CEO::Get<StatsManager>()->maxHealth.Get();
	health->currHealth = CEO::Get<StatsManager>()->health.Curr();
	health->onHitCallback = std::bind(&PlayerControllerScript::OnDamageTaken, this, std::placeholders::_1);
	health->onDeathCallback = std::bind(&PlayerControllerScript::OnDeath, GetComponent< PlayerControllerScript>(*CEO::Get<Registry>()));

	HealthbarScript* healthbar = HealthbarGO.GetComponent<HealthbarScript>();
	healthbar->TargetGameobject = entity;
}

void PlayerControllerScript::SetAttackStat()
{

}

void PlayerControllerScript::OnStart(Registry& registry)
{
#ifdef PLATFORM_ANDROID

	Input::GetControllers()[0].active = true;
	Input::GetControllers()[1].active = true;
#endif // DEBUG
#ifdef PLATFORM_WINDOWS
	CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "crosshair"});
#endif

	for (int i = 1; i <= 9; i++)
	{
		std::string footstepnumber = "SFX\\Player\\Footstep" + std::to_string(i) + ".wav";
		footsteps.push_back(&CEO::Instance().GetManager<ResourceManager>()->GetAudio(footstepnumber));
	}
	
	playerTransform = GetComponent<TransformComponent>(registry);
	playerVelocity = GetComponent<PhysicsComponent>(registry);
	playerAnimator = GetComponent<AnimatorComponent>(registry);
	spriterenderer = GetComponent<SpriteRendererComponent>(registry);
	PlayerRun = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_run.anim");
	PlayerIdle = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_idle.anim");
	PlayerDamage = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_damage.anim");
	PlayerDeath = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_death.anim");
	movementSpeed = CEO::Get<StatsManager>()->movementSpeed.Get();

	HealthbarGO = HealthbarPrefab.Instantiate();

	SetPlayerHealth();

	CEO::Get<Pathfind>()->SetPlayerID(entity);
};

/*!
* \brief
*	Per-frame update for handling input and moving the player entity.
*	Processes keyboard input (WASD) and applies translation to the
*	entity's TransformComponent.
* \param
*	registry - ECS registry instance used to access entity components
* \param
*	dt - delta time between frames
* \param
*	firstframe - true if this is the first frame after startup
*/
void PlayerControllerScript::OnFixedUpdate(Registry& registry, float dt, bool )
{
	if (deathconfirmed) return;

	CEO::Get<Pathfind>()->SetPlayerPos(GetComponent<TransformComponent>(registry)->translate);
	timeSinceDamage += dt;
	timeSinceDamage = std::clamp(timeSinceDamage, 0.0f, 3.0f);
	if (timeSinceDamage < 0.3) return;
	else
	{
		spriterenderer->visible = true;

		// it is facing left as default
		// it should be facing right when velocity is more than 1 and if it is facing left
		// it should be facing left when velocity is less than -1 and if it is facing right
		if (playerVelocity->velocity.Length() > 1)
		{
			static int previousFrameCount = -1;
			if (!changeAnim)
			{
				changeAnim = true;
				playerAnimator->currFrame = 0;
				playerAnimator->currFrame =
					previousFrameCount == playerAnimator->currFrame ? previousFrameCount += 7 : playerAnimator->currFrame;
			}
			if (playerAnimator->currFrame % 7 == 0 && (previousFrameCount != playerAnimator->currFrame))
			{

				TransformComponent* tC = GetComponent<TransformComponent>(*CEO::Get<Registry>());

				Vec2 distanceInView = CEO::Get<CameraManager>()->GetView() * (tC->translate);
				Vec2 size = CEO::Get<CameraManager>()->GetScreenSize() * 0.7f;

				float mapValue = distanceInView.x / (size.x);
				mapValue = std::clamp(mapValue, -0.7f, 0.7f);

				int rand = std::rand() % footsteps.size();

				footsteps[rand]->Play(0.5f,1.0f,mapValue);
			}
			previousFrameCount = playerAnimator->currFrame;

			playerAnimator->currAnim = PlayerRun;
			TrialPath.GetComponent<ParticleEmitterComponent>()->enabled = true;


		}
		else if (playerVelocity->velocity.Length() == 0)
		{
			playerAnimator->currAnim = PlayerIdle;
			changeAnim = false;
			TrialPath.GetComponent<ParticleEmitterComponent>()->enabled = false;

		}
	}

	float& PlayerScaleX = GetComponent<TransformComponent>(registry)->scale.x;
	if (movementDirection.x < 0)
	{
		PlayerScaleX = abs(PlayerScaleX);
	}
	else if (movementDirection.x > 0)
	{
		PlayerScaleX = -abs(PlayerScaleX);
	}


	movementDirection.Normalise();

	float moveX = movementDirection.x * movementSpeed;
	float moveY = movementDirection.y * movementSpeed;

	playerVelocity->velocity.x = moveX;
	playerVelocity->velocity.y = moveY;
};

/*!
* \brief
*	Called every fixed timestep for physics-based or time-independent updates.
*	This implementation currently performs no fixed updates.
* \param
*	registry - ECS registry instance used to access entity components
* \param
*	dt - fixed timestep delta
* \param
*	firstframe - true if this is the first fixed update after startup
*/
void PlayerControllerScript::OnUpdate(Registry& registry, float dt, bool)
{

	playerTransform = GetComponent<TransformComponent>(registry);
	playerVelocity = GetComponent<PhysicsComponent>(registry);
	playerAnimator = GetComponent<AnimatorComponent>(registry);
	spriterenderer = GetComponent<SpriteRendererComponent>(registry);
	
	auto inCombat{ CEO::Get<PersistentDataManager>()->Get<bool>("InCombat") };

	if (inCombat && !*inCombat)
	{
		idleTimer += dt;

		if (idleTimer >= 15.0f)
		{
			idleTimer = 0.0f;
			for (Registry::Entity ent : registry.GetEntitiesWithComponent<DialogueScript>()) {
				registry.GetComponent<DialogueScript>(ent)->Trigger("IDLE");
			}
		}
	}

	if (deathconfirmed )
	{
		playerTransform->scale.x = 470.0f;
		if (playerAnimator->currFrame == 7)
		{
			CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("LOSE");
			SceneManager::QueueSceneAction("Lose", SceneManager::PUSH);
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Am I Stuck Forever.wav");
		}
		return;
	}

	movementDirection.x = 0;
	movementDirection.y = 0;


#ifdef PLATFORM_WINDOWS
    if (Input::GetGamepad(0).connected)
	{
		movementDirection = Input::GetLeftStick();
	}
	else
	{
		if (Input::IsKeyHeld(GLFW_KEY_A))
		{

			movementDirection.x = -1;



		}
		if (Input::IsKeyHeld(GLFW_KEY_D))
		{
			movementDirection.x = 1;


		}
		if (Input::IsKeyHeld(GLFW_KEY_W))
		{
			movementDirection.y = 1;


		}
		if (Input::IsKeyHeld(GLFW_KEY_S))
		{
			movementDirection.y = -1;


		}
		if (Input::IsKeyPressed(GLFW_KEY_P))
		{
			TakeDamage(1);
		}
		// CHEATS
		if (Input::IsKeyHeld(GLFW_KEY_RIGHT_CONTROL) && Input::IsKeyHeld(GLFW_KEY_LEFT_CONTROL)) {
			ItemManager* items = CEO::Get<ItemManager>();
			StatsManager* stats = CEO::Get<StatsManager>();
			if (Input::IsKeyPressed(GLFW_KEY_1)) {
				items->AddItem(items->GetItemList().find("Green Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_2)) {
				items->AddItem(items->GetItemList().find("Blue Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_3)) {
				items->AddItem(items->GetItemList().find("Chocolate Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_4)) {
				items->AddItem(items->GetItemList().find("Magnet Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_5)) {
				items->AddItem(items->GetItemList().find("Knife-fuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_6)) {
				items->AddItem(items->GetItemList().find("Mallet Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_7)) {
				items->AddItem(items->GetItemList().find("Split Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_8)) {
				items->AddItem(items->GetItemList().find("Daifuku")->second, *stats);
			}
			if (Input::IsKeyPressed(GLFW_KEY_9)) {
				items->AddItem(items->GetItemList().find("Bouncy Mochi")->second, *stats);
			}
		}
	}

#endif
#ifdef PLATFORM_ANDROID
		movementDirection = Input::GetControllers()[0].GetJoystickValue();


#endif


#ifdef PLATFORM_WINDOWS
	if (Input::IsButtonHeld(GLFW_MOUSE_BUTTON_LEFT)) {
		Vec2 mouseWorldPos = CEO::Get<CameraManager>()->ScreenToWorldPos(Vec2{ Input::GetX(), Input::GetY() });
		Vec2 shootDir = (mouseWorldPos - playerTransform->translate).Normalised();
		GetComponent<ShootScript>(registry)->TryShoot(registry, shootDir);
	}
	if (Input::GetRightStick().x || Input::GetRightStick().y)
	{
		GetComponent<ShootScript>(registry)->TryShoot(registry, Input::GetRightStick());
	}

	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {

		CEO::Get<SceneManager>()->QueueSceneAction("PauseMenu", SceneManager::PUSH);
	}
#else	// android
	if (Input::GetControllers()[1].GetJoystickValue().x || Input::GetControllers()[1].GetJoystickValue().y) {
		GetComponent<ShootScript>(registry)->TryShoot(registry, Input::GetControllers()[1].GetJoystickValue());

	}
#endif
};

void PlayerControllerScript::TakeDamage(int dmg) {
    if (timeSinceDamage < iFrame) return;
	HealthScript* health = GetComponent<HealthScript>(*CEO::Get<Registry>());
	health->TakeDamage(dmg);



}

void PlayerControllerScript::OnDamageTaken(int)
{
		timeSinceDamage = 0;

		// update stats and UI
		HealthScript* health = GetComponent<HealthScript>(*CEO::Get<Registry>());
		HealthbarGO.GetComponent<HealthbarScript>()->UpdateHealthbar(health->currHealth, health->maxHealth);
		CEO::Get<StatsManager>()->health.Curr(health->currHealth);
		health->currHealth = CEO::Get<StatsManager>()->health.Curr();

		// damage feedback
		playerAnimator->currAnim = PlayerDamage;
		playerAnimator->currAnim->doLooping = false;
		GetComponent<FlashingVFX>(*CEO::Get<Registry>())->StartFlashing();
		//GetComponent<AudioComponent>(*CEO::Get<Registry>())->audio->Play();
		int rand = std::rand() % 3;
		std::vector<std::string> playerDamagePath{
			"SFX\\Player\\PlayerDamage1.wav",
			"SFX\\Player\\PlayerDamage2.wav",
			"SFX\\Player\\PlayerDamage3.wav"
		};
		CEO::Instance().GetManager<ResourceManager>()->GetAudio(playerDamagePath[rand]).Play();
#ifdef PLATFORM_ANDROID
        Haptics::VibrateFromCpp(500, HapticStrength::Weak);
#endif
}

void PlayerControllerScript::OnDeath()
{
	
	playerAnimator->currAnim = PlayerDeath;
	playerAnimator->currFrame = 0;
	//CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\Player\\PlayerDeath.wav").Play();
	// Dialogue Event
	Registry* reg = CEO::Get<Registry>();

	// Event to call Dialogue
	for (Registry::Entity ent : reg->GetEntitiesWithComponent<DialogueScript>()) {
		reg->GetComponent<DialogueScript>(ent)->Trigger("DEATH");
	}
	TrialPath.GetComponent<ParticleEmitterComponent>()->enabled = false;
	auto registry{ CEO::Get<Registry>() };
	registry->GetComponent<PhysicsComponent>(entity)->isKinematic = true;
	deathconfirmed = true;
#ifdef PLATFORM_ANDROID
	Input::GetControllers()[0].active = false;
	Input::GetControllers()[1].active = false;
#endif
#ifdef PLATFORM_WINDOWS
	CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "pointer"});
#endif
}

void PlayerControllerScript::OnTriggerExit(const Collider& other)
{
	if (!other.registry)
	{
		return;
	}
	Registry& registry = *other.registry;
	auto entities = registry.GetEntitiesWithComponents<NameComponent, TransformComponent>();
	for (auto candidate : entities)
	{
		NameComponent* name = registry.GetComponent<NameComponent>(candidate);
		if (!name || name->name != "Main_camera")
		{
			continue;
		}

		CameraControllerScript* cam = registry.GetComponent<CameraControllerScript>(candidate);
		if (!cam)
		{
			continue;
		}

		cam->OnPlayerDoorExit(registry, other.GetEntity());
		break;
	}
}
