/*!
@file       Animation.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       25/09/2025
@brief  Contains data structures and functiosn related to sprite animation

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "Animation.h"
#include "Components.h"
#include "UpdateStackManager.h"

/*
* @brief set texture origin and offset based on animaton frame
*
* @param animator	- animator to calculate slice
* @param renderer	- renderer containing texture to be sliced
*/
void CalculateFrameSlice(AnimatorComponent& animator, SpriteRendererComponent& renderer)
{
    GLsizei sliceWidth = animator.currAnim->sheetSize.first / animator.currAnim->cols; // width of a single sprite frame
    GLsizei sliceHeight = animator.currAnim->sheetSize.second / animator.currAnim->rows; // height of a single sprite frame

    int row = animator.currFrame / animator.currAnim->cols;
    int col = animator.currFrame % animator.currAnim->cols;

    // offsets in texel space from origin
    int xTexelOffset = col * sliceWidth; 
    int yTexelOffset = (animator.currAnim->rows - row - 1) * sliceHeight;

    // offsets in texture space from origin
    float xTextureOffset = static_cast<float>(xTexelOffset) / renderer.texture->Width();
    float yTextureOffset = static_cast<float>(yTexelOffset) / renderer.texture->Height();

    // assign texture offset to texture in renderer
    renderer.start = { xTextureOffset, yTextureOffset };
    renderer.size = { static_cast<float>(sliceWidth) / renderer.texture->Width(),
                    static_cast<float>(sliceHeight) / renderer.texture->Height() };
}

void UpdateAnimatedSprites(Registry& _registry, float _dt)
{
    auto entities = _registry.GetEntitiesWithComponents<AnimatorComponent, SpriteRendererComponent>();
    UpdateStackManager* usm = CEO::Get<UpdateStackManager>();
    ResourceManager* rem = CEO::Get<ResourceManager>();

    for (auto entity : entities)
    {
        if (usm->ShouldNotUpdate(entity)) continue; // skipping the update if it is part of a lower stack

        SpriteRendererComponent* renderer = _registry.GetComponent<SpriteRendererComponent>(entity);
        AnimatorComponent* animator = _registry.GetComponent<AnimatorComponent>(entity);
        if (animator->currAnim)
        {
            // animation changed set spritesheet to texture
            if (animator->currAnim != animator->prevAnim)
            {
                renderer->texture = &rem->GetTexture(animator->currAnim->spriteSheetName);
                animator->currFrame = 0;
                // start playing immediately only if animation plays on start
                // prevAnim is null when entering scene, only prevent play when switching between anims
                if(animator->prevAnim != nullptr)
					animator->isPlaying = animator->currAnim->playOnStart;
                
                // set texture to the first frame of animation
                CalculateFrameSlice(*animator, *renderer);
            }
        }
        else // no texture set, skip
        {
            renderer->texture = &rem->GetTexture("error.png");
            continue;
        }
        
        // animation not playing, skip
        if (!animator->isPlaying)
        {
            animator->prevAnim = animator->currAnim;
            continue;
        }

        animator->timeElapsed += animator->animSpeed * static_cast<float>(_dt);
        // going to next animation frame or new animation
        if (animator->timeElapsed >= animator->currAnim->frameDelay || animator->currAnim != animator->prevAnim)
        {
            animator->timeElapsed = 0.f;

            // eached end of animation
            if (++animator->currFrame >= animator->currAnim->frameCount)
            {
                // stop playing if looping disabled
                if (!animator->currAnim->doLooping)
                    animator->isPlaying = false;

                animator->currFrame = 0;
            }

            CalculateFrameSlice(*animator, *renderer);
        }
        animator->prevAnim = animator->currAnim;
    }
}