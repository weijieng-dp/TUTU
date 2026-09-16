/*!
@file       BroadPhaseGrid.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file declares the [BroadPhaseGrid] class, which implements a spatial
partitioning grid used to accelerate collision detection by dividing the
world into fixed-size cells. It maintains row and column bitsets to
efficiently track entity positions and generate candidate collision pairs.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "Components.h"
#include <bitset>
#include <vector>
#include <array>
#include <unordered_map>

constexpr int GRID_CELL_SIZE = 2080 + 320 + 160;
constexpr int MAX_ENTITIES = 2048;

class BroadPhaseGrid {
public:
    // Core API
    void Clear();
    void InsertEntities(Registry& registry); // optional full rebuild for first frame
    void UpdateEntity(Registry& registry, int entity);
    void RemoveEntity(int entity);
    void GetCandidatePairs(std::vector<std::pair<EntityRegistry::Entity, EntityRegistry::Entity>>& outPairs);
    std::unordered_map<int, std::bitset<MAX_ENTITIES>>& getRowBits() { return rowBits; }
    std::unordered_map<int, std::bitset<MAX_ENTITIES>>& getColBits() { return colBits; }

private:
    // Bitsets representing rows and columns
    std::unordered_map<int, std::bitset<MAX_ENTITIES>> rowBits;
    std::unordered_map<int, std::bitset<MAX_ENTITIES>> colBits;

    // Stores which rows/cols each entity currently occupies
    struct EntityGridBounds {
        int minRow, maxRow;
        int minCol, maxCol;
    };
    std::unordered_map<int, EntityGridBounds> entityBounds;

    // Internal helpers
    std::pair<int, int> WorldToCell(const Vec2& pos);

    bool broadGridDebugEnabled{ false };
};
