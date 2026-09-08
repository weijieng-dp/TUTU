/*!
@file       FlashingVFX.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       20/01/2026

Declarations for a script-driven visual effect that applies a timed
flashing color animation to a SpriteRendererComponent. The effect
supports configurable duration, interval, peak hold, delay, and
can be triggered via events or script callbacks.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

/*!
 * \class FlashingVFX
 * \brief
 *   Script component that applies a flashing color effect to a sprite.
 *
 *   FlashingVFX controls time-based color flashing on a SpriteRendererComponent.
 *   It supports configurable blink duration, interval, peak hold, delay, and
 *   target color, and can be triggered via events or script calls.
 */
class FlashingVFX: public ScriptInstance
{
public:
    /*!
     * \brief
     *   Binds data from the ECS component into the script instance.
     *
     *   Copies the component data associated with this entity into
     *   the script instance for runtime execution.
     */
    void BindFrom() {
        GetComponent<FlashingVFX>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<FlashingVFX>(*CEO::Get<Registry>());
    };

    /*!
     * \brief
     *   Binds data from the script instance back into the ECS component.
     *
     *   Writes modified runtime data back to the registry component.
     */
    void BindTo() {
        *GetComponent<FlashingVFX>(*CEO::Get<Registry>()) = *this;
    };

    /*!
     * \brief
     *   Called once when the script is initialized.
     *
     * \param[in] registry
     *   ECS registry containing the entity and its components.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief
     *   Called every frame to update the flashing logic.
     *
     * \param[in] registry
     *   ECS registry containing the entity and its components.
     * \param[in] dt
     *   Delta time for the current frame.
     * \param[in] firstframe
     *   True if this is the first update frame.
     */
    void OnUpdate(Registry& registry, float dt, bool firstframe);

    /*!
     * \brief
     *   Called at a fixed timestep to update time-sensitive logic.
     *
     * \param[in] registry
     *   ECS registry containing the entity and its components.
     * \param[in] dt
     *   Fixed delta time.
     * \param[in] firstframe
     *   True if this is the first fixed update frame.
     */
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
     * \brief
     *   Callback function to start flashing of the sprite.
     *
     *   Initiates the flashing sequence based on the parameters
     *   specified in the script component.
     */
    void StartFlashing();

    /*!
     * \brief
     *   Total duration of the flashing effect.
     */
    float blinkTotalTime{ 0 };

    /*!
     * \brief
     *   Time interval between individual flashes.
     */
    float blinkInterval{ 0 };

    /*!
     * \brief
     *   Duration to hold the color at peak intensity.
     */
    float blinkPeakHold{ 0 };

    /*!
     * \brief
     *   Delay before the flashing effect starts.
     */
    float delay{ 0 };

    /*!
     * \brief
     *   Target color used for the flashing effect.
     */
    Color colorToBlink{ 1.f, 1.f, 1.f, 1.f };

    /*!
     * \brief
     *   Target game object whose sprite will flash.
     */
    GameObject target;
    GameObject additionalTarget;

    REFLECTABLE_PROPERTIES;

private:
    /*!
     * \brief
     *   Remaining time for the flashing effect.
     */
    float blinkTimeLeft{};

    /*!
     * \brief
     *   Original sprite color before flashing begins.
     */
    Color originalColor{ 1.f, 1.f, 1.f, 1.f };

    /*!
     * \brief
     *   Accumulated delta time for flashing calculations.
     */
    float blinkDt{};

    /*!
     * \brief
     *   Current peak intensity timer.
     */
    float blinkPeak{};

    /*!
     * \brief
     *   Indicates whether the flashing effect has been triggered.
     */
    bool triggered = false;

    /*!
     * \brief
     *   Cached pointer to the target's SpriteRendererComponent.
     */
    std::vector<SpriteRendererComponent*> sprites;
};

/*!
 * \brief
 *   Registers FlashingVFX fields for reflection and editor inspection.
 */
REFL_AUTO(
    type(FlashingVFX),
    field(target),
    field(additionalTarget),
    field(blinkTotalTime),
    field(blinkInterval),
    field(blinkPeakHold),
    field(delay),
    field(colorToBlink)
)