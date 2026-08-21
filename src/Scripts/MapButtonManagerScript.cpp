/**___________________________________________________________________________/
@file       MapButtonManagerScript.h
@author     Tan Jun Jie (t.junjie) (50%)
@author     Ng Wei Jie (weijie.ng) (30%)
@author     LORENZO YONGYONG De Guzman Adrian (d.lorenzoyongoyong) (20%)
@date		03/02/2026 (DD/MM/YYYY)
@brief		Handles button behaviour in Map.scene, for player to cycle through
			available tiles for players to place down.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "MapButtonManagerScript.h"
#include "../CoreLib/InputManager.h"
#include <functional>
#include "../CoreLib/ResourceManager.h"
#include "../CoreLib/AchievementManager.h"

#include "../CoreLib/SceneManager.h"
#include "GameStateManagerScript.h"
#include "../CoreLib/MapManager.h"
#include "TileManagerScript.h"
#include "FadeScript.h"

#include "../CoreLib/PersistentDataManager.h"

void MapButtonManagerScript::OnStart(Registry& registry) {
	this->isTransitioning = false;

	for (auto NameEntity : registry.GetEntitiesWithComponents<NameComponent>()) {
		NameComponent& nameComp = registry.TryGetComponent<NameComponent>(NameEntity);
		if (nameComp.name == "TileSelect") {
			this->DisplayTileEntity = NameEntity;
			break;
		}
	}

	std::string hoverTex{}, idleTex{};
	// go through all button entities and set them up
	for (auto ent : registry.GetEntitiesWithComponent<ButtonComponent>()) {	//Assign all button functions to each button on scene
		auto& button{ *registry.GetComponent<ButtonComponent>(ent) };
		auto buttonSprite{ registry.GetComponent<SpriteRendererComponent>(ent) };
		std::string buttonName{ registry.GetComponent<NameComponent>(ent)->name };

		if (buttonName == "Button_Play") {
			hoverTex = idleTex = "ui\\game\\map_play_inactive.png";
			buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(idleTex);
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, false, "Game", false); 
		}
		else if (buttonName == "Tile_Select_L") {
			hoverTex = "ui\\game\\small_arrow_hover.png";
			idleTex = "ui\\game\\small_arrow.png";
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, false, "LTiles");
		}
		else if (buttonName == "Tile_Select_R") {
			hoverTex = "ui\\game\\small_arrow_hover.png";
			idleTex = "ui\\game\\small_arrow.png";
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, false, "RTiles");
		}
		else if (buttonName == "PauseResumeButton") {
			hoverTex = "ui\\game\\map_pause_hover.png";
			idleTex = "ui\\game\\map_pause.png";
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, false, "Resume");
		}
		else if (buttonName == "PauseHomeButton") {
			hoverTex = "ui\\menus\\homehover.png";
			idleTex = "ui\\menus\\homeidle.png";
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, true, "MainMenu_Scene");
		}
		else if (buttonName == "TileSelect") {
			this->AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				"", "", false, "Drag_Tiles");
		}
	}

	auto entities{ registry.GetEntitiesWithComponent<TileManagerScript>() };
	if (!entities.empty()) { tileManagerScriptEntity = entities[0]; }
};
void MapButtonManagerScript::OnUpdate(Registry&, float, bool)
{
#ifdef PLATFORM_WINDOWS
	if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
		CEO::Get<SceneManager>()->QueueSceneAction("PauseMenu", SceneManager::PUSH);
	}
#endif
	if (!this->isTransitioning) return;
	fadeBG.GetComponent<FadeScript>()->FadeToBlack();
		CEO::Instance().GetManager<AchievementManager>()->UpdateTracker("TILES");
	this->isTransitioning = false;
};
void MapButtonManagerScript::OnFixedUpdate(Registry&, float, bool) { /*empty by design*/ }

void MapButtonManagerScript::AssignButton(Registry& registry, ButtonComponent& button, SpriteRendererComponent* buttonSprite,
	std::string hoverTex, std::string idleTex, bool sceneSwitch, std::string sceneToSwitch, bool active) {
	if (active) {
		button.onClick = std::bind(ButtonClicked, std::reference_wrapper<Registry>(registry),
			GetComponent<MapButtonManagerScript>(registry), sceneSwitch, sceneToSwitch);
		button.onPress = std::bind(ButtonPressed, std::reference_wrapper<Registry>(registry),
			GetComponent<MapButtonManagerScript>(registry), sceneSwitch, sceneToSwitch);
	}
	else {
		button.onClick = nullptr;
		button.onPress = nullptr;
	}
	if (!hoverTex.empty()) button.onHoverEnter = std::bind(SetSprite, buttonSprite, hoverTex, true);
	if (!idleTex.empty()) button.onHoverExit = std::bind(SetSprite, buttonSprite, idleTex, false);
};


