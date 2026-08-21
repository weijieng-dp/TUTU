/*!
@file       TilesetManager.h
@author     Ou Yukang (yukang.ou) 100%
@date       08/01/2026
@brief		Interface for tileset object. Loading and unloading is handled
            by the ResourceManager.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include <string>
#include "Tileset.h"
#include <optional>
#include "rapidjson/document.h"
class TilesetManager
{
public:
    /*!
    * \brief
    *   Deserializes json data from filepath
    * \param
    *   filepath - path relative to asset folder
    * \return
    *   deserialized tileset
    */
    static std::optional<Tileset> DeserializeTileset(std::string const& filepath);
    /*!
    * \brief
    *   Serializes tileset into a rapidjson document
    * \param
    *   tileset - tileset to be serialized
    * \return
    *   rapidjson document containing serialized tileset
    */
    static rapidjson::Document SerializeTileset(Tileset const& tileset);
    /*!
    * \brief
    *   Converts world coordinates into chunk coordinates
    * \param
    *   tilemapTransform - transform of tilemap to get chunk coordinates
    * \param
    *   worldPos - world position to be converted
    * \return
    *   chunk coordinates of tilemap
    */
    static ChunkCoordinates GetChunkCoordinates(TransformComponent const& tilemapTransform, Vec2 worldPos);
    /*!
    * \brief
    *   Converts world coordinates into tile coordinates within chunk
    * \param
    *   tilemapTransform - transform of tilemap to get tile coordinates
    * \param
    *   worldPos - world position to be converted
    * \return
    *   tile coordinates of tilemap
    */
    static std::pair<int, int> GetTileCoordinates(TransformComponent const& tilemapTransform, Vec2 worldPos);
};