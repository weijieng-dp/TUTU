/**___________________________________________________________________________/
@file           MapSpawner.h
@author         Tan Jun Jie (t.junjie) (100%)
@date           03/02/2026 (DD/MM/YYYY)
@brief          Script that calls SpawnMapTiles() function to spawn in all map
                tiles on entry to Game.scene.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class MapSpawner: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<MapSpawner>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<MapSpawner>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<MapSpawner>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(MapSpawner)
)