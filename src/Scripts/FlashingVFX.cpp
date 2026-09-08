/*!
@file       FlashingVFX.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       20/01/2026

Implementation of the FlashingVFX script.

Implements a script-driven visual effect that applies a timed
flashing color animation to a SpriteRendererComponent. The effect
supports configurable duration, interval, peak hold, delay, and
can be triggered via events or script callbacks.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/

#include "FlashingVFX.h"
#include "../CoreLib/EventsDispatcher.h"
#include "../CoreLib/CEO.h"	

void FlashingVFX::OnStart(Registry& r)
{
	SpriteRendererComponent* sprite = nullptr;
	// Cache SpriteRendererComponent from target entity if it exists
	sprite = target.GetEntityID()
		? r.GetComponent<SpriteRendererComponent>(target.GetEntityID())
		: r.GetComponent<SpriteRendererComponent>(entity);
	if (sprite)
		sprites.push_back(sprite);

	if (additionalTarget.GetEntityID())
	{
		sprite = r.GetComponent<SpriteRendererComponent>(additionalTarget.GetEntityID());
		if(sprite)
			sprites.push_back(sprite);
	}

	// Log error if sprite component is missing
	if (!sprites.empty()) {
		LOGE("No game object specified or game object does not have a SpriteRendererComponent");
	}
};

void FlashingVFX::OnUpdate(Registry&, float, bool)
{
	// No per-frame update logic required
};

void FlashingVFX::OnFixedUpdate(Registry&, float dt, bool)
{
	if (!sprites.empty()) {
		// Decrease remaining blink time if active
		blinkTimeLeft = blinkTimeLeft > 0.f ? blinkTimeLeft - dt : blinkTimeLeft;

		// If blinking is active
		if (blinkTimeLeft > 0.f && blinkTimeLeft <= blinkTotalTime) {

			// Accumulate blink interval time
			blinkDt += dt;

			// Clamp blink time within a single interval
			blinkDt = std::clamp(blinkDt, 0.f, blinkInterval);

			float lerpValue = 1.f;

			// Ramp up towards flash color
			if (blinkDt < blinkPeak) {
				lerpValue = blinkDt / blinkPeak;
			}
			// Ramp back down to original color
			else if (blinkDt > blinkInterval - blinkPeak) {
				lerpValue =
					(blinkPeak - (blinkDt - blinkInterval / 2)) / blinkPeak;
			}
			
			for (SpriteRendererComponent* sprite : sprites)
			{
				// Lerp each color channel independently
				sprite->color.r =
					originalColor.r +
					(colorToBlink.r - originalColor.r) * lerpValue;

				sprite->color.g =
					originalColor.g +
					(colorToBlink.g - originalColor.g) * lerpValue;

				sprite->color.b =
					originalColor.b +
					(colorToBlink.b - originalColor.b) * lerpValue;

				sprite->color.a =
					originalColor.a +
					(colorToBlink.a - originalColor.a) * lerpValue;
			}


			// Reset interval timer once a full blink cycle completes
			if (blinkDt == blinkInterval)
				blinkDt = 0.f;
		}
		// Reset color once blinking finishes
		else if (blinkTimeLeft <= 0.f && triggered) {
			for (SpriteRendererComponent* sprite : sprites)
				sprite->color = originalColor;
			triggered = false;
		}
	}
};

void FlashingVFX::StartFlashing()
{
	if (!sprites.empty()) {
		// Store original color if starting fresh
		if (blinkTimeLeft <= 0) {
			for (SpriteRendererComponent* sprite : sprites)
				originalColor = sprite->color;
		}
		// Restore original color if retriggered mid-flash
		else {
			for (SpriteRendererComponent* sprite : sprites)
				sprite->color = originalColor;
		}

		// Reset blink timers
		blinkTimeLeft = blinkTotalTime + delay;
		blinkDt = 0;

		// Calculate peak time for rising/falling color interpolation
		blinkPeak =
			std::clamp(
				(blinkInterval - blinkPeakHold) / 2.f,
				0.f,
				blinkInterval);

		// Mark flashing as active
		triggered = true;
	}
	else {
		// Safety log if sprite component is missing
		LOGE("No game object specified or game object does not have a SpriteRendererComponent");
	}
}
