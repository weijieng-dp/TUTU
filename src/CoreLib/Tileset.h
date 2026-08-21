/*!
@file       Tileset.h
@author     Ou Yukang (yukang.ou) 100%
@date       08/01/2026
@brief		Interface for tileset object. Loading and unloading is handled
            by the ResourceManager.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include <string>

struct SpriteInfo
{
    Vec2 min{ 0.f,0.f }, max{ 1.f,1.f };
};
class Tileset{
public:
    int tileSize{32};
    std::string path{};                     // filepath
    TextureObj* texture;
    std::vector<SpriteInfo> sprites;

    Tileset();

    //dtor
    ~Tileset() = default;

    /*!
    * \brief
    *   Get file path relative to the asset folder
    * \return
    *   file path
    */
	std::string const& GetPath() { return path; }

    private:

    friend class TilesetManager;
};

struct Tile
{
    int spriteID{-1};
};

struct ChunkCoordinates
{
    int chunkX, chunkY;

    bool operator<(ChunkCoordinates const& rhs) const
    {
        if (chunkX < rhs.chunkX)
            return true;
        else if (chunkX == rhs.chunkX)
            return chunkY < rhs.chunkY;
    
        return false;
    }
};