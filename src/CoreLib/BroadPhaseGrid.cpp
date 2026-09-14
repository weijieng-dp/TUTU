/*!
@file       BroadPhaseGrid.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       25/09/2025

This file implements the [BroadPhaseGrid] system used for broad-phase
collision detection. It updates grid cell occupancy for entities, clears
bitsets each frame, and generates candidate pairs based on overlapping
row and column bitsets for efficient narrow-phase testing.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "BroadPhaseGrid.h"

// Hash helper for storing unique pairs
struct PairHash {

    std::size_t operator()(const std::pair<int, int>& p) const noexcept {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

// Clear all state
void BroadPhaseGrid::Clear(){

    for (auto& row : rowBits) row.second.reset();
    for (auto& col : colBits) col.second.reset();
    entityBounds.clear();
}

// Convert world-space position to grid cell index
std::pair<int, int> BroadPhaseGrid::WorldToCell(const Vec2& pos){

    // bottom-left of the grid tile starts from 0, 0
    // the offset helps shift the center of 0, 0 grid to 0, 0
    int col = static_cast<int>((pos.x - GRID_CELL_SIZE / 2) / GRID_CELL_SIZE);
    int row = static_cast<int>((pos.y - GRID_CELL_SIZE / 2) / GRID_CELL_SIZE);


    //int col = static_cast<int>((pos.x + (GRID_COLS / 2) * GRID_CELL_SIZE) / GRID_CELL_SIZE);
    //int row = static_cast<int>((pos.y) / GRID_CELL_SIZE);
    //col = std::clamp(col, 0, GRID_COLS - 1);
    //row = std::clamp(row, 0, GRID_ROWS - 1);
    return { col, row };
}

// Full rebuild (used only on first frame or mass reset)
void BroadPhaseGrid::InsertEntities(Registry& registry){

    Clear();

    auto entities = registry.GetEntitiesWithComponents<TransformComponent, CollisionComponent>();
    for (auto e : entities) {
        UpdateEntity(registry, e);
    }
}

// Incremental update of one entity's row/column occupancy
void BroadPhaseGrid::UpdateEntity(Registry& registry, int e){

    auto* collisionComp = registry.GetComponent<CollisionComponent>(e);
    if (!collisionComp) return;

    Vec2 min = collisionComp->worldMin;
    Vec2 max = collisionComp->worldMax;

    auto [minCol, minRow] = WorldToCell(min);
    auto [maxCol, maxRow] = WorldToCell(max);

    // Remove previous bits if they exist

    auto it = entityBounds.find(e);
    if (it != entityBounds.end()) {
        for (int y = it->second.minRow; y <= it->second.maxRow; ++y) {
            rowBits[y].reset(e);
        }
        for (int x = it->second.minCol; x <= it->second.maxCol; ++x) {
            colBits[x].reset(e);
        }
        it->second = { minRow, maxRow, minCol, maxCol };
    }
    else {
        rowBits.insert({ e , std::bitset<MAX_ENTITIES>{} });
        colBits.insert({ e , std::bitset<MAX_ENTITIES>{} });
        
        entityBounds.insert({ e , { minRow, maxRow, minCol, maxCol } });
    }

    // Set new bits
    for (int y = minRow; y <= maxRow; ++y){
        rowBits[y].set(e);
    }
    for (int x = minCol; x <= maxCol; ++x){
        colBits[x].set(e);
    }
}

// Remove an entity from the grid
void BroadPhaseGrid::RemoveEntity(int e){

    auto it = entityBounds.find(e);
    if (it == entityBounds.end()) return;

    for (int y = it->second.minRow; y <= it->second.maxRow; ++y){
        rowBits[y].reset(e);
    }
    for (int x = it->second.minCol; x <= it->second.maxCol; ++x){
        colBits[x].reset(e);
    }

    entityBounds.erase(it);
}
#define UNUSED_PARAMETER(x) (void)(x)
// Broad-phase candidate pair generation
void BroadPhaseGrid::GetCandidatePairs(std::vector<std::pair<EntityRegistry::Entity, EntityRegistry::Entity>>& outPairs){
    auto const & physStorage = CEO::Get<Registry>()->GetStorage<PhysicsComponent>();
    auto const& hierStorage = CEO::Get<Registry>()->GetStorage<HierarchyComponnent>();
    outPairs.clear();
    //std::unordered_set<std::pair<int, int>, PairHash> uniquePairs;

    auto findPhysicsComp = [&](EntityRegistry::Entity ent)->bool {
        EntityRegistry::Entity currEnt = ent;
        do {
            if(physStorage->Get(currEnt)) return true;
            currEnt = hierStorage->Get(currEnt)->parent;
        } while (hierStorage->Get(currEnt));
        return false;
        };

    for (auto& row: rowBits) {

        if (!row.second.any()) continue; // skip empty rows

        for (auto& col: colBits) {

            if (!col.second.any()) continue; // skip empty columns

            std::bitset<MAX_ENTITIES> intersection = row.second & col.second;
            if (!intersection.any()) continue; // skip empty intersections

            // Collect entities in this intersection
            std::vector<int> entities;
            entities.reserve(16);
            for (int i = 0; i < MAX_ENTITIES; ++i){
                if (intersection.test(i)){
                    entities.push_back(i);
                }
            }

            // Add unique candidate pairs
            for (size_t i = 0; i < entities.size(); ++i){
                for (size_t j = i + 1; j < entities.size(); ++j){
                    int a = entities[i], b = entities[j];
                    if (a > b) std::swap(a, b);
                    if (findPhysicsComp(a) || findPhysicsComp(b))
                        outPairs.emplace_back(a, b);
                }
            }
        }
    }

    // Copy unique pairs to output
    //outPairs.reserve(uniquePairs.size());
    //for (const auto& aUniqPair : uniquePairs){
    //    outPairs.emplace_back(aUniqPair.first, aUniqPair.second);
    //}
}
#undef UNUSED_PARAMETER

