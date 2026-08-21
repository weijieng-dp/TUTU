/**___________________________________________________________________________/
@file          Pathfind.h
@author        j.junbo@digipen.edu
@date          9/29/2025

This does 2 things, DrawCollisionMap draws a grid map in 2 resolutions, as there
are 2 sizes for our enemies. MoveTo is a helper function to help with pathfinding

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "Pathfind.h"
#include "Components.h"


void Pathfind::DrawCollisionMap(Registry& r) {
	if (!playerPosSet) return;

	for (int i{}; i < 2; i++) {

		std::vector<unsigned>* CollisionMap;
		unsigned collisionRes{};

		if (i == 0) { CollisionMap = &CollisionMapSmall; collisionRes = 160; }
		else { CollisionMap = &CollisionMapNormal; collisionRes = 320; }

		unsigned colliMapWidth = static_cast<unsigned>(CollisionMapWidth) / collisionRes + 1;		// the x dimension of the collision map
		unsigned colliMapHeight = static_cast<unsigned>(CollisionMapHeight) / collisionRes + 1;		// the y dimension of the collision map

		// Intializing the collision map
		std::vector<EntityRegistry::Entity> collIdList = r.GetEntitiesWithComponent<CollisionComponent>();
		if (CollisionMap->empty()) CollisionMap->resize(colliMapWidth * colliMapHeight);
		std::fill(CollisionMap->begin(), CollisionMap->end(), 0);
		for (EntityRegistry::Entity e : collIdList) {
			if (e != playerID) { // excluding the player
				if (r.HasComponent<EnemyComponent>(e)) continue;
				CollisionComponent* cc = r.GetComponent<CollisionComponent>(e);
				if (cc->isTrigger) continue;
				// Getting the min/max coordinates of the collidable entity
				std::pair<unsigned, unsigned> mincoords = WorldToIndexPair(cc->worldMin, collisionRes);
				std::pair<unsigned, unsigned> maxcoords = WorldToIndexPair(cc->worldMax, collisionRes);
				// filling up the vector
				for (unsigned y{ mincoords.first }; y < maxcoords.first; y++) {
					std::fill_n(CollisionMap->begin() + y * colliMapWidth + mincoords.second, maxcoords.second - mincoords.second, -1);
				}
			}
		}

		std::pair<unsigned, unsigned> pcoords = WorldToIndexPair(playerPos, collisionRes);
		std::queue<std::pair<unsigned, unsigned>> queue1, queue2;									// No difference if im using stack or queue. Underlying container is also just a deque anyways
		std::queue<std::pair<unsigned, unsigned>>* inqueue{ &queue1 }, * outqueue{ &queue2 };		// Additional pointers to make swapping the queues much simpler
		auto updateTile = [colliMapWidth,colliMapHeight,&CollisionMap,this](int x, int y, int curr, const std::pair<unsigned, unsigned>& p, std::queue<std::pair<unsigned, unsigned>>* queue) {
			unsigned newx = p.second + x, newy = p.first + y;
			if (newx >= colliMapWidth || newy >= colliMapHeight) return;	// nothing to update
			if ((*CollisionMap)[IndexPairToIndex(newy, newx, colliMapWidth)] == 0) {
				queue->emplace(std::make_pair(newy, newx));
				(*CollisionMap)[IndexPairToIndex(newy, newx, colliMapWidth)] = curr + 1;
			}
			};

		// This is most likely the slowest part of the code by far as it effectively iterates through the entire vector, but will
		// also be doing alot of jumping around when I increment/decrement the y so the CPU most likely cannot optimize as well
		// What this is accomplishing is basically: the center player tile gets set the value 0. Then it checks the 4 adjacent tiles.
		// If the adjacent tile is of value 0, it updates it based on current tile value +1. 
		// The inqueue stores the current list of tiles im iterating through, and the newly updated tiles gets pushed into the outqueue
		// Then after it is done iterating through the inqueue, I swap the inqueue and outqueue pointers such that the newly updated tiles
		// are now what I need to iterate through. This continues until outqueue is empty which results in inqueue being empty.
		inqueue->push(pcoords);
		while (!inqueue->empty()) {
			while (!inqueue->empty()) {
				std::pair<unsigned, unsigned> incoords = inqueue->front();

				int currVal = (*CollisionMap)[IndexPairToIndex(incoords, colliMapWidth)];
				inqueue->pop();

				updateTile(1, 0, currVal, incoords, outqueue);
				updateTile(0, 1, currVal, incoords, outqueue);
				updateTile(-1, 0, currVal, incoords, outqueue);
				updateTile(0, -1, currVal, incoords, outqueue);
			}
			std::swap(inqueue, outqueue);
		}
		(*CollisionMap)[IndexPairToIndex(pcoords, colliMapWidth)] = 0; // The player center tile will have the value 2 from the formula, so this is resetting it back to 0

		// Debug print.
		//std::cout << '\n';
		//for (int i{}; i < colliMapHeight; i++) {
		//	std::for_each(CollisionMap->begin() + i * colliMapWidth, CollisionMap->begin() + (i + 1) * colliMapWidth, [](int i) { std::cout << std::setw(2) << std::setfill('0') << i << ' '; }); std::cout << '\n';
		//}
	}
	
}

std::pair<Vec2, Vec2> Pathfind::MoveTo(const Vec2& from, const Vec2& to, bool small) {

	if (!playerPosSet) return { to,{0,0} };

	std::vector<unsigned>* CollisionMap;
	unsigned collisionRes{};

	if (small) { CollisionMap = &CollisionMapSmall; collisionRes = 160; }
	else { CollisionMap = &CollisionMapNormal; collisionRes = 320; }

	unsigned colliMapWidth = static_cast<unsigned>(CollisionMapWidth) / collisionRes + 1;		// the x dimension of the collision map
	unsigned colliMapHeight = static_cast<unsigned>(CollisionMapHeight) / collisionRes + 1;		// the y dimension of the collision map

	if (CollisionMap->empty()) {
		return std::make_pair(to, to - from);
	}


	std::pair<unsigned, unsigned> arrPos = WorldToIndexPair(from, collisionRes);			// The [y][x] coordinates of the starting position
	std::pair<unsigned, unsigned> toPos = WorldToIndexPair(to, collisionRes);				// The [y][x] coordinates of the destination position
	unsigned currVal = (*CollisionMap)[IndexPairToIndex(arrPos, colliMapWidth)];			// The value of the tile at the starting coordinate
	unsigned toVal = (*CollisionMap)[IndexPairToIndex(toPos, colliMapWidth)];				// The value of the tile at the destination coordinate
	int stepy{}, stepx{};																	// The values will be 1/-1 based on the direction of traversal
	unsigned startx;																		// This is to store the starting x location for the calculation of the final vector later on
	bool invert{ currVal < toVal };															// This is if the destination has a higher tile value than the starting position, it inverts the checks
	bool (*comp)(unsigned, unsigned) = [](unsigned u1, unsigned u2) { return u1 < u2; };	// A basic comparison function pointer that can be changed if inversion is needed

	startx = arrPos.second;


	if ((*CollisionMap)[IndexPairToIndex(std::clamp<unsigned>(arrPos.first + 1, 0, colliMapHeight - 1), arrPos.second, colliMapWidth)] < currVal) stepy = 1;
	else if ((*CollisionMap)[IndexPairToIndex(std::clamp<unsigned>(arrPos.first - 1, 0, colliMapHeight - 1), arrPos.second, colliMapWidth)] < currVal) stepy = -1;
	if ((*CollisionMap)[IndexPairToIndex(arrPos.first, std::clamp<unsigned>(arrPos.second + 1, 0, colliMapWidth - 1), colliMapWidth)] < currVal) stepx = 1;
	else if ((*CollisionMap)[IndexPairToIndex(arrPos.first, std::clamp<unsigned>(arrPos.second - 1, 0, colliMapWidth - 1), colliMapWidth)] < currVal)stepx = -1;

	if (invert) {
		// This is for if the target position if further from the player.
		stepx *= -1;
		stepy *= -1;
		comp = [](unsigned u1, unsigned u2) { return u1 > u2; };
	}

	// This is to deal with the cases where the center sees a clear path but some part of the entity has not fully cleared the collidable block yet
	if ((*CollisionMap)[IndexPairToIndex(WorldToIndexPair({ from.x + (float)collisionRes * stepx, from.y - (float)collisionRes / 2.f }, collisionRes), colliMapWidth)] == -1) {
		stepx = 0; stepy = 0;
		arrPos.first++;
	}
	else if((*CollisionMap)[IndexPairToIndex(WorldToIndexPair({ from.x + (float)collisionRes * stepx, from.y + (float)collisionRes / 2.f }, collisionRes), colliMapWidth)] == -1) {
		stepx = 0; stepy = 0;
		arrPos.first--;
	}
	if ((*CollisionMap)[IndexPairToIndex(WorldToIndexPair({ from.x - (float)collisionRes / 2.f, from.y + (float)collisionRes * stepy }, collisionRes), colliMapWidth)] == -1) {
		if (stepx == 0) {
			stepx = 0; stepy = 0;
			arrPos.second++;
		}
	}
	else if ((*CollisionMap)[IndexPairToIndex(WorldToIndexPair({ from.x + (float)collisionRes / 2.f, from.y + (float)collisionRes * stepy }, collisionRes), colliMapWidth)] == -1) {
		if (stepx == 0) {
			stepx = 0; stepy = 0;
			arrPos.second--;
		}
	}

	// Start stepping in the x direction first. 
	while (stepx != 0) {
		if (arrPos.second + stepx >= colliMapWidth) { break; }
		unsigned newVal = (*CollisionMap)[IndexPairToIndex(arrPos.first, arrPos.second + stepx, colliMapWidth)];
		if (newVal == -1) { break; }
		if (comp(newVal, currVal)) {
			currVal = newVal;
			arrPos.second += stepx;
		}
		else {
			break;
		}
	}

	// Start stepping in the y direction, but instead of only checking the stepped tile, it checks each
	// row from the startx to where the x stepping stopped. This is a simpler implementaion to check for
	// blocking in diagonal movement but also more inaccurate. For my purposes I deem it good enough
	while (stepy != 0) {

		if (arrPos.first + stepy >= colliMapHeight) { break; }
		unsigned newVal = (*CollisionMap)[IndexPairToIndex(arrPos.first + stepy, arrPos.second, colliMapWidth)];
		if (newVal == -1) { break; }
		if (comp(newVal, currVal)) {
			currVal = newVal;

			if (stepx > 0) {
				if (std::any_of(CollisionMap->begin() + IndexPairToIndex(arrPos.first, startx, colliMapWidth),
					CollisionMap->begin() + IndexPairToIndex(arrPos.first, arrPos.second, colliMapWidth),
					[](unsigned u) { return u == -1; })) {
					break;
				}
			}
			else {
				if (std::any_of(CollisionMap->begin() + IndexPairToIndex(arrPos.first, arrPos.second, colliMapWidth),
					CollisionMap->begin() + IndexPairToIndex(arrPos.first, startx, colliMapWidth),
					[](unsigned u) { return u == -1; })) {
					break;
				}
			}
			arrPos.first += stepy;
		}
		else {
			break;
		}
	}

	// New destination
	Vec2 newTo(arrPos.second * collisionRes + collisionRes / 2.f + CollisionMapMin.x, 
			arrPos.first * collisionRes + collisionRes / 2.f + CollisionMapMin.y);

	return std::make_pair(newTo, newTo - from);
}


void Pathfind::SetMapMinMax(const Vec2& min, const Vec2& max) {
	CollisionMapMin = min; CollisionMapMax = max;
	CollisionMapWidth = max.x - min.x; CollisionMapHeight = max.y - min.y;
	size_t arrWidth = static_cast<size_t>(CollisionMapWidth / 320) + 1;
	size_t arrHeight = static_cast<size_t>(CollisionMapHeight / 320) + 1;
	CollisionMapNormal.resize(arrWidth * arrHeight);
	CollisionMapSmall.resize(arrWidth * arrHeight * 4);
}


bool Pathfind::IsCollidable(const Vec2& pos, bool small) {
	std::pair<unsigned, unsigned> p = WorldToIndexPair(pos, (small ? 160 : 320));
	return IsCollidable(p.second, p.first, small);
}

bool Pathfind::IsCollidable(unsigned x, unsigned y, bool small) {
	if (small) {
		unsigned index = IndexPairToIndex(y, x, static_cast<unsigned int>(CollisionMapWidth / 160 + 1));
		return CollisionMapSmall[index] == -1;
	}
	else {
		unsigned index = IndexPairToIndex(y, x, static_cast<unsigned int>(CollisionMapWidth / 320 + 1));
		return CollisionMapNormal[index] == -1;
	}
}


Vec2 Pathfind::IndexPairToWorld(const std::pair<unsigned, unsigned>& p, bool center, bool small) {
	return IndexPairToWorld(p.first, p.second, center, small);
}

Vec2 Pathfind::IndexPairToWorld(unsigned y, unsigned x, bool center, bool small) {
	float offset{}, size{};
	if (small) { offset = 80.f; size = 160.f; }
	else { offset = 160.f; size = 320.f; }
	if (!center) offset = 0;

	return Vec2{ x * size + offset + CollisionMapMin.x, y * size + offset + CollisionMapMin.y};
}

std::pair<unsigned, unsigned> Pathfind::WorldToIndexPair(const Vec2& v, unsigned collisionRes) {
	// Im using [y][x] for a 2d array
	Vec2 v2 = v;
	v2.x = v2.x - CollisionMapMin.x;
	v2.y = v2.y - CollisionMapMin.y;

	v2.x = std::clamp(v2.x, 0.f, CollisionMapMax.x - CollisionMapMin.x);
	v2.y = std::clamp(v2.y, 0.f, CollisionMapMax.y - CollisionMapMin.y);

	// static casting to floor the value
	return std::make_pair(static_cast<unsigned>(v2.y / collisionRes), static_cast<unsigned>(v2.x / collisionRes));

}

unsigned Pathfind::IndexPairToIndex(const std::pair<unsigned, unsigned>& p, unsigned width) {
	return IndexPairToIndex(p.first, p.second, width);
}

unsigned Pathfind::IndexPairToIndex(unsigned t1, unsigned t2, unsigned width) {
	return t1 * width + t2;
}