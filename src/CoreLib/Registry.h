/**___________________________________________________________________________/
@file          Registry.h
@author        Ng Wei Jie (weijie.ng) (100%)
@date          01/10/2025

This file defines the core ECS (Entity Component System) registry framework.
It includes the following:
- `EntityRegistry` for creating and tracking entity IDs.
- `IComponentStorage` and `ComponentStorage<T>` for managing component lifetimes.
- `Registry` as the main ECS manager for entities and components.
- `ComponentRegistry` for reflection-based component creation and removal.

The system supports component queries, runtime reflection integration, and
efficient storage using sparse–dense sets.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/

#pragma once
#include <vector>
#include <string>
#include "unordered_set"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <iostream>
#include <map>
#include <limits>
#include "Platform.h"
//#include "rttr/type.h"

#include "EventsDispatcher.h"

#include "RuntimeReflect.h"

#define NULL_INDEX std::numeric_limits<uint32_t>::max()

// =====================================
// =      Start of EntityRegistry      =
// =====================================
/*!
* \brief
*    Manages the creation, destruction, and tracking of entity IDs.
*
* \brief
*    - Entity IDs are unique integers.
*    - Supports resetting and retrieving all active entities.
*/
class  EntityRegistry
{
public:
	using Entity = uint32_t;

	/*!
	* \brief
	*    Create a new unique entity ID.
	*
	* \return
	*    [Entity] Newly created entity ID.
	*/
	Entity CreateEntityID();


	/*!
	 * \brief
	 *    Create an entity with a given explicit ID.
	 *
	 * \param
	 *    [int] ID - Explicit ID to assign.
	 *
	 * \return
	 *    [Entity] The entity created with the provided ID.
	 */
	Entity CreateEntityID(EntityRegistry::Entity ID);


	/*!
	* \brief
	*    Reset the entity counter to start from zero.
	*/
	void RestartEntityCount();

	/*!
	* \brief
	*    Destroy a specific entity, removing it from the registry.
	*
	* \param
	*    [Entity] entity - The entity to be destroyed.
	*/
	void DestroyEntity(Entity entity);


	/*!
	* \brief
	*    Add an entity to the registry.
	*
	* \param
	*    [Entity] entity - The entity to add.
	*/
	void AddEntity(Entity entity);



	/*!
	* \brief
	*    Retrieve all entities currently in the registry.
	*
	* \return
	*    [std::vector<Entity>] Vector of entity IDs.
	*/
	std::vector<Entity> GetAllEntity();

	bool IsEntityValid(Entity ent);

private:
	Entity nextId = 0;
	std::unordered_set<Entity> allEntities;
};

// =====================================
// =     Start of ComponentStorage     =
// =====================================
/*!
* \brief
*    Base interface for type-erased component storage.
*
* \details
*    Allows removal, reflection access, and clearing without knowing the
*    underlying component type.
*/
class IComponentStorage {
public:
	virtual ~IComponentStorage() = default;

	/*!
	* \brief
	*    Remove a component associated with a given entity.
	*
	* \param
	*    [EntityRegistry::Entity] entity - Entity whose component will be removed.
	*/

	virtual void Remove(EntityRegistry::Entity entity) = 0;

	/*!
	* \brief
	*    Get the runtime reflection instance of a component tied to an entity.
	*
	* \param
	*    [EntityRegistry::Entity] entity - Entity whose component is requested.
	*
	* \return
	*    [rtr::Instance] Reflection instance of the component, or empty if not found.
	*/
	virtual rtr::Instance GetCompFromEntity(EntityRegistry::Entity entity) = 0;

	/*!
	* \brief
	*    Clear all components from this storage.
	*/
	virtual void Clear() = 0;

};

// =====================================
// =  Template Component Storage Impl  =
// =====================================
/*!
* \brief
*    Sparse–dense set storage for a specific component type.
*
* \tparam Component The type of component stored.
*/
template <typename Component>
class ComponentStorage : public IComponentStorage {
public:
	/*!
	* \brief
	*    Add or update a component for an entity.
	*
	* \param
	*    [EntityRegistry::Entity] entity - Target entity.
	* \param
	*    [Component] component - Component to add or update.
	*/
	void Add(EntityRegistry::Entity entity, Component component)
	{
		//components[entity] = std::move(component);
		if (entity >= sparseSet.size())
		{
			sparseSet.resize(entity + 1, NULL_INDEX);
		}

		if (sparseSet[entity - 1] != NULL_INDEX) {
			// Update existing component
			denseSet[sparseSet[entity - 1]] = std::make_unique<Component>(component);
			return;
		}

		sparseSet[entity - 1] = static_cast<EntityRegistry::Entity>(denseSet.size());
		denseSet.emplace_back(std::make_unique<Component>(component));
	}


