/**___________________________________________________________________________/
@file           BackToMapScript.cpp
@author         Tan Jun Jie (t.junjie) (100%)
@date           03/02/2026 (DD/MM/YYYY)
@brief          Handles buttons in Game.scene, mainly back tot map button that
				brings player back to map scene upon clearing of a room.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "BackToMapScript.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/PersistentDataManager.h"
#include "FadeScript.h"

void BackToMapScript::OnStart(Registry & r) {

	for (auto btnEnt : r.GetEntitiesWithComponent<ButtonComponent>()) {
		if (r.GetComponent<NameComponent>(btnEnt)->name == "BackToMap") {
			backToMapBtn = GameObject(btnEnt);
		}
		else if (r.GetComponent<NameComponent>(btnEnt)->name == "PauseBtn") {
			pauseBtn = GameObject(btnEnt);
		}
	}

	if (backToMapBtn.IsValid()) {	// if backToMapBtn is valid, set up the button functions
		ButtonComponent& bc{ *backToMapBtn.GetComponent<ButtonComponent>() };
		bc.onClick = [&]() { 
			auto registry{ CEO::Get<Registry>() };
			for (auto ent : registry->GetEntitiesWithComponent<NameComponent>()) {
				const auto& name{ registry->GetComponent<NameComponent>(ent)->name };
				if (name == "Player") {
					CEO::Get<PersistentDataManager>()->Set("PlayerPosition", registry->GetComponent<TransformComponent>(ent)->translate);
				}
				else if (name == "Minimap_Camera")
					CEO::Get<PersistentDataManager>()->Set("MinimapCamPos", registry->GetComponent<TransformComponent>(ent)->translate);
			}
			
			//CEO::Get<SceneManager>()->QueueSceneAction("Map", SceneManager::SCENE_ACTION::CHANGE); 
			
			auto check{ CEO::Get<PersistentDataManager>()->Get<bool>("RoomCleared") };
			if (check && *check) {
				fadeBG.GetComponent<FadeScript>()->FadeToBlack();
				CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
			}
		};
		bc.onHoverEnter = [sprite = backToMapBtn.GetComponent<SpriteRendererComponent>()]() 
			{ 
				//sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("items/book_hover.png"); 
				CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
			};
		bc.onHoverExit = [sprite = backToMapBtn.GetComponent<SpriteRendererComponent>()]() {
			//sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("items/book_idle.png"); 
			};

		auto& mapManager{ *CEO::Get<MapManager>() };
		auto waveMode{ CEO::Get<PersistentDataManager>()->Get<bool>("IsWaveMode") };
		bool waveCheck{ !waveMode || !*waveMode };
		if (mapManager.GetTileInstanceSize() && mapManager.GetTileInstance(static_cast<int>(mapManager.GetTileInstanceSize()) - 1)->cleared && waveCheck)
			backToMapBtn.GetComponent<SpriteRendererComponent>()->color = { 1.f, 1.f, 1.f, 1.f };
		else 
			backToMapBtn.GetComponent<SpriteRendererComponent>()->color = { 0.5f, 0.5f, 0.5f, 0.5f };


	}
	if (pauseBtn.IsValid()) {	// if pauseBtn is valid, set up the pause button's button function
		ButtonComponent& bc{ *pauseBtn.GetComponent<ButtonComponent>() };
		bc.onClick = []() { 
			CEO::Get<SceneManager>()->QueueSceneAction("PauseMenu", SceneManager::PUSH); 
			CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
#ifdef PLATFORM_ANDROID
			Input::GetControllers()[0].active = false;
		Input::GetControllers()[1].active = false;
#endif
#ifdef PLATFORM_WINDOWS
			CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "pointer" });
#endif
			};
		bc.onHoverEnter = [sprite = pauseBtn.GetComponent<SpriteRendererComponent>()]() 
			{ 
				sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("ui/game/pause_hover.png");
				CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
			};
		bc.onHoverExit = [sprite = pauseBtn.GetComponent<SpriteRendererComponent>()]() {sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("ui/game/pause.png"); };
	}
};


void BackToMapScript::OnUpdate(Registry &, float dt, bool ) 
{
	auto check{ CEO::Get<PersistentDataManager>()->Get<bool>("RoomCleared") };
	if (check && *check) {
		auto sprite = backToMapBtn.GetComponent<SpriteRendererComponent>();
		timer += dt;
		if (timer >= 0.5f) timer = 0.0f;

		if (timer < 0.25f)
		{
			sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("items/book_hover.png");
		}
		else
		{
			sprite->texture = &CEO::Get<ResourceManager>()->GetTexture("items/book_idle.png");
		}
	}
#ifdef PLATFORM_WINDOWS
	if (Input::IsGamepadConnected(0))
	{
		if (Input::IsGamepadButtonPressed(Input::GamepadButton::A))
		{
			if (backToMapBtn.IsValid()) {
				auto registry{ CEO::Get<Registry>() };
				for (auto ent : registry->GetEntitiesWithComponent<NameComponent>()) {
					const auto& name{ registry->GetComponent<NameComponent>(ent)->name };
					if (name == "Player") {
						CEO::Get<PersistentDataManager>()->Set("PlayerPosition", registry->GetComponent<TransformComponent>(ent)->translate);
					}
					else if (name == "Minimap_Camera")
						CEO::Get<PersistentDataManager>()->Set("MinimapCamPos", registry->GetComponent<TransformComponent>(ent)->translate);
				}

				//CEO::Get<SceneManager>()->QueueSceneAction("Map", SceneManager::SCENE_ACTION::CHANGE); 

				
				if (check && *check) {
					fadeBG.GetComponent<FadeScript>()->FadeToBlack();
					CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
				}
			}


		}

		if (pauseBtn.IsValid())
		{
			if (Input::IsGamepadButtonPressed(Input::GamepadButton::B)) {
				CEO::Get<SceneManager>()->QueueSceneAction("PauseMenu", SceneManager::PUSH);
				CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();

				CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "pointer" });
			};
		}
	}
#endif
};

void BackToMapScript::OnFixedUpdate(Registry&, float, bool)
{
};
