/**___________________________________________________________________________/
@file           MapSpawner.cpp
@author         Tan Jun Jie (t.junjie) (100%)
@date           03/02/2026 (DD/MM/YYYY)
@brief          Script that calls SpawnMapTiles() function to spawn in all map
				tiles on entry to Game.scene.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "MapSpawner.h"
#include "../CoreLib/MapManager.h"
#include "../CoreLib/HierarchyManager.h"

void MapSpawner::OnStart(Registry & )
{
	// calls SpawnMapTiles on the start of the script to spawn all the map tiles into Game.scene
	CEO::Get<MapManager>()->SpawnMapTiles();
};


void MapSpawner::OnUpdate(Registry & , float , bool )
{
};

void MapSpawner::OnFixedUpdate(Registry&, float, bool)
{
};
