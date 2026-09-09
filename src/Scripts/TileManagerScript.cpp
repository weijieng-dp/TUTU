/**___________________________________________________________________________/
@file          TileManager.cpp
@author        Tan Jun Jie (t.junjie) (100%)
@date          03/02/2026 (DD/MM/YYYY)
@brief         Handles drag-and-drop of tiles in Map.scene.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "TileManagerScript.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/InputManager.h"
#include "../CoreLib/Camera.h"
#include "../CoreLib/UIManager.h"
#include "../CoreLib/PersistentDataManager.h"
#include "MapButtonManagerScript.h"

/*!
* \brief Handles picking back up of placed map tile.
* \param[in] ent		- The entity id of the entity with the tilemanagerscript.
* \param[in, out] go	- The map tile game object to remove / pick up.
* \param[in] index		- The tile instance index to pick up.
*/
static void PickupPlacement(EntityRegistry::Entity ent, GameObject& go, int index) {
	auto& registry{ *CEO::Get<Registry>() };
	auto tileScript{ registry.GetComponent<TileManagerScript>(ent) };
	if (tileScript->placing) return;

	auto& mapManager{ *CEO::Get<MapManager>() };

	auto* tile{ mapManager.GetTileInstance(index) };
	tileScript->savedGridPos = tile->anchor;

	auto tileScreenPos{ go.GetComponent<UITransformComponent>()->relativePos };
	mapManager.RemoveTile(index);			// remove the placed tile from map

	registry.GetComponent<ActiveComponent>(tileScript->placedTile.GetEntityID())->isActiveSelf = false;

	tileScript->placed = false;
	tileScript->placing = true;
	tileScript->selectedOfferIndex = tileScript->lastSelectedOfferIndex;
	tileScript->previewTileEntity = tileScript->SpawnPreviewEntity(tileScreenPos);
	
}

void TileManagerScript::OnStart(Registry& registry) 
{
#ifdef PLATFORM_ANDROID
	// disable joysticks
	Input::GetControllers()[0].active = false;
	Input::GetControllers()[1].active = false;
#endif
#ifdef PLATFORM_WINDOWS
	CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeCursor>(Events::ChangeCursor{ "pointer"});
#endif

	auto& mapManager{ *CEO::Get<MapManager>() };
	auto& achievementManager{ *CEO::Get<AchievementManager>() };


	placed = false;	// set placed to false when entering map scene
	mapManager.GenerateTileOffers();	// generate the 3 random tile choices player can choose

	auto* start{ mapManager.GetTile(TileType::START) };
	auto* end{ mapManager.GetTile(TileType::END) };
	auto* treasure{ mapManager.GetTile(TileType::TREASURE) };
	auto tiles{ mapManager.GetTileOffers() };

	UITransformComponent ui;
	unsigned layerPriority{};
	// Set up the objects in map scene
	for (auto ent : registry.GetEntitiesWithComponent<UITransformComponent>()) {
		NameComponent& nameComp{ registry.TryGetComponent<NameComponent>(ent) };

		if (nameComp.name == "Start_Tile") {
			layerPriority = registry.GetComponent<LayerComponent>(ent)->renderPriority;
			ui = *registry.GetComponent<UITransformComponent>(ent);
			startTile = ent;
		}
		else if (nameComp.name == "End_Tile") { endTile = ent; }
		else if (nameComp.name == "Treasure_Tile") { treasureTile = ent; }
		else if (nameComp.name == "TileSelect") {
			registry.GetComponent<SpriteRendererComponent>(ent)->texture = tiles[0].tileData->sprite.first;
		}
		else if (nameComp.name == "Enemies_Text") {
			registry.GetComponent<TextRendererComponent>(ent)->text =
				achievementManager.GetEnemyTypeString(tiles[0].enemyType);
		}
		else if (nameComp.name == "Gimmick_Text") {
			registry.GetComponent<TextRendererComponent>(ent)->text =
				mapManager.GetGimmickTypeString(tiles[0].gimmickType);
		}
		else if (nameComp.name == "Item_Rarity") {
			registry.GetComponent<SpriteRendererComponent>(ent)->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(
				mapManager.GetItemTierSpritePath(tiles[0].itemTier));
		}
 	}

	if (startTile && endTile && treasureTile) {
		auto pos{ mapManager.CalculateGridPos(*start->tileData, start->anchor) };
		auto uiComp{ registry.GetComponent<UITransformComponent>(startTile) };
		bottomLeft = uiComp->relativePos;

		uiComp->relativePos.y = TranslateToScreenCoordinates(pos[0].first, pos[0].second).y;

		// set treasure tile
		pos = mapManager.CalculateGridPos(*treasure->tileData, treasure->anchor);
		uiComp = registry.GetComponent<UITransformComponent>(treasureTile);
		uiComp->relativePos.y = TranslateToScreenCoordinates(pos[0].first, pos[0].second).y;

		// set end tile
		pos = mapManager.CalculateGridPos(*end->tileData, end->anchor);
		uiComp = registry.GetComponent<UITransformComponent>(endTile);
		uiComp->relativePos.y = TranslateToScreenCoordinates(pos[0].first, pos[0].second).y;
	}
	SpawnTileInstances(registry, std::move(ui), layerPriority);	// spawn all the COMBAT tile instances
	ControllerCoord = { 0,0 };

	for (auto ent : registry.GetEntitiesWithComponent<MapButtonManagerScript>()) {
		mapButtonManagerScriptEnt = ent;
		break;
	}
	for (auto buttonEnt : registry.GetEntitiesWithComponent<ButtonComponent>()) {
		auto nameComp{ registry.GetComponent<NameComponent>(buttonEnt) };
		if (nameComp && nameComp->name == "Button_Play") {
			if (mapManager.CheckPathValidity(mapManager.GetStartTileIndex(), mapManager.GetEndTileIndex())) {
				if (mapButtonManagerScriptEnt) {
					registry.GetComponent<MapButtonManagerScript>(mapButtonManagerScriptEnt)->UpdatePlayButton(true);	// update the play button in Map.scene (un-grey it)
				}
			}
			break;
		}
	}
}