	/*!
	* \brief
	*    Remove a component associated with an entity.
	*
	* \param
	*    [EntityRegistry::Entity] entity - Entity whose component will be removed.
	*/
	void Remove(EntityRegistry::Entity entity) override
	{
		// Safe bounds checking (no -1)
		if (entity >= sparseSet.size() || sparseSet[entity - 1] == NULL_INDEX) {
			return;
		}

		EntityRegistry::Entity removed_index = sparseSet[entity - 1];
		EntityRegistry::Entity last_entity = static_cast<EntityRegistry::Entity>(denseSet.size() - 1);

		// If not removing the last element, perform swap
		if (removed_index != last_entity) {
			// Move last component to removed position
			denseSet[removed_index] = std::move(denseSet.back());

			// Update the entity that previously pointed to the last element
			for (EntityRegistry::Entity ent = 0; ent < sparseSet.size(); ++ent) {
				if (sparseSet[ent] == last_entity) {
					sparseSet[ent] = removed_index;
					break;
				}
			}
		}

		// Remove the last element
		denseSet.pop_back();
		sparseSet[entity - 1] = NULL_INDEX;
	}




	/*!
	* \brief
	*    Get the component of a given entity.
	*
	* \param
	*    [EntityRegistry::Entity] entity - Entity to query.
	*
	* \return
	*    [Component*] Pointer to the component, or nullptr if none exists.
	*/
	Component* Get(EntityRegistry::Entity entity)
	{
		if (entity >= sparseSet.size()) return nullptr;
		if (entity == 0) return nullptr;
		EntityRegistry::Entity index = sparseSet.at(entity - 1);
		if (index != NULL_INDEX) {
			return denseSet.at(index).get();
		}
		return nullptr;
	}


	 /*!
    * \brief
    *    Retrieve reflection instance of the component tied to an entity.
    *
    * \param
    *    [EntityRegistry::Entity] entity - Entity to query.
    *
    * \return
    *    [rtr::Instance] Reflection instance of the component, or empty if not found.
    */
	rtr::Instance GetCompFromEntity(EntityRegistry::Entity entity) override
	{
		if (entity >= sparseSet.size()) return rtr::Instance();

		EntityRegistry::Entity index = sparseSet[entity - 1];
		if (index != NULL_INDEX) {
			return denseSet[index]->GetInstance();
		}
		return rtr::Instance();
	}

	/*!
	  * \brief
	  *    Get a list of all entities that own this component type.
	  *
	  * \return
	  *    [std::vector<EntityRegistry::Entity>] Vector of entity IDs.
	  */
	std::vector<EntityRegistry::Entity> GetEntities() const
	{
		std::vector<EntityRegistry::Entity> result;

		for (int i = 0; i < sparseSet.size(); i++)
		{
			if (sparseSet[i] != NULL_INDEX)
			{
				result.push_back(i + 1);
			}
		}
		return result;
	}

	/*!
	* \brief
	*    Clear all components from this storage.
	*/
	void Clear() override {
		denseSet.clear();
		sparseSet.clear();
	}

private:

	std::vector<EntityRegistry::Entity> sparseSet;
	std::vector<std::unique_ptr<Component>> denseSet;
};

// =====================================
// =            Registry               =
// =====================================
/*!
* \brief
*    Main ECS registry that manages entities and their components.
*
* \details
*    Provides entity lifecycle management, component storage, queries, and
*    reflection-based component access.
*/

class Registry {
public:
	// Singleton Instance
	Registry() = default;
	Registry(Registry&&) noexcept = default;
	Registry& operator=(Registry&&) noexcept = default;

	using Entity = EntityRegistry::Entity;


	// ===== ENTITY SYSTEM =====

	 /*!
	 * \brief
	 *    Reset the entity counter in the registry.
	 */
	void RestartEntityCount();

	/*!
	  * \brief
	  *    Create a new entity.
	  *
	  * \return
	  *    [Entity] Newly created entity ID.
	  */
	Entity CreateEntity();

	/*!
	* \brief
	*    Create an entity with a specific ID.
	*
	* \param
	*    [int] ID - Explicit ID to assign.
	*
	* \return
	*    [Entity] The entity created with the given ID.
	*/
	Entity CreateEntity(int ID);

	/*!
	* \brief
	*    Destroy an entity and its components.
	*
	* \param
	*    [Entity] entity - Entity to destroy.
	*/
	void DestroyEntity(Entity entity);

