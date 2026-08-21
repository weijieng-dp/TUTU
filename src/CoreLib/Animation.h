/*!
@file       Animation.h
@author     Ou Yukang (yukang.ou) 100%
@date       25/09/2025
@brief  Contains data structures and functiosn related to sprite animation
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include <string>
#include "MathLib.h"
#include "Registry.h"

struct Animation
{
	std::string animName{};			// name of animation
	std::string spriteSheetName{};	// name of spritesheet texture 
	float frameDelay{ 1.f };		// delay between each frame
	int frameCount{ 1 };			// total frames in animation
	int rows{ 1 };					// rows of sprites in spritesheet
	int cols{ 1 };					// cols of sprites in spritesheet
	std::pair<int,int> sheetOrigin{ 0,0 };	// top left origin of sprite sheet on texture
	std::pair<int, int> sheetSize{ 1,1 };		// size in texels space from the top left origin
	bool doLooping{ true };
	bool playOnStart{ true };
};

/*
* @brief Update animation frames based on time
*
* @param _registry	- ECS registry
* @param _dt		- delta time
*/
void UpdateAnimatedSprites(Registry& registry, float dt);