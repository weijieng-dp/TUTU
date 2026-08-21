/*!
@file       PauseManagerScript.cpp
@author     De Guzman Adrian Lorenzo Yongoyong (d.lorenzoyongoyong) 100%
@date       15/01/2026
@brief		Implements the PauseManager script, which handles Pause Menu
			UI and Button Interactions as well as transitioning to various
			scenes.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#include "PauseManagerScript.h"
#include "../CoreLib/SceneManager.h"
void PauseManagerScript::OnStart(Registry & )
{
	isTransitioning = false;
	currentState = Settings;
	
	AssignAllButtons();
};


void PauseManagerScript::OnUpdate(Registry & , float , bool )
{
#ifdef PLATFORM_WINDOWS
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) SceneManager::QueueSceneAction("", SceneManager::POP);
#endif
	if (!bookTransition.GetComponent<AnimatorComponent>()->isPlaying && isTransitioning)
	{
		settingsPanel.SetActive(currentState == Settings);
		faqPanel.SetActive(currentState == FAQ);

		if (currentState == FAQ)
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
		bookTransition.SetActive(false);
		isTransitioning = false;
		AssignAllButtons();
	}
};

void PauseManagerScript::OnFixedUpdate(Registry&, float, bool)
{
};

void PauseManagerScript::AssignAllButtons()
{
	buttonList.push_back(homeButton);
	buttonList.push_back(settingsButton);
	buttonList.push_back(faqButton);
	buttonList.push_back(resumeButton);

	for (GameObject button : buttonList)
	{
		std::string buttonName = button.GetComponent<NameComponent>()->name;
		AssignButton(button, buttonName);
	}
}

void PauseManagerScript::AssignButton(GameObject button, std::string buttonName)
{
	ButtonComponent* buttonComp = button.GetComponent<ButtonComponent>();
	std::string defaultSprite = "";
	std::string hoverSprite = "";
	std::string pressedSprite = "";

	if (buttonName == "Settings_ResumeGame_Button")
	{
		defaultSprite = "ui\\menus\\buttonidle.png";
		hoverSprite = "ui\\menus\\buttonhover_pink.png";
		pressedSprite = "ui\\menus\\buttonpressed.png";
	}

	if (buttonName == "FAQ_Button")
	{
		if (currentState == FAQ)
		{
			button.GetComponent<SpriteRendererComponent>()->texture =
				&CEO::Instance().GetManager<ResourceManager>()->
				GetTexture("ui\\menus\\howtoplaypressed.png");
			buttonComp->onClick = nullptr;
			buttonComp->onHoverEnter = nullptr;
			buttonComp->onHoverExit = nullptr;
			return;
		}
		else
		{
			button.GetComponent<SpriteRendererComponent>()->texture =
				&CEO::Instance().GetManager<ResourceManager>()->
				GetTexture("ui\\menus\\howtoplayidle.png");
			pressedSprite = "ui\\menus\\howtoplaypressed.png";
			hoverSprite = "ui\\menus\\howtoplayhover.png";
			defaultSprite = "ui\\menus\\howtoplayidle.png";
		}
	}

	if (buttonName == "Settings_Button")
	{
		if (currentState == Settings)
		{
			button.GetComponent<SpriteRendererComponent>()->texture =
				&CEO::Instance().GetManager<ResourceManager>()->
				GetTexture("ui\\menus\\settingspressed.png");
			buttonComp->onClick = nullptr;
			buttonComp->onHoverEnter = nullptr;
			buttonComp->onHoverExit = nullptr;
			return;
		}
		else
		{
			button.GetComponent<SpriteRendererComponent>()->texture =
				&CEO::Instance().GetManager<ResourceManager>()->
				GetTexture("ui\\menus\\settingsidle.png");
			pressedSprite = "ui\\menus\\settingspressed.png";
			hoverSprite = "ui\\menus\\settingshover.png";
			defaultSprite = "ui\\menus\\settingsidle.png";
		}
	}

	if (buttonName == "Home_Button")
	{
		button.GetComponent<SpriteRendererComponent>()->texture =
			&CEO::Instance().GetManager<ResourceManager>()->
			GetTexture("ui\\menus\\homeidle.png");
		pressedSprite = "ui\\menus\\homepressed.png";
		hoverSprite = "ui\\menus\\homehover.png";
		defaultSprite = "ui\\menus\\homeidle.png";
	}

	buttonComp->onClick = std::bind(&PauseManagerScript::ButtonClicked, GetComponent<PauseManagerScript>(*CEO::Get<Registry>()), pressedSprite, button);
	buttonComp->onHoverEnter = std::bind(&PauseManagerScript::ButtonHoverEnter, GetComponent<PauseManagerScript>(*CEO::Get<Registry>()), hoverSprite, button);
	buttonComp->onHoverExit = std::bind(&PauseManagerScript::ButtonHoverExit, GetComponent<PauseManagerScript>(*CEO::Get<Registry>()), defaultSprite, button);
}

void PauseManagerScript::ButtonClicked(std::string pressedSprite, GameObject button)
{
	AssignButtonTexture(pressedSprite, button.GetComponent<SpriteRendererComponent>(), false);
	std::string buttonName = button.GetComponent<NameComponent>()->name;

	
	if (buttonName == "Settings_Button")
	{
		button.GetComponent<ButtonComponent>()->onClick = nullptr;
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\BookFlip.wav").Play();
		isTransitioning = true;
		currentState = Settings;
	}

	if (buttonName == "FAQ_Button")
	{
		button.GetComponent<ButtonComponent>()->onClick = nullptr;
		CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\BookFlip.wav").Play();
		isTransitioning = true;
		currentState = FAQ;
	}

	if (buttonName == "Home_Button")
	{
		isTransitioning = false;
		SceneManager::QueueSceneAction("Confirm", SceneManager::PUSH);
	}

	if (buttonName == "Settings_ResumeGame_Button")
	{
#ifdef PLATFORM_ANDROID
		if (CEO::Get<SceneManager>()->BaseScene() == "Game" || CEO::Get<SceneManager>()->BaseScene() == "FinalWave")
		{
			Input::GetControllers()[0].active = true;
			Input::GetControllers()[1].active = true;
		}
#endif
#ifdef PLATFORM_WINDOWS
		CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "crosshair" });
#endif
		isTransitioning = false;
		SceneManager::QueueSceneAction("", SceneManager::POP);
	}
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();

	if (!isTransitioning) return;
	bookTransition.SetActive(true);
	bookTransition.GetComponent<AnimatorComponent>()->isPlaying = true;
	
}

void PauseManagerScript::ButtonHoverEnter(std::string hoverSprite, GameObject button)
{
	AssignButtonTexture(hoverSprite, button.GetComponent<SpriteRendererComponent>(), true);
}

void PauseManagerScript::ButtonHoverExit(std::string defaultSprite, GameObject button)
{
	AssignButtonTexture(defaultSprite, button.GetComponent<SpriteRendererComponent>(), false);
}

void PauseManagerScript::AssignButtonTexture(std::string texturePath, SpriteRendererComponent* buttonSprite, bool hover)
{
	buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(texturePath);
	if (!hover) return;
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
}