void MapButtonManagerScript::ButtonPressed(Registry& registry, MapButtonManagerScript* mainMenuButton,
	bool, std::string sceneToSwitch)
{
	if (sceneToSwitch == "Drag_Tiles") {
		if (mainMenuButton->tileManagerScriptEntity) {
			CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\TilePickup.wav", "SFX").Play();
			auto* tileScript{ registry.GetComponent<TileManagerScript>(mainMenuButton->tileManagerScriptEntity) };
			if (tileScript->BeginPlacement(mainMenuButton->TileNumber)) {
				SpriteRendererComponent& displayTile = registry.TryGetComponent<SpriteRendererComponent>(mainMenuButton->DisplayTileEntity);
				displayTile.visible = false;
			}
		}
	}

}

void MapButtonManagerScript::ButtonClicked(Registry& registry, MapButtonManagerScript* mainMenuButton,
	bool sceneSwitch, std::string sceneToSwitch)
{
	CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonPressed.wav", "SFX").Play();
	auto offers{ CEO::Instance().GetManager<MapManager>()->GetTileOffers() };

	//Scene switch
	if (sceneToSwitch == "LTiles" || sceneToSwitch == "RTiles") {
		SpriteRendererComponent& displaytile = registry.TryGetComponent<SpriteRendererComponent>(mainMenuButton->DisplayTileEntity);

		do {
			mainMenuButton->TileNumber += sceneToSwitch == "LTiles" ? -1 : 1;

			// Wrap Around
			if (mainMenuButton->TileNumber < 0) mainMenuButton->TileNumber = static_cast<int>(offers.size()) - 1;
			if (mainMenuButton->TileNumber >= offers.size()) mainMenuButton->TileNumber = 0;
		} while (offers[mainMenuButton->TileNumber].tileData == nullptr);

		displaytile.texture = offers[mainMenuButton->TileNumber].tileData->sprite.first;

		int updated{ 0 };
		auto& mapManager{ *CEO::Instance().GetManager<MapManager>() };
		for (auto ent : registry.GetEntitiesWithComponent<UITransformComponent>()) {
			if (updated == 3) break;
			const auto& nameComp{ registry.TryGetComponent<NameComponent>(ent) };
			if (nameComp.name == "Enemies_Text") {
				registry.GetComponent<TextRendererComponent>(ent)->text = mapManager.GetEnemyTypeString(offers[mainMenuButton->TileNumber].enemyType);
				++updated;
			}
			else if (nameComp.name == "Gimmick_Text") {
				registry.GetComponent<TextRendererComponent>(ent)->text = mapManager.GetGimmickTypeString(offers[mainMenuButton->TileNumber].gimmickType);
				++updated;
			}
			else if (nameComp.name == "Item_Rarity") {
				registry.GetComponent<SpriteRendererComponent>(ent)->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(
					CEO::Instance().GetManager<MapManager>()->GetItemTierSpritePath(offers[mainMenuButton->TileNumber].itemTier));
				++updated;
			}
		}
	}
	else if (sceneToSwitch == "Game") { 
		if (sceneSwitch) {
			GameStateManagerScript::Instance().SetState(GameStateManagerScript::Game); 
			CEO::Get<PersistentDataManager>()->Set("IsMinimapCamDirty", true);
		}
	}
	else if (sceneToSwitch == "Resume") { SceneManager::QueueSceneAction("PauseMenu", SceneManager::PUSH); }
	else if (sceneToSwitch == "MainMenu_Scene") {
		GameStateManagerScript::Instance().SetState(GameStateManagerScript::MainMenu);
		GameStateManagerScript::Instance().PauseGame(registry);
	}

	if (!sceneSwitch) return;
	mainMenuButton->switchScene = sceneToSwitch;
	mainMenuButton->isTransitioning = true;
};

void MapButtonManagerScript::SetSprite(SpriteRendererComponent* buttonSprite, std::string texPath, bool hover)
{
	if (!texPath.empty()) buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(texPath);
	if (hover) CEO::Instance().GetManager<ResourceManager>()->GetAudio("SFX\\UI\\UIButtonHover.wav", "SFX").Play();
}

void MapButtonManagerScript::UpdatePlayButton(bool placed) {
	Registry& registry{ *CEO::Instance().GetManager<Registry>() };
	for (auto ent : registry.GetEntitiesWithComponent<ButtonComponent>()) {
		auto& nameComp{ *registry.GetComponent<NameComponent>(ent) };
		if (nameComp.name == "Button_Play") {
			auto& button{ *registry.GetComponent<ButtonComponent>(ent) };
			auto* buttonSprite{ registry.GetComponent<SpriteRendererComponent>(ent) };

			std::string hoverTex{}, idleTex{"ui\\game\\map_play.png"};
			bool change{ false };
			if (placed) {
				hoverTex = "ui\\game\\map_play_hover.png";
				change = true;
			}
			else { 
				hoverTex = idleTex = "ui\\game\\map_play_inactive.png"; 
			}
			buttonSprite->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(idleTex);

			AssignButton(std::reference_wrapper<Registry>(registry), button, buttonSprite,
				hoverTex, idleTex, change, "Game", change);

			return;
		}
	}
}