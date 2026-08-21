#pragma once
/*!
@file       GameObject.h
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
#include "Registry.h"
#include "CEO.h"
#include <queue>


class GameObject
{
public:
	using Entity = EntityRegistry::Entity;

	/*!
* \brief
*    Constructs an invalid GameObject with no associated entity or prefab.
*/
	GameObject();

	/*!
	* \brief
	*    Constructs a GameObject from a prefab name.
	*
	* \param
	*    PrefabName - The name of the prefab associated with this GameObject.
	*/
	GameObject(std::string PrefabName);

	/*!
	* \brief
	*    Constructs a GameObject from an existing entity.
	*
	* \param
	*    Entity - The entity to associate with this GameObject.
	*/
	GameObject(EntityRegistry::Entity Entity);

	//GameObject(Entity entity)
	//	: m_Entity(entity) {
	//}


	/*!
	* \brief
	*    Assigns another GameObject to this instance.
	*
	* \param
	*    GO - The GameObject to assign from.
	*
	* \return
	*    [GameObject&] Reference to this GameObject after assignment.
	*/
	GameObject& operator=(GameObject const GO)
	{
		m_Entity = GO.m_Entity;
		prefabName = GO.prefabName;
		
		return *this;
	}

	/*!
	* \brief
	*    Assigns an entity to this GameObject.
	*
	* \param
	*    GO - The entity to associate with this GameObject.
	*
	* \return
	*    [GameObject&] Reference to this GameObject after assignment.
	*/
	GameObject& operator=(EntityRegistry::Entity GO)
	{
		if (GO == 0) return *this;
		m_Entity = GO;
		prefabName.clear();
		return *this;
	}

	/*!
	* \brief
	*    Assigns a prefab name to this GameObject.
	*
	* \param
	*    GO - The prefab name to associate with this GameObject.
	*
	* \return
	*    [GameObject&] Reference to this GameObject after assignment.
	*/
	GameObject& operator=(std::string GO)
	{
		if (GO.empty()) return *this;

		m_Entity = 0;
		prefabName = GO;
		return *this;
	}


	/*!
	* \brief
	*    Destroys the GameObject instance.
	*/
	~GameObject();

	/*!
	* \brief
	*    Retrieves the entity ID associated with this GameObject.
	*
	* \return
	*    [Entity] The entity ID.
	*/
	Entity GetEntityID() const { return  m_Entity; }

	/*!
	* \brief
	*    Retrieves the prefab name associated with this GameObject.
	*
	* \return
	*    [std::string] The prefab name.
	*/
	std::string GetPrefabName() const { return prefabName; }

	/*!
	* \brief
	*    Checks whether the GameObject is valid.
	*
	* \return
	*    [bool] True if the GameObject references a valid entity or prefab.
	*/
	bool IsValid() const { return m_Entity || !prefabName.empty(); }

	// --------------------
	// Component API
	// --------------------

	/*!
	* \brief
	*    Adds a component of type T to the GameObject's entity.
	*/
	template<typename T>
	void AddComponent()
	{
		CEO::Instance().GetManager<Registry>()->AddComponent<T>(m_Entity, {});
	}

	/*!
	* \brief
	*    Retrieves a component of type T attached to the GameObject.
	*
	* \return
	*    [T*] Pointer to the component if found, otherwise nullptr.
	*/
	template<typename T>
	T* GetComponent()
	{
		return CEO::Instance().GetManager<Registry>()->GetComponent<T>(m_Entity);
	}


	/*!
	* \brief
	*    Retrieves all components of type T found in the GameObject's children.
	*
	* \return
	*    [std::vector<T*>] List of matching components in child entities.
	*/
	template<typename T>
	std::vector<T*> GetComponentsInChildren()
	{
		//out param
		std::vector<T*> entities;
		std::queue<Entity> entity;

		entity.push(m_Entity);
		while (entity.size())
		{
			Entity cur = entity.front();
			entity.pop();

			if (m_Entity != cur)
			{
				if (auto* comp = CEO::Get<Registry>()->GetComponent<T>(cur))
				{
					entities.push_back(comp);
				}

			}

			HierarchyComponnent* h = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(cur);

			Registry::Entity child = h->firstChild;
			while (child != 0)
			{
				entity.push(child);
				child = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(child)->nextSibling;
			}

		}
		return entities;

	}
	/*!
* \brief
*   Retrieves all child GameObjects in the hierarchy that contain the specified component type.
*   The search is performed using a breadth-first traversal starting from the current entity.
*
* \param null
*
* \return [std::vector<GameObject>] a list of GameObjects whose entities contain the component T
*/
	template<typename T>
	std::vector<GameObject> GetChildrenWithComponent()
	{
		//out param
		std::vector<GameObject> entities;
		std::queue<Entity> entity;

		entity.push(m_Entity);
		while (entity.size())
		{
			Entity cur = entity.front();
			entity.pop();

			if (m_Entity != cur)
			{
				if (CEO::Get<Registry>()->GetComponent<T>(cur))
				{
					entities.push_back(cur);
				}

			}

			HierarchyComponnent* h = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(cur);

			Registry::Entity child = h->firstChild;
			while (child != 0)
			{
				entity.push(child);
				child = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(child)->nextSibling;
			}

		}
		return entities;

	}

	/*!
	* \brief
	*    Retrieves the first component of type T found in the GameObject's children.
	*
	* \return
	*    [T*] Pointer to the component if found, otherwise nullptr.
	*/
	template<typename T>
	T* GetComponentInChildren()
	{
		std::queue<Entity> entity;

		entity.push(m_Entity);
		while (entity.size())
		{
			Entity cur = entity.front();
			entity.pop();

			if (m_Entity != cur)
			{
				if (auto* comp = CEO::Get<Registry>()->GetComponent<T>(cur))
				{
					return comp;
				}

			}

			HierarchyComponnent* h = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(cur);

			Registry::Entity child = h->firstChild;
			while (child != 0)
			{
				entity.push(child);
				child = CEO::Get<Registry>()->GetComponent<HierarchyComponnent>(child)->nextSibling;
			}

		}
		return nullptr;
	}

	/*!
	* \brief
	*    Removes a component of type T from the GameObject's entity.
	*/
	template<typename T>
	void RemoveComponent()
	{
		CEO::Instance().GetManager<Registry>()->RemoveComponent<T>(m_Entity);
	}

	/*!
	* \brief
	*    Sets the specified entity as a child of this GameObject.
	*
	* \param
	*    ChildEntity - The entity to assign as a child.
	*/

	void SetChild(EntityRegistry::Entity ChildEntity);

	/*!
	* \brief
	*    Sets the specified entity as the parent of this GameObject.
	*
	* \param
	*    ParentEntity - The entity to assign as a parent.
	*/
	void SetParent(EntityRegistry::Entity ParentEntity);


	/*!
	* \brief
	*    Instantiates a new GameObject based on this GameObject.
	*
	* \return
	*    [GameObject] The newly instantiated GameObject.
	*/
	GameObject Instantiate();

	/*!
	* \brief
	*    Sets whether the GameObject is active.
	*
	* \param
	*    isActive - True to activate, false to deactivate.
	*/
	void SetActive(bool isActive);

	/*!
	* \brief
	*    Destroys the GameObject and schedules it for removal.
	*/
	void Destroy();

