/**___________________________________________________________________________/
@file       PopupManagerScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       03/03/2026   (DD/MM/YYYY)
@brief      Script that handles the achievement popup animation system.
            Reads from AchievementManager's popup queue, instantiates popup
            prefabs with achievement data, and animates them in/out using
            an elastic easing function. Transitions to the main menu scene
            once all popups have been displayed.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/

#include "PopupManagerScript.h"
#include "../CoreLib/SceneManager.h"
#ifndef PI
#define PI 3.1415926545f
#endif

void PopupManagerScript::OnStart(Registry &)
{
    AchievementManager* am = CEO::Instance().GetManager<AchievementManager>();
    /*if (!am->shown && am->showEndCutscene)
    {
      
        return;
    }*/
    if (am->GetLockedAchievements().empty() || am->popUpQueue.empty())
    {
        SceneManager::QueueSceneAction("MainMenu", SceneManager::CHANGE);
        CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
        return;
    }
	index = 0;
	showingPopup = true;
    timer = 0.f;
    isAnimatingIn = true;
    isWaiting = false;
    isAnimatingOut = false;


	while (!CEO::Instance().GetManager<AchievementManager>()->popUpQueue.empty())
	{
		GameObject ach = popupPrefab.Instantiate();
		Achievement a = CEO::Instance().GetManager<AchievementManager>()->popUpQueue.front();
        ach.GetComponent<UITransformComponent>()->relativePos.x = startX;
		std::vector texts = ach.GetChildrenWithComponent<NameComponent>();

		for (auto t : texts)
		{
			if (t.GetComponent<NameComponent>()->name == "Achievement Unlock")
			{
                t.GetComponent<TextRendererComponent>()->text = a.flavourText;
			}
			else if (t.GetComponent<NameComponent>()->name == "Achievement Name")
			{
                t.GetComponent<TextRendererComponent>()->text = a.achievementName;
			}
		}
		
		popupQueue.push(ach);

        CEO::Instance().GetManager<AchievementManager>()->UnlockAchievement(a);
		
		CEO::Instance().GetManager<AchievementManager>()->popUpQueue.pop();
	}
};


void PopupManagerScript::OnUpdate(Registry &, float dt, bool)
{
	/*if (!showingPopup)return;
	if (popupQueue.size() == 0) {
		showingPopup = false;
		CEO::Instance().GetManager<AchievementManager>()->popUpQueue.empty();
	}*/

    if (!showingPopup || popupQueue.empty())
    {
        showingPopup = false;
        return;
    }

    GameObject& current = popupQueue.front();
    float& x = current.GetComponent<UITransformComponent>()->relativePos.x;

    if (!soundPlayed)
    {
        CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\achievement popup.wav").Play();
        soundPlayed = true;
    }

    if (isAnimatingIn)
    {
        
        timer += dt;
        float t = std::clamp(timer / slideDuration, 0.f, 1.f);
        x = startX + (0.f - startX) * EaseInOutElastic(t);

        if (t >= 1.f)
        {
            x = 0.f;
            isAnimatingIn = false;
            isWaiting = true;
            timer = 0.f;
        }
    }
    else if (isWaiting)
    {
        timer += dt;
        if (timer >= waitDuration)
        {
            if (!popupQueue.empty()) soundPlayed = false;
            isWaiting = false;
            isAnimatingOut = true;
            timer = 0.f;
        }
    }
    else if (isAnimatingOut)
    {
        timer += dt;
        float t = std::clamp(timer / slideDuration, 0.f, 1.f);
        x = 0.f + (endX - 0.f) * EaseInOutElastic(t);

        // slide next one in simultaneously
        if (popupQueue.size() > 1)
        {
            std::queue<GameObject> nextqueue = popupQueue;
            nextqueue.pop();
            GameObject next = nextqueue.front();
            float& nextX = next.GetComponent<UITransformComponent>()->relativePos.x;
            nextX = startX + (0.f - startX) * EaseInOutElastic(t);
        }

        if (t >= 1.f)
        {
            popupQueue.pop();
            isAnimatingOut = false;
            timer = 0.f;
            if (!popupQueue.empty())
            {
                isWaiting = true;
            }
            else
            {
                showingPopup = false;
                CEO::Instance().GetManager<AchievementManager>()->popUpQueue = {};
                SceneManager::QueueSceneAction("MainMenu", SceneManager::CHANGE);
                CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Get Me Out Of Hell.wav");
            }
        }
    }
};

void PopupManagerScript::OnFixedUpdate(Registry&, float, bool)
{
};


float PopupManagerScript::EaseInOutElastic(float t) {
    float t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8.0f * t2 * t2 * sin(t * PI * 9.0f);
    }
    else if (t < 0.55f) {
        return 0.5f + 0.75f * sin(t * PI * 4.0f);
    }
    else {
        t2 = (t - 1.0f) * (t - 1.0f);
        return 1.0f - 8.0f * t2 * t2 * sin(t * PI * 9.0f);
    }
}