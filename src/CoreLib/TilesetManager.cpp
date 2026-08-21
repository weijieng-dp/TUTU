/*!
@file       TilesetManager.h
@author     Ou Yukang (yukang.ou) 100%
@date       08/01/2026
@brief		Interface for tileset object. Loading and unloading is handled
            by the ResourceManager.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "TilesetManager.h"
#include "rapidjson/document.h"
#include "ResourceManager.h"
#include "CEO.h"
#undef GetObject

std::optional<Tileset> TilesetManager::DeserializeTileset(std::string const& filePath)
{
    // load json data
     std::stringstream sstr = CEO::Instance().GetManager<FileManager>()->ReadFile(filePath, false, std::ios_base::binary | std::ios_base::in);
     if (sstr.str().empty())
         return std::nullopt;
     rapidjson::Document doc;
     doc.Parse(sstr.str());
     Tileset tileset;

     // load texture associated with tileset
     if (doc.HasMember("texture") && doc["texture"].IsString())
     {
         tileset.texture = &CEO::Get<ResourceManager>()->GetTexture(doc["texture"].GetString());
     }
     else // no texture loaded, error
         return std::nullopt;

     // load tile data
     for (rapidjson::Value::ConstValueIterator itr = doc["sprites"].Begin(); itr != doc["sprites"].End(); ++itr)
     {
         SpriteInfo info;
         info.min = Vec2{ itr->GetObject()["min"].GetArray()[0].GetFloat(),itr->GetObject()["min"].GetArray()[1].GetFloat()};
         info.max = Vec2{ itr->GetObject()["max"].GetArray()[0].GetFloat(),itr->GetObject()["max"].GetArray()[1].GetFloat()};
		 tileset.sprites.push_back(info);
     }

     tileset.path = filePath;
	 return tileset;
}

rapidjson::Document TilesetManager::SerializeTileset(Tileset const& tileset)
{
    // create new json document
    rapidjson::Document json;

    // serialize tile size
    json.SetObject();
    json.AddMember("tileSize", tileset.tileSize, json.GetAllocator());

    // serialize associated texture
    std::string texPath = CEO::Get<ResourceManager>()->GetTexPath(*tileset.texture);
    if (texPath != "ERROR")
        texPath = texPath.substr(texPath.find("Assets\\Textures\\") + std::string("Assets\\Textures\\").size());
    else
        texPath = "";
    json.AddMember("texture", texPath, json.GetAllocator());

    // serialize sprites (subtextures used by tile)
    rapidjson::Value sprites;
    sprites.SetArray();
    for (SpriteInfo const& sprite : tileset.sprites)
    {
        rapidjson::Value spriteValue;
        rapidjson::Value min;
        rapidjson::Value max;
        spriteValue.SetObject();
        min.SetArray();
        max.SetArray();
        min.PushBack(sprite.min.x, json.GetAllocator());
        min.PushBack(sprite.min.y, json.GetAllocator());
        max.PushBack(sprite.max.x, json.GetAllocator());
        max.PushBack(sprite.max.y, json.GetAllocator());
        spriteValue.AddMember("min", min, json.GetAllocator());
        spriteValue.AddMember("max", max, json.GetAllocator());
        sprites.PushBack(spriteValue.Move(), json.GetAllocator());
    }

    json.AddMember("sprites", sprites, json.GetAllocator());

    return json;
}

ChunkCoordinates TilesetManager::GetChunkCoordinates(TransformComponent const& tilemapTransform, Vec2 worldPos)
{
    // use integer coordintaes for calculation
    // get relative position to tilemap origin
    float posX = worldPos.x - tilemapTransform.translate.x;
    float posY = worldPos.y - tilemapTransform.translate.y;

    // normalize position
    posX /= tilemapTransform.scale.x;
    posY /= tilemapTransform.scale.y;

    int row = static_cast<int>(posY) / TilemapComponent::CHUNK_SIZE;
    int col = static_cast<int>(posX) / TilemapComponent::CHUNK_SIZE;

    // if row/col is negative, flip the value
    if (posY < 0)
        row = -(abs(row) + 1);
    if (posX < 0)
        col = -(abs(col) + 1);

    //assert(col >= 0 && col < TilemapComponent::CHUNK_SIZE && row >= 0 && row < 16);
    //LOGI("Chunk coords: %d, %d", row, col);
    return ChunkCoordinates{ col,row};
}

std::pair<int,int> TilesetManager::GetTileCoordinates(TransformComponent const& tilemapTransform, Vec2 worldPos)
{
    // world to relative position (tilemap)
    float posX = worldPos.x - tilemapTransform.translate.x; 
    float posY = worldPos.y - tilemapTransform.translate.y;

    // normalize position
    posX /= tilemapTransform.scale.x;
    posY /= tilemapTransform.scale.y;
    
    // get tile coordinates
    int col;
    int row;
	if (posX > 0)
        col = abs(static_cast<int>(posX)) % TilemapComponent::CHUNK_SIZE;
    else
    {
        col = static_cast<int>(posX) % TilemapComponent::CHUNK_SIZE -1;
        col = TilemapComponent::CHUNK_SIZE + col;
    }
    if (posY > 0)
        row = abs(static_cast<int>(posY)) % TilemapComponent::CHUNK_SIZE;
    else
    {
        row = static_cast<int>(posY) % TilemapComponent::CHUNK_SIZE - 1;
        row = TilemapComponent::CHUNK_SIZE + row;
    }

    assert(col >= 0 && col < TilemapComponent::CHUNK_SIZE && row >= 0 && row < 16);
    //LOGI("Tile coords: %d, %d", row, col);
    return std::make_pair(col, row);
}
