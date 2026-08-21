/*!
@file       ConfirmSceneScript.cpp
@author     De Guzman Adrian Lorenzo Yongoyong (d.lorenzoyongoyong) 100%
@date       20/01/2026
@brief		Implements the ConfirmScene script, which handles the
			various Confirm scenes and popping out when finished

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "ConfirmSceneScript.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/Application.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/StatsManager.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/AchievementManager.h"
#include "StampManagerScript.h"
#include "../CoreLib/PersistentDataManager.h"

void ConfirmSceneScript::OnStart(Registry &)
{
	std::string pressedSprite = "ui\\menus\\buttonpressed.png";
	std::string hoverSprite = "ui\\menus\\buttonhover_pink.png";
	std::string defaultSprite = "ui\\menus\\buttonidle.png";
	ButtonComponent* yesBComp = yesButton.GetComponent<ButtonComponent>();
	if (yesBComp)
	{
		yesBComp->onClick = std::bind(&ConfirmSceneScript::ButtonClicked, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), pressedSprite, yesButton);
		yesBComp->onHoverEnter = std::bind(&ConfirmSceneScript::ButtonHoverEnter, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), hoverSprite, yesButton);
		yesBComp->onHoverExit = std::bind(&ConfirmSceneScript::ButtonHoverExit, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), defaultSprite, yesButton);
	}
	ButtonComponent* noBComp = noButton.GetComponent<ButtonComponent>();
	if (noBComp)
	{
		noBComp->onClick = std::bind(&ConfirmSceneScript::ButtonClicked, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), pressedSprite, noButton);
		noBComp->onHoverEnter = std::bind(&ConfirmSceneScript::ButtonHoverEnter, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), hoverSprite, noButton);
		noBComp->onHoverExit = std::bind(&ConfirmSceneScript::ButtonHoverExit, GetComponent<ConfirmSceneScript>(*CEO::Get<Registry>()), defaultSprite, noButton);
	}
};


void ConfirmSceneScript::OnUpdate(Registry & , float , bool )
{

    if (CEO::Get<SceneManager>()->SceneStack().top() == "Lose") return;
#ifdef PLATFORM_WINDOWS
    if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) SceneManager::QueueSceneAction("", SceneManager::POP);
#endif
};

void ConfirmSceneScript::OnFixedUpdate(Registry&, float, bool)
{
};

void ConfirmSceneScript::ButtonClicked(std::string pressedSprite, GameObject button)
{
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
	AssignButtonTexture(pressedSprite, button.GetComponent<SpriteRendererComponent>(), false);
	std::string buttonName = button.GetComponent<NameComponent>()->name;

	if (buttonName == "Yes_Menu")
	{
#ifdef PLATFORM_ANDROID
		Input::GetControllers()[0].active = false;
		Input::GetControllers()[1].active = false;
#endif
		CEO::Get<StatsManager>()->ResetPlayer();
		CEO::Get<ItemManager>()->ResetItems();

		//To add going back to map scene as a new game
		CEO::Get<PersistentDataManager>()->Set("PlayerPosition", Vec2(0.f, 0.f));
		CEO::Get<MapManager>()->ClearMapData();
#ifdef PLATFORM_WINDOWS
		CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{"pointer"});
#endif
		SceneManager::QueueSceneAction("MainMenu", SceneManager::CHANGE);
		CEO::Get<ResourceManager>()->StopAllAudio();
		CEO::Instance().GetManager<AchievementManager>()->RollbackTracker();
		//CEO::Instance().GetManager<ResourceManager>()->GetAudio("BGM\\Get Me Out Of Hell.wav", "BGM", true, true).Play();
		CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
	}

	if (buttonName == "Achievement")
	{
		CEO::Get<StatsManager>()->ResetPlayer();
		CEO::Get<ItemManager>()->ResetItems();

		//To add going back to map scene as a new game
		CEO::Get<PersistentDataManager>()->Set("PlayerPosition", Vec2(0.f, 0.f));
		CEO::Get<MapManager>()->ClearMapData();
#ifdef PLATFORM_ANDROID
		Input::GetControllers()[0].active = false;
		Input::GetControllers()[1].active = false;
#endif
		auto& am{ *CEO::Get<AchievementManager>() };
		if (am.popUpQueue.size() > 0)
		{
			SceneManager::QueueSceneAction("Achievement", SceneManager::CHANGE);
		}
		else
		{
			SceneManager::QueueSceneAction("MainMenu", SceneManager::CHANGE);
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
		}
	}
	if (buttonName == "Yes_Quit")
	{
		Application::SignalExit();
	}
	if (buttonName == "No")
	{
		SceneManager::QueueSceneAction("", SceneManager::POP);
	}
	if (buttonName == "Reset")
	{
		CEO::Get<StatsManager>()->ResetPlayer();
		CEO::Get<ItemManager>()->ResetItems();

		//To add going back to map scene as a new game
		CEO::Get<PersistentDataManager>()->Set("PlayerPosition", Vec2(0.f, 0.f));
		CEO::Get<MapManager>()->ClearMapData();
		SceneManager::QueueSceneAction("Map", SceneManager::CHANGE);
		CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\It_s Scary Here.wav");
	}
	if (buttonName == "Restart")
	{
		CEO::Instance().GetManager<AchievementManager>()->ResetData();
		Registry& registry{ *CEO::Instance().GetManager<Registry>() };
		for (auto ent : registry.GetEntitiesWithComponent<StampManagerScript>()) {
			auto& sComp{ registry.TryGetComponent<StampManagerScript>(ent) };
			sComp.UpdateStamps();
		}
		//GetComponent<StampManagerScript>(registry)->UpdateStamps();
		SceneManager::QueueSceneAction("", SceneManager::POP);
	}
};
void ConfirmSceneScript::ButtonHoverEnter(std::string hoverSprite, GameObject button)
{
	AssignButtonTexture(hoverSprite, button.GetComponent<SpriteRendererComponent>(), true);
};
void ConfirmSceneScript::ButtonHoverExit(std::string defaultSprite, GameObject button)
{
	AssignButtonTexture(defaultSprite, button.GetComponent<SpriteRendererComponent>(), false);
};
void ConfirmSceneScript::AssignButtonTexture(std::string texturePath, SpriteRendererComponent* buttonSprite, bool hover)
{
	buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(texturePath);
	if (!hover) return;
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
};