/*!
@file       GameObject.cpp
@author      Ng Wei Jie (weijie.ng) 100%
@date       3/02/2026
@brief
			Declares the GameObject and GameObjectTracker classes, which provide
			a high-level interface for entity manipulation, component access,
			hierarchy management, and controlled destruction within the ECS.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*______________________________________________________________________*/

#include "pch.h"
#include "SceneManager.h"
#include "Components.h"
#include "UpdateStackManager.h"

GameObject::GameObject() : m_Entity(0), prefabName{}{};
GameObject::GameObject(std::string PrefabName) : m_Entity(0), prefabName(PrefabName) {};
GameObject::GameObject(EntityRegistry::Entity Entity) : m_Entity(Entity), prefabName() {
};

GameObject::~GameObject()
{
}

GameObject CreateGameobject(EntityRegistry::Entity newEnt)
{
	GameObject go;
	if(newEnt == 0)
		newEnt = CEO::Get<Registry>()->CreateEntity();
	go = newEnt;
	go.AddComponent<HierarchyComponnent>();
	go.AddComponent<ActiveComponent>();
	go.AddComponent<NameComponent>();
	go.AddComponent<LayerComponent>();
	go.AddComponent<TransformComponent>();
	go.AddComponent<UpdateStackComponent>();
	CEO::Get<Registry>()->GetComponent<UpdateStackComponent>(go.GetEntityID())->stack = CEO::Get<UpdateStackManager>()->GetStack();

	CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ newEnt, go.GetComponent<LayerComponent>()->layer, Events::EntityModified::MODIFICATION::ADD_ENTITY });
	return go;
}

void DestroyGameObject(GameObject go)
{
	CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ go.GetEntityID(), go.GetComponent<LayerComponent>()->layer, Events::EntityModified::MODIFICATION::REMOVE_ENTITY });

	go.Destroy();
}


GameObject GameObject::Instantiate()
{

	if (!IsValid())
	{
		//you forgot to put entity or prefab Bruh!!!!
		DEBUGBREAK;
		
		return GameObject();
	}
	if (!m_Entity)
	{
		GameObject go{ CEO::Get<ResourceManager>()->InstantiatePrefab(*CEO::Get<Registry>(), prefabName) };
		CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ go.GetEntityID(), go.GetComponent<LayerComponent>()->layer, Events::EntityModified::MODIFICATION::ADD_ENTITY});
		return go;

	}
	else
	{




		Registry::Entity newEnt = CEO::Get<Registry>()->CreateEntity();
		SceneManager::CopyEntity(*CEO::Get<Registry>(), m_Entity,newEnt);
		GameObject go{ newEnt };
		CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ newEnt, go.GetComponent<LayerComponent>()->layer, Events::EntityModified::MODIFICATION::ADD_ENTITY });
		return go;
	}
}
// --------------------
// Lifetime
// --------------------
void GameObject::Destroy()
{
	if (m_Entity != 0)
	{
		if(!CEO::Instance().GetManager<HierarchyManager>()->ReparentToRoot(m_Entity)) return;

		Registry* reg = CEO::Instance().GetManager<Registry>();
		EntityRegistry::Entity cur = m_Entity;
		while (cur != 0)
		{
			auto* h = reg->GetComponent<HierarchyComponnent>(cur);

			if (h->firstChild != 0)
			{
				// go down
				cur = h->firstChild;
			}
			else
			{
				// save data BEFORE deletion
				EntityRegistry::Entity next = h->nextSibling;
				EntityRegistry::Entity parent = h->parent;

				auto Native = reg->GetComponent<NativeScriptingComponent>(cur);
				if (Native)
				{
					for (auto& [name, Instance] : Native->scripts)
					{
						if (Instance.Instance)
						{
							Instance.OnDeleteScript(Instance);
						}
					}
					Native->scripts.clear();
				}
				
				reg->DestroyEntity(cur);

				if (next != 0)
				{
					cur = next;
				}
				else
				{
					cur = parent;
					if (parent != 0)
						reg->GetComponent<HierarchyComponnent>(parent)->firstChild = 0;
				}
			}
		}

		reg->DestroyEntity(m_Entity);
	}
}


void GameObject::SetActive(bool isActive)
{
	if (!IsValid())
	{
		//you forgot to put entity BRUH!!!
		DEBUGBREAK;
		return;
	}
	else if (!prefabName.empty())
	{
		//Why are u setting Active for prefab BRUH!!!
		DEBUGBREAK;
		return;
	}
	CEO::Instance().GetManager<Registry>()->GetComponent<ActiveComponent>(m_Entity)->isActiveSelf = isActive;
}

void GameObject::SetChild(EntityRegistry::Entity ChildEntity)
{
	CEO::Get<HierarchyManager>()->Reparent(ChildEntity, m_Entity);
}
void GameObject::SetParent(EntityRegistry::Entity ParentEntity)
{
	CEO::Get<HierarchyManager>()->Reparent(m_Entity, ParentEntity);

}

GameObject CreateUIGameobject(EntityRegistry::Entity newEnt)
{
	GameObject go;
	if (newEnt == 0)
		newEnt = CEO::Get<Registry>()->CreateEntity();
	go = newEnt;
	go.AddComponent<UITransformComponent>();
	go.AddComponent<NameComponent>();
	go.AddComponent<HierarchyComponnent>();
	go.AddComponent<ActiveComponent>();
	go.AddComponent<LayerComponent>();
	go.AddComponent<UpdateStackComponent>();
	CEO::Get<Registry>()->GetComponent<UpdateStackComponent>(go.GetEntityID())->stack = CEO::Get<UpdateStackManager>()->GetStack();

	CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ newEnt, go.GetComponent<LayerComponent>()->layer, Events::EntityModified::MODIFICATION::ADD_ENTITY });
	return go;
}


void GameObjectTracker::EnqueueDestroyGameObject(GameObject Go)
{
	if (!Go.GetEntityID())
	{

		LOGE("bruh u trying to destroy prefabs");
		return;
	}
	GameobjectQueue.push(Go);
}

void GameObjectTracker::DestroyGameObjectQueue()
{
	while (GameobjectQueue.size())
	{
		GameobjectQueue.front().Destroy();
		GameobjectQueue.pop();
	}
}

void GameObjectTracker::AddGameObject(EntityRegistry::Entity entity, GameObject* Go)
{
	GameObjectsList[entity] = Go;
}

void GameObjectTracker::RemoveGameObject(EntityRegistry::Entity entity)
{
	GameObjectsList.erase(entity);
}

GameObject* GameObjectTracker::GetGameObject(EntityRegistry::Entity entity)
{
	return GameObjectsList[entity];
}

