/*!
@file       BlindManipulationScript.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       10/03/2026 (DD/MM/YYYY0
@brief		Simple script to increase / decrease the vignette for blind
			gimmick tile.

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "BlindManipulationScript.h"
#include "../CoreLib/PersistentDataManager.h"

void BlindManipulationScript::OnStart(Registry& registry) {
	auto postProcessComp{ GetComponent<PostProcessComponent>(registry) };
	if (postProcessComp) {	// set the initial vignette values and pre-calculate the deltas
		initialVignetteIntensity = postProcessComp->vignetteIntensity;
		initialVignetteFalloff = postProcessComp->vignetteFalloff;

		float falloffDivisor{ 1.f / vignetteFalloffDuration }, intensityDivisor{ 1.f / vignetteIntensityDuration };

		falloffDelta = (targetVignetteFalloff - initialVignetteFalloff) * falloffDivisor;
		intensityDelta = (targetVignetteIntensity - initialVignetteIntensity) * intensityDivisor;
	}
};


void BlindManipulationScript::OnUpdate(Registry& registry, float dt, bool) {
	if (lerping) {
		lerpTimer += dt;

		float tFalloff{ std::min(lerpTimer / vignetteFalloffDuration, 1.f) },
			tIntensity{ std::min(lerpTimer / vignetteIntensityDuration, 1.f) };

		auto& postProcessComp{ *GetComponent<PostProcessComponent>(registry) };	// get vignette component
		postProcessComp.vignetteIntensity += intensityDelta * dt * (toBlind ? 1 : -1);
		postProcessComp.vignetteFalloff += falloffDelta * dt * (toBlind ? 1 : -1);

		if (tIntensity >= 1.f) postProcessComp.vignetteIntensity = toBlind ? targetVignetteIntensity : initialVignetteIntensity;
		if (tFalloff >= 1.f) postProcessComp.vignetteFalloff = toBlind ? targetVignetteFalloff : initialVignetteFalloff;

		if (tIntensity >= 1.f && tFalloff >= 1.f) {
			lerping = false;	// turn off lerping
			lerpTimer = 0.f;	// reset lerp timer
		}
	}
};

void BlindManipulationScript::OnFixedUpdate(Registry&, float, bool)
{
};

void BlindManipulationScript::EnableBlind(bool blind) {
	lerping = true;			// enable lerping
	toBlind = blind;
	CEO::Get<PersistentDataManager>()->Set("IsBlindModeOn", blind);	// update blind mode tracker
	if(disableMinimap) CEO::Get<PersistentDataManager>()->Set("ToggleMinimapOffDuringCombat", blind);	// if disable minimap flag is on, update tracker
}
