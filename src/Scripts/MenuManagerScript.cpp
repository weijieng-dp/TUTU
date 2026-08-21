/*!
@file       MenuManagerScript.cpp
@author     De Guzman Adrian Lorenzo Yongoyong (d.lorenzoyongoyong) 100%
@date       12/01/2026
@brief		Implements the MenuManager script, which handles Main Menu
			UI and Button Interactions as well as transitioning to various
			scenes.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "MenuManagerScript.h"
#include "../CoreLib/InputManager.h"
#include <functional>
#include "../CoreLib/ResourceManager.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/Application.h"
#include "../CoreLib/StatsManager.h"
#include "../CoreLib/ItemManager.h"
#include "../CoreLib/AchievementManager.h"
#include "../CoreLib/PersistentDataManager.h"
#include "GameStateManagerScript.h"
#include <cmath>

void MenuManagerScript::OnStart(Registry&)
{
	
	//CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
	isTransitioning = false;
	fadeTimer = 0;
	currentState = Home;
	AssignAllButtons();
}

void MenuManagerScript::OnUpdate(Registry&, float dt, bool)
{
	FadeToBlack(dt);
	if (!bookAnimator.GetComponent<AnimatorComponent>()->isPlaying && isTransitioning)
	{
		
		// Switch Panels
		homePanel.SetActive(currentState == Home);
		settingPanel.SetActive(currentState == Settings);
		helpPanel.SetActive(currentState == Help);
		if (currentState == Help)
		{
#ifdef PLATFORM_WINDOWS
			windowsPanel.SetActive(true);
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_ANDROID
			androidPanel.SetActive(true);
#endif // PLATFORM_ANDROID
		}
		else
		{
			windowsPanel.SetActive(false);
			androidPanel.SetActive(false);
		}
		bookAnimator.SetActive(false);
		isTransitioning = false;

		

		AssignAllButtons();
	}
}

void MenuManagerScript::OnFixedUpdate(Registry&, float, bool)
{

}

void MenuManagerScript::AssignAllButtons()
{
	//Put all buttons in a vec
	buttonList.push_back(playButton);
	buttonList.push_back(restartButton);
	buttonList.push_back(exitButton);
	buttonList.push_back(homeButton);
	buttonList.push_back(settingsButton);
	buttonList.push_back(helpButton);
	//Assigns all buttons in the vec
	for (GameObject button : buttonList)
	{
		std::string buttonName = button.GetComponent<NameComponent>()->name;
		AssignButton(button, buttonName);
	}
}

void MenuManagerScript::AssignButton(GameObject button, std::string buttonName)
{
	ButtonComponent* buttonComp = button.GetComponent<ButtonComponent>();
	std::string pressedSprite = "";
	std::string hoverSprite = "";
	std::string defaultSprite = "";

	if (buttonName == "MainMenu_Play_Button" || buttonName == "MainMenu_Restart_Button" || buttonName == "MainMenu_Quit_Button")
	{
		pressedSprite = "ui\\menus\\buttonpressed.png";
		hoverSprite = "ui\\menus\\buttonhover_pink.png";
		defaultSprite = "ui\\menus\\buttonidle.png";
	}

	if (buttonName == "Home_Button" && currentState != Home)
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\homeidle.png");
		pressedSprite = "ui\\menus\\homepressed.png";
		hoverSprite = "ui\\menus\\homehover.png";
		defaultSprite = "ui\\menus\\homeidle.png";
	}

	if (buttonName == "Settings_Button" && currentState != Settings)
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\settingsidle.png");
		pressedSprite = "ui\\menus\\settingspressed.png";
		hoverSprite = "ui\\menus\\settingshover.png";
		defaultSprite = "ui\\menus\\settingsidle.png";
	}

	if (buttonName == "Help_Button" && currentState != Help)
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\howtoplayidle.png");
		pressedSprite = "ui\\menus\\howtoplaypressed.png";
		hoverSprite = "ui\\menus\\howtoplayhover.png";
		defaultSprite = "ui\\menus\\howtoplayidle.png";
	}

	if (buttonName == "Home_Button" && currentState == Home) 
	{
		button.GetComponent<SpriteRendererComponent>()->texture = 
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\homepressed.png");
		buttonComp->onClick = nullptr;
		buttonComp->onHoverEnter = nullptr;
		buttonComp->onHoverExit = nullptr;
		return;
	}

	if (buttonName == "Settings_Button" && currentState == Settings)
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\settingspressed.png");
		buttonComp->onClick = nullptr;
		buttonComp->onHoverEnter = nullptr;
		buttonComp->onHoverExit = nullptr;
		return;
	}

	if (buttonName == "Help_Button" && currentState == Help)
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\howtoplaypressed.png");
		buttonComp->onClick = nullptr;
		buttonComp->onHoverEnter = nullptr;
		buttonComp->onHoverExit = nullptr;
		return;
	}
		
	// Default values
	buttonComp->onClick = std::bind(&MenuManagerScript::ButtonClicked, GetComponent<MenuManagerScript>(*CEO::Get<Registry>()), pressedSprite, button);
	buttonComp->onHoverEnter = std::bind(&MenuManagerScript::ButtonHoverEnter, GetComponent<MenuManagerScript>(*CEO::Get<Registry>()), hoverSprite, button);
	buttonComp->onHoverExit = std::bind(&MenuManagerScript::ButtonHoverExit, GetComponent<MenuManagerScript>(*CEO::Get<Registry>()), defaultSprite, button);
}

void MenuManagerScript::ButtonClicked(std::string pressedSprite, GameObject button)
{
	AssignButtonTexture(pressedSprite, button.GetComponent<SpriteRendererComponent>(), false);
	std::string buttonName = button.GetComponent<NameComponent>()->name;
	if (buttonName == "Home_Button" && currentState != Home)
	{
		LOGI("Home Button Clicked");
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\BookFlip.wav").Play();
		isTransitioning = true;
		currentState = Home;
		button.GetComponent<ButtonComponent>()->onClick = nullptr;
	}

	if (buttonName == "Settings_Button" && currentState != Settings)
	{
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\BookFlip.wav").Play();
		LOGI("Settings Button Clicked");
		isTransitioning = true;
		currentState = Settings;
		button.GetComponent<ButtonComponent>()->onClick = nullptr;
	}

	if (buttonName == "Help_Button" && currentState != Help)
	{
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\BookFlip.wav").Play();
		LOGI("Help Button Clicked");
		isTransitioning = true;
		currentState = Help;
		button.GetComponent<ButtonComponent>()->onClick = nullptr;
	}

	if (buttonName == "MainMenu_Play_Button")
	{
		LOGI("Play Button Clicked");

		isFading = true;	
		isTransitioning = false;
		auto& pm{ *CEO::Get<PersistentDataManager>() };
		CEO::Instance().Get<MapManager>()->ClearMapData();
		pm.Set("PlayerPosition", Vec2(0.f, 0.f));
		pm.Set<bool>("DoorsClosed", false);
		CEO::Instance().GetManager<AchievementManager>()->SaveTracker();
		pm.Set("IsWin", false);
		pm.Set("IsWaveMode", false);
		pm.Set<int>("WaveEnemiesSpawned", 0);
		CEO::Get<MapManager>()->ResetWave();
		if (CEO::Instance().GetManager<AchievementManager>()->firstPlay != true)
		{
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\It_s Scary Here.wav", 0.5f, 0.5f);
		}
		else
		{
			CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\It_s Scary Here.wav", 0.0f, 0.0f);
		}
		CEO::Get<PersistentDataManager>()->Set("MinimapCamPos", Vec2(0.f, 0.f));

		CEO::Get<StatsManager>()->ResetPlayer();
		CEO::Get<ItemManager>()->ResetItems();
	}
	if (buttonName == "MainMenu_Restart_Button")
	{
		isTransitioning = false;
		SceneManager::QueueSceneAction("Confirm_Restart", SceneManager::PUSH);
	}
	if (buttonName == "MainMenu_Quit_Button")
	{
		isTransitioning = false;
		SceneManager::QueueSceneAction("Confirm_Quit", SceneManager::PUSH);
	}
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
	//If transitioning within scene, play the book animation
	if (!isTransitioning) return;
	LOGI("%d / %d", isTransitioning, currentState);
	bookAnimator.SetActive(true);
	bookAnimator.GetComponent<AnimatorComponent>()->isPlaying = true;
	isTransitioning = true;
}

void MenuManagerScript::ButtonHoverEnter(std::string hoverSprite, GameObject button)
{
	AssignButtonTexture(hoverSprite, button.GetComponent<SpriteRendererComponent>(), true);
}

void MenuManagerScript::ButtonHoverExit(std::string defaultSprite, GameObject button)
{
	AssignButtonTexture(defaultSprite, button.GetComponent<SpriteRendererComponent>(), false);
}

void MenuManagerScript::AssignButtonTexture(std::string texturePath, SpriteRendererComponent* buttonSprite, bool hover)
{
	buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(texturePath);
	//Play button hover sound when hover enter
	if (!hover) return;
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
}

void MenuManagerScript::FadeToBlack(float dt)
{
	if (!isFading)
	{
		fadeTimer = 0.0f;
		return;
	}
	fadeTimer += dt;
	if (fadeTimer <= fadeDuration)
	{
		float progress = fadeTimer / fadeDuration;
		fadePanel.GetComponent<SpriteRendererComponent>()->color.a = progress;
		/*GameObject a(fadePanel);
		for (GameObject haha : a.GetChildrenWithComponent<ButtonComponent>())
		{
			haha.SetActive(true);
		}*/

		Registry& registry{ *CEO::Get<Registry>() };
		auto f = registry.GetEntitiesWithComponent<ButtonComponent>();
		for (auto fb : f)
		{
			std::string name = registry.GetComponent<NameComponent>(fb)->name;
			if (name == "FakeButton")
			{
				registry.GetComponent<ActiveComponent>(fb)->isActiveSelf = true;
			}
		}
	}
	else
	{
		fadeTimer = 0;
		isFading = false;
		if (CEO::Instance().GetManager<AchievementManager>()->firstPlay == true)
		{
			SceneManager::QueueSceneAction("video test", SceneManager::CHANGE);
		}
		else
		{
			SceneManager::QueueSceneAction("Map", SceneManager::CHANGE);
		}
	}
}