	/*!
	* \brief
	*    Destroy all entities in the registry.
	*/
	void DestroyAllEntities();

	/*!
	* \brief
	*    Get all entities currently in the registry.
	*
	* \return
	*    [std::vector<Entity>] Vector of entity IDs.
	*/
	std::vector<Entity> GetAllEntity();

	/*!
	* \brief
	*    Check if entity exists in registry
	*
	* \return
	*    If entity exists in registry
	*/
	bool IsEntityValid(Entity ent);


	// ===== COMPONENT MANAGEMENT =====

	/*!
	* \brief
	*    Add a component to an entity.
	*
	* \tparam Component The component type.
	* \param entity Target entity.
	* \param component Component to add.
	*/	
	template <typename Component>
	void AddComponent(Entity entity, Component component) {
		GetStorage<Component>()->Add(entity, std::move(component));
	}


	template <typename Component>
	std::vector<Component>& GetDataComponent() {
		return GetStorage<Component>()->GetDataSet();
	}

	/*!
	* \brief
	*    Get a pointer to a component belonging to an entity.
	*
	* \tparam Component The component type.
	* \param entity Entity to query.
	*
	* \return
	*    [Component*] Pointer to the component, or nullptr if not found.
	*/
	template <typename Component>
	Component* GetComponent(Entity entity) {
		return GetStorage<Component>()->Get(entity);
	}


	template <typename Component>
	Component& TryGetComponent(Entity entity)
	{
		Component* comp = GetComponent<Component>(entity);
		if (comp)
			return *comp;
		static Component dummy{};
		LOGE("Missing Component: %s On Entity %d", typeid(Component).name(), entity);
		return dummy;
	}
    /*!
    * \brief
    *    Remove a component from an entity.
    *
    * \tparam Component The component type.
    * \param entity Entity to modify.
    */
	template <typename Component>
	void RemoveComponent(Entity entity) {
		GetStorage<Component>()->Remove(entity);
	}

	/*!
	* \brief
	*    Get all components of an entity as reflection instances.
	*
	* \param
	*    [Entity] entity - Entity to query.
	*
	* \return
	*    [std::vector<rtr::Instance>] Vector of reflection instances.
	*/
	std::vector<rtr::Instance> GetAllComponentsRTR(Entity entity);

	/*!
	* \brief
	*    Get a specific component of an entity by reflection type.
	*
	* \param
	*    [Entity] entity - Entity to query.
	* \param
	*    [rtr::TypeInfo] type - Type information of the component.
	*
	* \return
	*    [rtr::Instance] Reflection instance of the component.
	*/
	rtr::Instance GetComponentsRTR(Entity entity, rtr::TypeInfo type);

	/*!
	* \brief
	*    Remove a component from an entity using runtime type info.
	*
	* \param
	*    [Entity] ent - Entity to modify.
	* \param
	*    [rtr::TypeInfo] type - Type information of the component.
	*/
	void RemoveComponent(Registry::Entity ent, rtr::TypeInfo type);

	/*!
	* \brief
	*    Retrieves all components on an entity that contain a member of type T.
	*
	* \param
	*    [Entity] entity - Entity whose components will be inspected.
	*
	* \return
	*    Vector of runtime instances representing components that have
	*    at least one member of type T.
	*/
	template <typename T>
	std::vector<rtr::Instance> GetComponentsWithMember(Entity entity) {
        std::vector<rtr::Instance> outVec{ GetAllComponentsRTR(entity) };
		outVec.erase(std::remove_if(outVec.begin(), outVec.end(),
			[](rtr::Instance& inst) {
				return inst.GetType().GetMembersWithType<T>().empty();
			}), 
			outVec.end());
	}

	// ===== QUERYING ENTITIES =====

	/*!
	* \brief
	*    Check if an entity has a component of a given type.
	*
	* \tparam Component The component type.
	* \param entity Entity to query.
	*
	* \return
	*    [bool] True if the entity has the component, false otherwise.
	*/

	template <typename Component>
	bool HasComponent(Entity entity) {
		auto storage = GetStorage<Component>();
		return storage && storage->Get(entity);
	}


	/*!
	* \brief
	*    Get all entities that have a specific component type.
	*
	* \tparam Component The component type.
	*
	* \return
	*    [std::vector<Entity>] Vector of entity IDs.
	*/
	template <typename Component>
	std::vector<Entity> GetEntitiesWithComponent() {
		std::vector<Entity> result;
		auto storage = GetStorage<Component>();
		if (!storage) return result;
		result = storage->GetEntities();
#ifdef PLATFORM_WINDOWS

		result.erase(
			std::remove_if(
				result.begin(),
				result.end(),
				[&](Entity entity) {
					return (HasComponent<PrefabDummyMetatag>(entity));
				}
			),
			result.end()
		);
#endif
		return result;
	}

