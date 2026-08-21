/*!
@file       GameobjectPoolScript.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple game object pool script that recycles the use of 1 type of prefab

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "GameobjectPoolScript.h"
void GameobjectPoolScript::OnStart(Registry & )
{
};


void GameobjectPoolScript::OnUpdate(Registry &, float , bool )
{
};

void GameobjectPoolScript::OnFixedUpdate(Registry&, float, bool)
{
}

void GameobjectPoolScript::AddPrefab(GameObject _prefab)
{
	prefabMap.insert({ _prefab.GetPrefabName(), _prefab });
}

GameObject GameobjectPoolScript::GetGameobject(const std::string& prefabName)
{
	Registry::Entity freeEntity = 0;
	Registry* reg = CEO::Get<Registry>();
	std::vector<Registry::Entity>& pool = poolMap[prefabName];

	for (size_t i = 0; i < pool.size(); ++i)
	{
		if (!reg->GetComponent<ActiveComponent>(pool[i])->isActiveSelf)
		{
			freeEntity = pool[i];
			break;
		}
	}

	if ((freeEntity != 0))
	{
		return GameObject{ freeEntity };
	}
	else// no available gameobjects found in pool
	{
		GameObject instance = prefabMap[prefabName].Instantiate();
		pool.push_back(instance.GetEntityID());
		return instance;
	}
}