void TileManagerScript::OnUpdate(Registry& registry, float dt, bool) {
	auto& input{ *Input::GetInstance() };
#ifdef PLATFORM_WINDOWS
	if (placed) {
		if (Input::GetInstance()->IsGamepadButtonReleased(Input::GamepadButton::B)) {
			CEO::Get<MapManager>()->RemoveTile(placedTileIndex);
			placedTile.Destroy();
			CEO::Get<EventsDispatcher>()->Dispatch<Events::UpdateSelectedEntity>(Events::UpdateSelectedEntity{ 0 });

			CancelPlacement(registry, true);
		}
	}
#endif
	if (!placing || previewTileEntity.GetEntityID() == 0) return;	// skip iif no tiles are currently in drag-and-drop
#ifdef PLATFORM_WINDOWS
	if (Input::IsGamepadConnected(0)) {
		Vec2 stick = Input::GetLeftStick();

		if (inputCooldown > 0.0f) {
			inputCooldown -= dt;
		}
		else {
			if (stick.x > 0.5f) { ControllerCoord.first += 1; inputCooldown = 0.2f; }
			if (stick.x < -0.5f) { ControllerCoord.first -= 1; inputCooldown = 0.2f; }
			if (stick.y > 0.5f) { ControllerCoord.second += 1; inputCooldown = 0.2f; }
			if (stick.y < -0.5f) { ControllerCoord.second -= 1; inputCooldown = 0.2f; }
		}

		 int mapWidth{ CEO::Get<MapManager>()->GetMapGridWidth() },
			 mapHeight{ CEO::Get<MapManager>()->GetMapGridHeight() };

		if (ControllerCoord.first < 0) ControllerCoord.first = 0;
		if (ControllerCoord.second < 0) ControllerCoord.second = 0;
		if (ControllerCoord.first >= mapWidth ) ControllerCoord.first = mapWidth - 1;
		if (ControllerCoord.second >= mapHeight) ControllerCoord.second = mapHeight - 1;
	}
#endif

#ifdef EditorFlag
	Vec2 screenPos{ CameraManager::EditorPosToViewportPos(Vec2{ input.GetX(), input.GetY() }) };
#else
#ifdef PLATFORM_WINDOWS
	Vec2 screenPos{ CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{input.GetX(), input.GetY()}) };
#else
	Vec2 screenPos = { CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{input.GetX(0),input.GetY(0)} )};
#endif
#endif
	auto& map{ *CEO::Get<MapManager>() };
	auto offers{ map.GetTileOffers() };
	UITransformComponent& transform{ *previewTileEntity.GetComponent<UITransformComponent>()};

#ifndef PLATFORM_WINDOWS
    transform.relativePos = screenPos - grabOffset;
