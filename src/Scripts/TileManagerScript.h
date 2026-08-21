/**___________________________________________________________________________/
@file          TileManagerScript.h
@author        Tan Jun Jie (t.junjie) (100%)
@date          03/02/2026 (DD/MM/YYYY)
@brief         Handles drag-and-drop of tiles in Map.scene.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/MathLib.h"
#include "../CoreLib/GameObjects.h"

class TileManagerScript : public ScriptInstance {
public:
    void BindFrom() {
        GetComponent<TileManagerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<TileManagerScript>(*CEO::Get<Registry>());
    };
    void BindTo() { *GetComponent<TileManagerScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief Translate from map grid coordinates to screen coordinates.
    * \param[in] x - Grid x coordinates to translate.
    * \param[in] y - Grid y coordinates to translate.
    * \return The translated screen coordiantes.
    */
    Vec2 TranslateToScreenCoordinates(int x, int y);

    /*!
    * \brief Translate from screen coordinates to map grid coordinates.
    * \param[in] screenPos - Screen x-y coordinates.
    * \return The translated grid coordinates.
    */
    std::pair<int, int> TranslateToGridAnchor(const Vec2& screenPos);

    /*!
    * \brief Spawn in all the tile instances on map grid.
    * \param[in] registry - The entity registry.
    * \param[in] transform   - The preset UI transform component to set for all tile instances.
    * \param[in] layer       - Render priority to set for all tile instances.
    */
    void SpawnTileInstances(Registry& registry, UITransformComponent&& transform, unsigned layer);

    /*!
    * \brief Help set flags and spawns the drag-and-drop tile.
    * \param[in] index      - The tile choice index.
    */
    bool BeginPlacement(int index);

    /*!
    * \brief Spawns the dragging tile that follows player's input.
    * \return The GameObject of the spawned tile.
    */
    GameObject SpawnPreviewEntity();
    /*!
    * \brief Spawns the dragging tile that follows player's input at specified position.
    * \param[in] startPos - The starting position to spawn the dragging tile.
    * \return The GameObject of the spawned tile.
    */
    GameObject SpawnPreviewEntity(const Vec2& startPos);

    /*!
    * \brief Handles when drag-and-drop map tile placement succeeds.
    * \param[in] registry   - Entity registry.
    * \param[in] index      - The tile instance index after successful placement.
    */
    void ConfirmPlacement(Registry& registry, int index);

    /*!
    * \brief Handles when drag-and-drop map tile placement fails.
    * \param[in] registry   - Entity registry.
    * \param[in] reset      - Whether placement was fully reset
    */
    void CancelPlacement(Registry& registry, bool reset);

    REFLECTABLE_PROPERTIES;
    std::pair<int, int> savedGridPos{ -1, -1 }; // the grid position of the placed tile (for resetting)

    GameObject previewTileEntity{};     // the preview tile that follows user input upon selection
    GameObject placedTile{};            // the tile placed in this scene
    int placedTileIndex{ -1 };          // the tile instance index of the placed tile

    int selectedOfferIndex{ -1 }, lastSelectedOfferIndex{ -1 };

    bool placed{ false };               // track whether map tile has already been placed
    bool placing{ false };              // whether map tile is currently being placed
private:
    Vec2 bottomLeft{}, grabOffset{};
    EntityRegistry::Entity startTile{}, endTile{}, treasureTile{};  // track the special tiles entity id
    EntityRegistry::Entity mapButtonManagerScriptEnt{};             // cache the entity with mapbuttonmanager script

    std::pair<int, int> ControllerCoord;
    float inputCooldown = 0.2f;
    float increment{ 90.f };
};
REFL_AUTO(
    type(TileManagerScript)
)