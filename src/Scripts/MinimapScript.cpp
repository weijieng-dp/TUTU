/*!
@file       MinimapScript.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026
@brief		Handles minimap behaviour (such as moving / updating
			minimap camera when player enters a new tile/room, lerping
			the minimap's position and viewport when moving, and toggling
			it on/off if "ToggleMinimapOffDuringCombat" flag is set.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "MinimapScript.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/MapManager.h"
#include "PlayerControllerScript.h"
#include "../CoreLib/EventsDispatcher.h"
#include "../CoreLib/PersistentDataManager.h"

void MinimapScript::OnStart(Registry& registry) {
	for (auto ent : registry.GetEntitiesWithComponent<CameraComponent>()) {
		auto name{ registry.GetComponent<NameComponent>(ent) };
		if (name && name->name == "Minimap_Camera") {

			Vec2 vpSize{ registry.GetComponent<CameraComponent>(ent)->viewportSize },
				camTranslate{ registry.GetComponent<TransformComponent>(ent)->translate };

			CEO::Get<PersistentDataManager>()->Set("InitVPSize", vpSize);		// capture the initial viewport size of minimap camera
			CEO::Get<PersistentDataManager>()->Set("MinimapCamEnt", ent);		// save the entity id of the minimap camera
			UpdateMinimapCam(camTranslate, vpSize);								// call to update minimap camera's
			break;
		}
	}

	for (auto ent : registry.GetEntitiesWithComponent<UITransformComponent>()) {
		auto name{ registry.GetComponent<NameComponent>(ent) };
		if (name && name->name == "Minimap") {
			CEO::Get<PersistentDataManager>()->Set("MinimapEnt", ent);			// get the entity id of the minimap ui entity that the minimap resides on
			break;
		}
	}

	CEO::Get<PersistentDataManager>()->Set("InCombat", false);						// reset in combat flag to false
#ifdef PLATFORM_WINDOWS
	CEO::Get<PersistentDataManager>()->Set("ToggleMinimapOffDuringCombat", toggleMinimapOffDuringCombat);	// update minimapofftoggle flag tracker
#else
	auto waveMode{ CEO::Get<PersistentDataManager>()->Get<bool>("IsWaveMode") };
	if (!waveMode || !*waveMode) CEO::Get<PersistentDataManager>()->Set("ToggleMinimapOffDuringCombat", true);
	else CEO::Get<PersistentDataManager>()->Set("ToggleMinimapOffDuringCombat", false);
#endif
	CEO::Get<PersistentDataManager>()->Set("IsBlindModeOn", false);					// reset blindmodeon flag to false
};


void MinimapScript::OnUpdate(Registry & registry, float dt, bool) {
	auto cleared{ CEO::Get<PersistentDataManager>()->Get<bool>("RoomCleared") };	// get room cleared tracker
	if (cleared && *cleared) {	// if room is cleared
		auto vpSize{ CEO::Get<PersistentDataManager>()->Get<Vec2>("InitVPSize") };
		UpdateMinimapCam(targetPos, vpSize ? *vpSize : Vec2(10240.f, 10240.f));		// update minimap camera to zoom out
	}

	if (isMinimapCamLerping) {
		auto minimapCamera{ CEO::Get<PersistentDataManager>()->Get<EntityRegistry::Entity>("MinimapCamEnt") };	// get thhe minimap camera entity id
		if (!minimapCamera || !*minimapCamera) {	// check if minimap camera has been set, else end lerp early
			isMinimapCamLerping = false;			
			lerpTimer = 0.f;
			return;
		}
		CEO::Get<PersistentDataManager>()->Set("IsMinimapCamDirty", true);	// set minimapcamdirty flag to true to update the tile background
		
		lerpTimer += dt;

		float tPos{ std::min(lerpTimer / minimapCamPosLerpDuration, 1.f) },
			tSize{std::min(lerpTimer / minimapCamSizeLerpDuration, 1.f)};

		auto& minimapCamPos{ registry.GetComponent<TransformComponent>(*minimapCamera)->translate };	// get minimap camera's translate component
		minimapCamPos += posDelta * dt;					// lerp minimap camera's position
		if(tPos >= 1.f) minimapCamPos = targetPos;

		auto& minimapCamVPSize{ registry.GetComponent<CameraComponent>(*minimapCamera)->viewportSize };	// get minimap camera's viewport component
		minimapCamVPSize += vpDelta * dt;				// lerp minimap camera's viewport size
		if(tSize >= 1.f) minimapCamVPSize = targetVpSize;


		if (tPos >= 1.f && tSize >= 1.f) {				// check if lerp is over
			isMinimapCamLerping = false;
			lerpTimer = 0.f;
			CEO::Get<PersistentDataManager>()->Set("IsMinimapCamDirty", false);	// set minimapcamdirty flag to false
		}
	}
};

void MinimapScript::OnFixedUpdate(Registry&, float, bool) {

};

void MinimapScript::UpdateMinimapCam(int tileIndex) {
	auto& mapManager{ *CEO::Get<MapManager>() };
	auto tile{ mapManager.GetTileInstance(tileIndex) };
	if (tile == nullptr) return;			// check validity of tileIndex

	auto gridPos{ mapManager.CalculateGridPos(*tile) };			// get the tile's position in grid coordinates
	int minX{ gridPos[0].first }, maxX{ gridPos[0].first };
	int minY{ gridPos[0].second }, maxY{ gridPos[0].second };
	for (const auto& pos : gridPos) {
		minX = std::min(minX, pos.first);
		maxX = std::max(maxX, pos.first);
		minY = std::min(minY, pos.second);
		maxY = std::max(maxY, pos.second);
	}
	// calculate the center of the tile
	MapManager::GridPos center{
		static_cast<int>((minX + maxX) * 0.5f),
		static_cast<int>((minY + maxY) * 0.5f)
	};

	auto worldPos{ mapManager.ConvertGridToWorld(center) };		// convert center into world position
#ifdef PLATFORM_WINDOWS
	// Change viewport size based on whether player is in combat or not
	Vec2 viewportSize{};
	auto tileMultiplier{ mapManager.GetTileMultiplier() };
	auto isCombat{ CEO::Get<PersistentDataManager>()->Get<bool>("InCombat") };
	if (isCombat && *isCombat) {
		if (minX != maxX) worldPos.x += tileMultiplier.x * 0.5f;
		if (minY != maxY) worldPos.y += tileMultiplier.y * 0.5f;

		float maxExtent{ std::max((maxX - minX + 1) * tileMultiplier.x, (maxY - minY + 1) * tileMultiplier.y) };
		viewportSize = { maxExtent, maxExtent };
	}
	else {
		auto initViewportSize{ CEO::Get<PersistentDataManager>()->Get<Vec2>("InitVPSize") };
		if (initViewportSize) viewportSize = *initViewportSize;
	}
	UpdateMinimapCam(worldPos, viewportSize);
#else
	auto viewportSize{ CEO::Get<PersistentDataManager>()->Get<Vec2>("InitVPSize") };
	UpdateMinimapCam(worldPos, viewportSize ? *viewportSize : Vec2(10240.f, 10240.f));
#endif
}
void MinimapScript::UpdateMinimapCam(const Vec2& target, const Vec2& vpSize) {
	auto minimapCamera{ CEO::Get<PersistentDataManager>()->Get<EntityRegistry::Entity>("MinimapCamEnt") };
	if (!minimapCamera || !*minimapCamera) return;
	// Set up lerping
	isMinimapCamLerping = true;
	targetPos = target;
	targetVpSize = vpSize;

	const auto& currentVpSize{ CEO::Get<Registry>()->GetComponent<CameraComponent>(*minimapCamera)->viewportSize };
	const auto& currentPos{ CEO::Get<Registry>()->GetComponent<TransformComponent>(*minimapCamera)->translate };

	// pre-calculate the delta in position and viewport size
	float posDivisor{ 1.f / minimapCamPosLerpDuration }, sizeDivisor{ 1.f / minimapCamSizeLerpDuration };
	posDelta = { (target.x - currentPos.x) * posDivisor, (target.y - currentPos.y) * posDivisor };
	vpDelta = { (vpSize.x - currentVpSize.x)* sizeDivisor, (vpSize.y - currentVpSize.y)* sizeDivisor };

	lerpTimer = 0.f;
}