#endif
#ifdef PLATFORM_WINDOWS
	if (!Input::IsGamepadConnected(0)) {
            transform.relativePos = screenPos - grabOffset;
	}
	else if(input.IsGamepadButtonHeld(Input::GamepadButton::A)){
		auto currentChoice{ offers[selectedOfferIndex].tileData };
		auto clampedAnchor{ map.ClampAnchor(*currentChoice, ControllerCoord) };
		auto pos{ map.CalculateGridPos(*currentChoice, clampedAnchor) };
		int minX{ INT_MAX }, minY{ INT_MAX };
		for (const auto& p : pos) {
			minX = std::min(minX, p.first);
			minY = std::min(minY, p.second);
		}

		transform.relativePos = TranslateToScreenCoordinates(minX, minY);
	}
	if (input.IsButtonRelease(GLFW_MOUSE_BUTTON_LEFT)|| input.IsGamepadButtonReleased(Input::GamepadButton::A)) {
#else
    if(input.IsPointerRelease(0)) {
#endif
		auto result{ map.PlaceTile(offers[selectedOfferIndex], TranslateToGridAnchor(transform.relativePos)) };
		
		if (result.result == PlaceTileResult::SUCCESS) {	// if tile placement was successful
			CEO::Get<ResourceManager>()->GetAudio("SFX\\UI\\TileDrop.wav", "SFX").Play();
			placedTile.Destroy();
			ConfirmPlacement(registry, result.index);
		}
		else {
			bool reset{ true };
			if (result.result == PlaceTileResult::OUTOFBOUNDS) {
				placed = false;
				placedTile.Destroy();
			}
			else if (savedGridPos.first != -1) {
				auto restore{ map.PlaceTile(offers[selectedOfferIndex], savedGridPos) };
				if (restore.result == PlaceTileResult::SUCCESS) { 
					placedTile.Destroy();
					ConfirmPlacement(registry, restore.index);
				}
				else {
					registry.GetComponent<ActiveComponent>(placedTile.GetEntityID())->isActiveSelf = true;
				}
				reset = false;
			}
			CancelPlacement(registry, reset);
		}

		previewTileEntity = 0;							// reset dragging tile entity id to 0
		lastSelectedOfferIndex = selectedOfferIndex;	// update last selected offer tile index with current index
		selectedOfferIndex = -1;						// reset index to -1
		placing = false;								// reset placing flag
	}
}
void TileManagerScript::OnFixedUpdate(Registry&, float, bool) {
}

Vec2 TileManagerScript::TranslateToScreenCoordinates(int x, int y) {
	return Vec2(bottomLeft.x + x * increment, bottomLeft.y + y * increment);
}

std::pair<int, int> TileManagerScript::TranslateToGridAnchor(const Vec2& screenPos) {
	float gx{ (screenPos.x - bottomLeft.x) / increment };
	float gy{ (screenPos.y - bottomLeft.y) / increment };

	int x{ static_cast<int>(std::round(gx)) };
	int y{ static_cast<int>(std::round(gy)) };

		return std::make_pair(x, y);
}

void TileManagerScript::SpawnTileInstances(Registry& registry, UITransformComponent&& transform, unsigned layerPriority) {
	auto& map{ *CEO::Instance().GetManager<MapManager>() };

	for (int i{}; i < map.GetTileInstanceSize(); ++i) {
		auto* tile{ map.GetTileInstance(i) };
		if (tile->tileData->type != TileType::COMBAT) continue;

		GameObject tileObj{ CreateUIGameobject(registry.CreateEntity()) };
		tileObj.AddComponent<SpriteRendererComponent>();

		tileObj.GetComponent<NameComponent>()->name = tile->tileData->tileName + std::to_string(i);
		tileObj.GetComponent<SpriteRendererComponent>()->texture = tile->tileData->sprite.first;
		*tileObj.GetComponent<UITransformComponent>() = transform;
		tileObj.GetComponent<UITransformComponent>()->relativePos = TranslateToScreenCoordinates(tile->anchor.first, tile->anchor.second);
		tileObj.GetComponent<LayerComponent>()->renderPriority = layerPriority;
	}
}

bool TileManagerScript::BeginPlacement(int index) {
	if (placing || placed) return false;	// if tile is placing or already placed, early return
	placing = true;							// set placing flag to true
	selectedOfferIndex = index;				// get the tile chpice index
	previewTileEntity = SpawnPreviewEntity();	// spawn the tile to follow input for dragging
	
	return true;
}

GameObject TileManagerScript::SpawnPreviewEntity() {
	if (selectedOfferIndex == -1) return 0;

	auto offers{ CEO::Instance().GetManager<MapManager>()->GetTileOffers() };
	Registry& registry{ *CEO::Instance().GetManager<Registry>() };

	UITransformComponent ui;

	for (auto ent : registry.GetEntitiesWithComponent<ButtonComponent>()) {
		NameComponent& name{ registry.TryGetComponent<NameComponent>(ent) };
		if (name.name == "TileSelect") {
			ui = *registry.GetComponent<UITransformComponent>(ent);
		}
	}

	GameObject previewTile{ CreateUIGameobject(registry.CreateEntity()) };
	previewTile.AddComponent<SpriteRendererComponent>();

	auto* sprite{ previewTile.GetComponent<SpriteRendererComponent>() };
	sprite->texture = offers[selectedOfferIndex].tileData->sprite.first;

	auto* transform{ previewTile.GetComponent<UITransformComponent>() };
	*transform = ui;

	previewTile.GetComponent<LayerComponent>()->renderPriority = 11;

#ifdef PLATFORM_WINDOWS
	if (!Input::IsGamepadConnected(0))
	{
		grabOffset = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(), Input::GetY() }) - transform->relativePos;

	}
	
