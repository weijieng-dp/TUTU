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

constexpr int GRID_COLS = 7*4*2;
constexpr int GRID_ROWS = 7 * 4;
constexpr int GRID_CELL_SIZE = 640;
constexpr int MAX_ENTITIES = 2048;

class BroadPhaseGrid {
public:
    // Core API
    void Clear();
    void InsertEntities(Registry& registry); // optional full rebuild for first frame
    void UpdateEntity(Registry& registry, int entity);
    void RemoveEntity(int entity);
    void GetCandidatePairs(std::vector<std::pair<EntityRegistry::Entity, EntityRegistry::Entity>>& outPairs);
    std::array<std::bitset<MAX_ENTITIES>, GRID_ROWS>& getRowBits() { return rowBits; }
    std::array<std::bitset<MAX_ENTITIES>, GRID_COLS>& getColBits() { return colBits; }

private:
    // Bitsets representing rows and columns
    std::array<std::bitset<MAX_ENTITIES>, GRID_ROWS> rowBits;
    std::array<std::bitset<MAX_ENTITIES>, GRID_COLS> colBits;

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