	std::vector<Entity> GetDummyEntities();

	/*!
	* \brief
	*    Get all entities that have all the specified component types.
	*
	* \tparam Components Variadic list of component types.
	*
	* \return
	*    [std::vector<Entity>] Vector of entity IDs.
	*/
	template <typename... Components>
	std::vector<Entity> GetEntitiesWithComponents() {
		auto firstStorage = GetStorage<std::tuple_element_t<0, std::tuple<Components...>>>();
		std::vector<Entity> result;

		for (Entity entity : firstStorage->GetEntities()) {
			if ((HasComponent<Components>(entity) && ...)) {
				result.push_back(entity);
			}
		}

#ifdef PLATFORM_WINDOWS

        result.erase(
			std::remove_if(
				result.begin(),
				result.end(),
				[&](Entity entity) {
					return (HasComponent<PrefabDummyMetatag>(entity));
				}
			),
			result.end()
		);
#endif

		return result;
	}

	/*!
	* \brief
	*    Clear all component storages in the registry.
	*/
	void ClearAllComponentStorages();
	void EraseComponentStorage(std::type_index componentType);

	// ===================================
	// ===== COMPONENT STORAGE ACCESS =====
	// ===================================



	/*!
	* \brief
	*    Retrieve or create component storage for a type.
	*
	* \tparam Component The component type.
	*
	* \return
	*    [ComponentStorage<Component>*] Pointer to the component storage.
	*/
	template <typename Component>
	ComponentStorage<Component>* GetStorage() {
		auto type = std::type_index(typeid(Component));
		if (componentStorages.find(type) == componentStorages.end()) {
			componentStorages[type] = std::make_unique<ComponentStorage<Component>>();
		}
		return static_cast<ComponentStorage<Component>*>(componentStorages[type].get());
	}
private:

	EntityRegistry entityRegistry;
	std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> componentStorages;



	//friend class Savestate; //hehe
};

// =====================================
// =   Start of Component Registry     =
// =====================================
/*!
* \brief
*    Stores metadata and reflection-based creation/removal for components.
*
* \details
*    Maps component names to creation/removal functions to support
*    editor/runtime reflection.
*/
class ComponentRegistry
{
public:
	ComponentRegistry() = default;
	ComponentRegistry(ComponentRegistry&&) noexcept = default;
	ComponentRegistry& operator=(ComponentRegistry&&) noexcept = default;

	ComponentRegistry& operator=(const ComponentRegistry&) = delete;


	using ComponentCreator = std::function<void(EntityRegistry::Entity)>;
	using ComponentRemover = std::function<void(EntityRegistry::Entity)>;



	/*!
	* \brief
	*    Get the names of all registered component types.
	*
	* \return
	*    [std::vector<std::string>] Vector of component type names.
	*/
	std::vector<std::string> GetComponentTypes() const;


	/*!
	* \brief
	*    Create a component by name for a given entity.
	*
	* \param
	*    [std::string] name - Component type name.
	* \param
	*    [EntityRegistry::Entity] entity - Entity to add the component to.
	*/
	void CreateComponent(const std::string& name, EntityRegistry::Entity entity);

	/*!
	* \brief
	*    Remove a component by name from a given entity.
	*
	* \param
	*    [std::string] name - Component type name.
	* \param
	*    [EntityRegistry::Entity] entity - Entity to remove the component from.
	*/
	void RemoveComponent(const std::string& name, EntityRegistry::Entity entity);

	/*!
	* \brief
	*    Register a component type with custom creator and remover functions.
	*
	* \tparam Component The component type.
	* \param name Component type name.
	* \param creator Function that creates the component for an entity.
	* \param remover Function that removes the component from an entity.
	*/
	template <typename Component>
	void RegisterComponent(const std::string& name, ComponentCreator creator, ComponentRemover remover) {
		componentCreators[name] = creator;
		componentRemovers[name] = remover;
	}

	/*!
	* \brief
	*    Unregister a component type.
	*
	* \param
	*    [std::string] name - Component type name.
	*/
	void UnregisterComponent(const std::string& name);

private:

	std::unordered_map<std::string, ComponentCreator> componentCreators;
	std::unordered_map<std::string, ComponentCreator> componentRemovers;
};