private:
	EntityRegistry::Entity m_Entity;
	std::string prefabName;
};


class GameObjectTracker
{
public:
	/*!
	* \brief
	*    Enqueues a GameObject for deferred destruction.
	*
	* \param
	*    Go - The GameObject to destroy.
	*/
	void EnqueueDestroyGameObject(GameObject Go);

	/*!
	* \brief
	*    Destroys all GameObjects currently queued for destruction.
	*/
	void DestroyGameObjectQueue();

	/*!
	* \brief
	*    Registers a GameObject with the tracker.
	*
	* \param
	*    entity - The entity associated with the GameObject.
	* \param
	*    Go - Pointer to the GameObject instance.
	*/
	void AddGameObject(EntityRegistry::Entity entity, GameObject* Go);

	/*!
	* \brief
	*    Removes a GameObject from the tracker.
	*
	* \param
	*    entity - The entity associated with the GameObject.
	*/
	void RemoveGameObject(EntityRegistry::Entity entity);

	/*!
	* \brief
	*    Retrieves a GameObject by entity.
	*
	* \param
	*    entity - The entity associated with the GameObject.
	*
	* \return
	*    [GameObject*] Pointer to the GameObject if found, otherwise nullptr.
	*/
	GameObject* GetGameObject(EntityRegistry::Entity entity);

	/*!
	* \brief
	*    Retrieves the internal GameObject map.
	*
	* \return
	*    [std::map<EntityRegistry::Entity, GameObject*>&] Reference to the map.
	*/
	std::map<EntityRegistry::Entity, GameObject*>& GetGameObjList() { return GameObjectsList; }

private:
	std::queue<GameObject> GameobjectQueue;
	std::map<EntityRegistry::Entity, GameObject*> GameObjectsList;
};

/*!
* \brief
*    Creates a new GameObject from an entity.
*
* \param
*    newEnt - Optional entity to associate with the GameObject.
*
* \return
*    [GameObject] The created GameObject.
*/
GameObject CreateGameobject(EntityRegistry::Entity newEnt = 0);

/*!
* \brief
*    Creates a new UI GameObject from an entity.
*
* \param
*    newEnt - Optional entity to associate with the GameObject.
*
* \return
*    [GameObject] The created UI GameObject.
*/
GameObject CreateUIGameobject(EntityRegistry::Entity newEnt = 0);

/*!
* \brief
*    Destroys the specified GameObject.
*
* \param
*    go - The GameObject to destroy.
*/
void DestroyGameObject(GameObject go);