#endif
#ifdef PLATFORM_ANDROID
	grabOffset = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(0), Input::GetY(0) }) - transform->relativePos;
#endif

	return previewTile;
}

GameObject TileManagerScript::SpawnPreviewEntity(const Vec2& startPos) {
	if (selectedOfferIndex == -1) return 0;
	auto ent{ SpawnPreviewEntity() };
	auto transform{ ent.GetComponent<UITransformComponent>()};
	transform->relativePos = startPos;

#ifdef PLATFORM_WINDOWS
	grabOffset = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(), Input::GetY() }) - transform->relativePos;
#endif
#ifdef PLATFORM_ANDROID
	grabOffset = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(0), Input::GetY(0) }) - transform->relativePos;
#endif
	return ent;
}

void TileManagerScript::ConfirmPlacement(Registry& registry, int index) {
	UITransformComponent ui;
	for (auto ent : registry.GetEntitiesWithComponent<UITransformComponent>()) {
		NameComponent& name{ registry.TryGetComponent<NameComponent>(ent) };
		if (name.name == "Start_Tile") {
			ui = *registry.GetComponent<UITransformComponent>(ent);
			break;
		}
	}
	// get the tile instance
	auto* tile{ CEO::Instance().GetManager<MapManager>()->GetTileInstance(index) };

	Vec2 screenPos{ TranslateToScreenCoordinates(tile->anchor.first, tile->anchor.second) };
	ui.relativePos = screenPos;

	placedTile = CreateUIGameobject(registry.CreateEntity());	// create tile entity on map grid
	placedTile.AddComponent<SpriteRendererComponent>();						// add sprite renderer component for sprite texture
	placedTile.GetComponent<LayerComponent>()->renderPriority = 11;			// set render priority to be high
	*placedTile.GetComponent<UITransformComponent>() = ui;					// copy the UI transform to be the same as Start_Tile
	placedTile.GetComponent<SpriteRendererComponent>()->texture = tile->tileData->sprite.first;	// set the texture

	placedTile.AddComponent<ButtonComponent>();
	auto placedTileBtnComp{ placedTile.GetComponent<ButtonComponent>() };

	placedTileIndex = index;
	placedTileBtnComp->onHeld = [this, index]() { PickupPlacement(entity, placedTile, index); };

	previewTileEntity.Destroy();		// despawn the preview drag-and-drop tile
	CEO::Get<EventsDispatcher>()->Dispatch<Events::UpdateSelectedEntity>(Events::UpdateSelectedEntity{ placedTile.GetEntityID()});
	CEO::Get<PersistentDataManager>()->Set("RoomCleared", false);					// reset room cleared flag to false

	placing = false;
	placed = true;		// set place flag to true

	if (mapButtonManagerScriptEnt) {
		auto& mapScript{ *registry.GetComponent<MapButtonManagerScript>(mapButtonManagerScriptEnt) };
		auto& sprite{ *registry.GetComponent<SpriteRendererComponent>(mapScript.DisplayTileEntity) };
		sprite.visible = true;		// reset selected tile choice visibility back to true
		sprite.color = { 0.5f, 0.5f, 0.5f, 0.5f };	// grey out all tile choices

		mapScript.UpdatePlayButton(placed);	// update the play button in Map.scene (un-grey it)
	}
}

void TileManagerScript::CancelPlacement(Registry& registry, bool reset) {
	previewTileEntity.Destroy();	// remove the dragging tile entity
	// reset tile that is being dragged to be visible
	if (reset) {
		if (mapButtonManagerScriptEnt) {
			auto& mapScript{ *registry.GetComponent<MapButtonManagerScript>(mapButtonManagerScriptEnt) };
			auto& sprite{ *registry.GetComponent<SpriteRendererComponent>(mapScript.DisplayTileEntity) };
			sprite.visible = true;
			sprite.color = { 1.f, 1.f, 1.f, 1.f };	// grey out all tile choices

			mapScript.UpdatePlayButton(false);		// update the play button in Map (grey it out).
		}
		placed = false;				// reset placed flag
		placedTileIndex = -1;		// reset index to -1	
	}
	savedGridPos = { -1, -1 };
	CEO::Get<PersistentDataManager>()->Set("RoomCleared", true);
	CEO::Get<ResourceManager>()->GetAudio("SFX\\UI\\TileDestroy.wav", "SFX").Play(0.5f);
}