/**___________________________________________________________________________/
@file       StampManagerScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       05/03/2026   (DD/MM/YYYY)
@brief      Script that handles the achievement stamp display on the Main Menu
			Scene. Maps each stamp GameObject to an Achievement, updates
			their sprites based on unlock status, and manages a hover tooltip
			that displays achievement details when a stamp is hovered over.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "StampManagerScript.h"
#include "../CoreLib/Camera.h"
void StampManagerScript::OnStart(Registry &)
{
	Init();
};


void StampManagerScript::OnUpdate(Registry &, float, bool)
{
		auto& input{ *Input::GetInstance() };
#ifdef EditorFlag
		Vec2 screenPos{ CameraManager::EditorPosToViewportPos(Vec2{ input.GetX(), input.GetY() }) };

#else
#ifdef PLATFORM_WINDOWS
		Vec2 screenPos{ CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{input.GetX(), input.GetY()}) };
#else
		Vec2 screenPos = { input.GetX((0)), input.GetY(0) };
#endif
#endif

#ifdef PLATFORM_WINDOWS

    if (!Input::IsGamepadConnected(0))
		{
			screenPos.y += 170.0f;
			hover.GetComponent<UITransformComponent>()->relativePos = screenPos;
		}
		else
		{
#endif
			if (selectedEntity.IsValid())
				hover.GetComponent<UITransformComponent>()->relativePos = GetComponent<StampManagerScript>(*CEO::Get<Registry>())->selectedEntity.GetComponent<UITransformComponent>()->relativePos;
#ifdef PLATFORM_WINDOWS

    }
#endif
};

void StampManagerScript::OnFixedUpdate(Registry&, float, bool)
{
};

void StampManagerScript::UpdateStamps()
{
	std::vector<Achievement> unlockedAchievements = CEO::Instance().GetManager<AchievementManager>()->GetUnlockedAchievements();
	std::vector<Achievement> allAchievements = CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements();

	if (CEO::Instance().GetManager<AchievementManager>()->shown)
	{
		for (auto it = stampAchievements.begin(); it != stampAchievements.end(); it++)
		{
			it->first->GetComponent<ActiveComponent>()->isActiveSelf = false;
		}
		lastStamp.GetComponent<ActiveComponent>()->isActiveSelf = true;
		return;
	}
	else
	{
		lastStamp.GetComponent<ActiveComponent>()->isActiveSelf = false;
		for (auto it = stampAchievements.begin(); it != stampAchievements.end(); it++)
		{
			it->first->GetComponent<ActiveComponent>()->isActiveSelf = true;
		}
	}
	for (auto& [obj, ach] : stampAchievements)
	{
		ButtonComponent* button = obj->GetComponent<ButtonComponent>();
		button->onHoverEnter = [this, ach,obj]() {ShowHover(ach);
		GetComponent<StampManagerScript>(*CEO::Get<Registry>())->selectedEntity = obj->GetEntityID(); 
			};
		button->onHoverExit = [this]() {HideHover(); };

#ifdef PLATFORM_ANDROID
		button->onHeld = [this, ach]() {ShowHover(ach); };
#endif // PLATFORM_ANDROID

		auto it = std::find_if(allAchievements.begin(), allAchievements.end(),
			[&](const Achievement& a) {
				return a.achievementName == ach.achievementName;
			});

		bool unlocked = std::find_if(unlockedAchievements.begin(), unlockedAchievements.end(),
			[&](const Achievement& unlockedAch) {
				return unlockedAch.achievementName == ach.achievementName;
			}) != unlockedAchievements.end();

		int index = static_cast<int>(std::distance(allAchievements.begin(), it));

		if (unlocked && index >=0)
		{
			obj->GetComponent<SpriteRendererComponent>()->texture = 
				&CEO::Instance().GetManager<ResourceManager>()->GetTexture(stampSprite[index]);
		}
		else
		{
			obj->GetComponent<SpriteRendererComponent>()->texture =
				&CEO::Instance().GetManager<ResourceManager>()->GetTexture("ui\\menus\\stampunfilled.png");
		}
	}
};

void StampManagerScript::Init()
{
	stampAchievements.emplace(&stamp0, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[0]);
	stampAchievements.emplace(&stamp1, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[1]);
	stampAchievements.emplace(&stamp2, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[2]);
	stampAchievements.emplace(&stamp3, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[3]);
	stampAchievements.emplace(&stamp4, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[4]);
	stampAchievements.emplace(&stamp5, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[5]);
	stampAchievements.emplace(&stamp6, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[6]);
	stampAchievements.emplace(&stamp7, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[7]);
	stampAchievements.emplace(&stamp8, CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements()[8]);

	hover = hoverPrefab.Instantiate();
	hover.GetComponent<ActiveComponent>()->isActiveSelf = false;

	UpdateStamps();

	
}

void StampManagerScript::ShowHover(Achievement ach)
{
	std::vector<Achievement> unlockedAchievements = CEO::Instance().GetManager<AchievementManager>()->GetUnlockedAchievements();
	std::vector<Achievement> allAchievements = CEO::Instance().GetManager<AchievementManager>()->GetAllAchievements();
	hover.GetComponent<ActiveComponent>()->isActiveSelf = true;
	std::vector texts = hover.GetChildrenWithComponent<TextRendererComponent>();

	for (auto it : texts)
	{
		std::string name = it.GetComponent<NameComponent>()->name;
		TextRendererComponent* text = it.GetComponent<TextRendererComponent>();

		bool unlocked = std::find_if(unlockedAchievements.begin(), unlockedAchievements.end(),
			[&](const Achievement& unlockedAch) {
				return unlockedAch.achievementName == ach.achievementName;
			}) != unlockedAchievements.end();

		if (name == "Achievement Name")
		{
			text->text = ach.achievementName;
		}

		if (name == "Achievement Condition")
		{
			if (unlocked)
			{
				text->text = ach.flavourText;
			}
			else
			{
				text->text = ach.unlockText;
			}
		}
	}
}

void StampManagerScript::HideHover()
{
	hover.GetComponent<ActiveComponent>()->isActiveSelf = false;
}
