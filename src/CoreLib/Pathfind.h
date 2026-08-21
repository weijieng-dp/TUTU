/**___________________________________________________________________________/
@file          Pathfind.h
@author        j.junbo@digipen.edu
@date          9/29/2025

This does 2 things, DrawCollisionMap draws a grid map in 2 resolutions, as there
are 2 sizes for our enemies. MoveTo is a helper function to help with pathfinding

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "MathLib.h"
// ???? there was an issue with "bool small" triggering some error where it vs thinks small means char. I dont know man
#ifdef small
#undef small
#endif

class Pathfind {
public:
    /*!
    * \brief
    *    Draws out the collisionmap to be used for pathfinding
    *
    * \param [Registry&] the registry
    */
	void DrawCollisionMap(Registry& r);

    /*!
    * \brief
    *    Pathfinding algorithm
    *
    * \param [const Vec2&] The starting position
    * \param [const Vec2&] The ending position
    * \param [bool] Whether to use the small collisionmap (for the small enemies)
    *
    * \return A pair of Vec2s. First one is the new destination (if there is something blocking the path)
    * \return and the Second one is the movement direction.
    */
	std::pair<Vec2, Vec2> MoveTo(const Vec2& from, const Vec2& to, bool small = false);

    /*!
    * \brief Function to update the algorithm on where the player is
    *
    * \param [const Vec2&] Where the player is
    */
    void SetPlayerPos(const Vec2& pos) { playerPos = pos; if (!playerPosSet) playerPosSet = true; }

    /*!
    * \brief Function to update the algorithm on the player's ID
    *
    * \param [Entity] The ID
    */
    void SetPlayerID(Registry::Entity e) { playerID = e; }  

    /*!
    * \brief Function to update the algorithm on where the player is
    *
    * \param [const Vec2&] Where the player is
    */
    void SetMapMinMax(const Vec2& min, const Vec2& max);

    /*!
    * \brief Checks if a tile is collidable or not
    *
    * \param [const Vec2&] World coordinates
    * \param [bool] If the enemy is a small enemy
    */
    bool IsCollidable(const Vec2& pos, bool small = false);

    /*!
    * \brief Checks if a tile is collidable or not
    *
    * \param [unsigned] Internal coordinates. It is y,x.
    * \param [bool] If the enemy is a small enemy
    */
    bool IsCollidable(unsigned, unsigned, bool small = false);

    Vec2 GetPlayerPos() { if (playerPosSet) return playerPos; else return Vec2{}; }
    Registry::Entity GetPlayerID() { return playerID; }

    /*!
    * \brief Converts world coordinates to array pair coordinates in y,x format
    *
    * \param [const Vec2&] Where the player is
    */
    std::pair<unsigned, unsigned> WorldToIndexPair(const Vec2&, unsigned);

    /*!
    * \brief Reverse of World to index pair.
    *
    * \param [const Vec2&] Where the player is
    */  
    Vec2 IndexPairToWorld(unsigned y, unsigned x, bool center = false, bool small = false); // Reverse of WorldToIndexPair, and will give grid aligned coords
    Vec2 IndexPairToWorld(const std::pair<unsigned, unsigned>& p, bool center = false, bool small = false);

    /*!
    * \brief Turns the pair of index to a singluar number to index the array
    */
    unsigned IndexPairToIndex(const std::pair<unsigned, unsigned>&, unsigned);		// It takes in a pair of value as <y,x>
    unsigned IndexPairToIndex(unsigned, unsigned, unsigned);						// and returns the index of the CollisionMap vector
private:
	std::vector<unsigned> CollisionMapSmall;
	std::vector<unsigned> CollisionMapNormal;
    Vec2 playerPos{};
    Registry::Entity playerID{ 0 };
    bool playerPosSet{ false };
    Vec2 CollisionMapMin{ -2560.f,-2560.f };
    Vec2 CollisionMapMax{ 2560.f,2560.f };
    float CollisionMapWidth{ 5120.f };
    float CollisionMapHeight{ 5120.f };
};