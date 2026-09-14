/**___________________________________________________________________________/
@file       FadeScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       24/02/2026   (DD/MM/YYYY)
@brief      Script that handles screen fade transitions between scenes.
			Supports fading to and from black, optional BGM crossfading,
			and video playback integration where the fade out is triggered
			once the video has finished playing.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "FadeScript.h"
#include "GameStateManagerScript.h"
#include "../CoreLib/MapManager.h"

void FadeScript::OnStart(Registry& r)
{
	fakeButton = CEO::Instance().GetManager<ResourceManager>()->InstantiatePrefab(r, "FakeButton");
	fadeToBlack = false;
	fade = true;
	if (videoPlayer.IsValid()) videoFound = true;
	fadeTimer = fadeDuration;
};


void FadeScript::OnUpdate(Registry& registry, float dt, bool)
{
	if (videoFound && videoPlayer.GetComponent<VideoComponent>()->isEnded)
	{
		if (!isFading) {
			isFading = true;
			FadeToBlack();
		}
		// If it stopped before initial fade is finished
		else if (!fadeToBlack && fade) {
			queueFadeAction = true;
		}

		// Trigger the fade out after the fade in if said action is queued
		if (queueFadeAction && fadeToBlack && !fade) {
			isFading = true;
			fade = true;
			FadeToBlack();
		}
	}
	ToggleFade(fadeToBlack, dt, registry);
};

void FadeScript::OnFixedUpdate(Registry&, float, bool)
{
};

void FadeScript::ToggleFade(bool toBlack, float dt, Registry& registry) {
	if (toBlack) {
		if (!fade) {
			fadeTimer = 0.0f;
			return;
		}

		fadeTimer += dt;
		if (fadeTimer <= fadeDuration)
		{
			float progress = fadeTimer / fadeDuration;
			GetComponent<SpriteRendererComponent>(registry)->color.a = progress;
		
			/*GameObject a(entity);
			for (GameObject haha : a.GetChildrenWithComponent<ButtonComponent>())
			{
				haha.SetActive(true);
			}*/

			registry.GetComponent<ActiveComponent>(fakeButton)->isActiveSelf = true;

		}
		else
		{

			fadeTimer = 0;
			fade = false;
			if (sceneToTransition != "")
			{
				SceneManager::QueueSceneAction(sceneToTransition, SceneManager::CHANGE);
			}
		}
	}
	else {
		if (!fade) {
			fadeTimer = fadeDuration;
			return;
		}

		fadeTimer -= dt;
		if (fadeTimer >= 0)
		{
			float progress = fadeTimer / fadeDuration;

			GetComponent<SpriteRendererComponent>(registry)->color.a = progress;
		}
		else
		{
			GetComponent<SpriteRendererComponent>(registry)->color.a = 0.0f;
			fadeTimer = 0;
			fade = false;
			fadeToBlack = true;
			/*GameObject a(entity);
			for (GameObject haha : a.GetChildrenWithComponent<ButtonComponent>())
			{
				haha.SetActive(false);
			}*/
			registry.GetComponent<ActiveComponent>(fakeButton)->isActiveSelf = false;
			if (CEO::Get<MapManager>()->losecon)
			{
				SceneManager::QueueSceneAction("Lose", SceneManager::PUSH);
				CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\Am I Stuck Forever.wav");
				CEO::Get<MapManager>()->losecon = false;
			}
		}
	}
}

void FadeScript::FadeToBlack()
{
	Registry& registry{ *CEO::Get<Registry>() };
	registry.GetComponent<ActiveComponent>(fakeButton)->isActiveSelf = true;
	if (musicToTransition != "")
	{
		CEO::Instance().GetManager<ResourceManager>()->QueueBGM("BGM\\" + musicToTransition, fadeDuration, fadeDuration);
	}
	fade = true;
};
