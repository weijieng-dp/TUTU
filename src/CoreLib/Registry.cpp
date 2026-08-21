/**___________________________________________________________________________/
@file          Registry.cpp
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
#include "Registry.h"
#include "Components.h"


// =====================================
// =      Start of EntityRegistry      =
// =====================================
EntityRegistry::Entity EntityRegistry::CreateEntityID()
{
	return ++nextId;
}

EntityRegistry::Entity EntityRegistry::CreateEntityID(EntityRegistry::Entity ID)
{
	if (nextId > ID)
		return ID;
	else
		return nextId = ID;
}

void EntityRegistry::RestartEntityCount()
{
	nextId = 0;
}

void EntityRegistry::DestroyEntity(Entity entity)
{
	allEntities.erase(entity);
}

void EntityRegistry::AddEntity(Entity entity)
{
	allEntities.insert(entity);
}

std::vector<EntityRegistry::Entity> EntityRegistry::GetAllEntity()
{
	return std::vector<Entity>(allEntities.begin(), allEntities.end());
}

bool EntityRegistry::IsEntityValid(Entity ent) {
	return allEntities.find(ent) != allEntities.end();
}


// =====================================
// =        Start of Registry          =
// =====================================

std::vector<Registry::Entity> Registry::GetAllEntity()
{
	return entityRegistry.GetAllEntity();
}

void Registry::RestartEntityCount()
{
	entityRegistry.RestartEntityCount();
}

Registry::Entity Registry::CreateEntity()
{
	Entity entity = entityRegistry.CreateEntityID();
	entityRegistry.AddEntity(entity);

	return entity;
}

Registry::Entity Registry::CreateEntity(int ID)
{
	Entity entity = entityRegistry.CreateEntityID(ID);
	entityRegistry.AddEntity(entity);

	return entity;
}

void Registry::DestroyAllEntities()
{
	for (Entity& entity : entityRegistry.GetAllEntity())
	{
		DestroyEntity(entity);
	}
}
bool Registry::IsEntityValid(Registry::Entity ent) {
	return entityRegistry.IsEntityValid(ent);
}

void Registry::ClearAllComponentStorages()
{

	for (auto it = componentStorages.begin(); it != componentStorages.end(); )
	{
		// Store the current iterator for safe erasure
		it->second->Clear();
		it = componentStorages.erase(it);  // Erase returns the next iterator

	}

}
void Registry::EraseComponentStorage(std::type_index componentType)
{
	componentStorages.erase(componentType);
}

void Registry::DestroyEntity(Registry::Entity entity) {

	if (CEO::Get<Registry>()->GetComponent<NativeScriptingComponent>(entity))
	{
		for (auto [Scripts, Instance] : CEO::Get<Registry>()->GetComponent<NativeScriptingComponent>(entity)->scripts)
		{
			Instance.OnDeleteScript(Instance);
		}
	}
	for (auto& pair : componentStorages) {
		auto& storage = pair.second;
		storage->Remove(entity);
	}

	entityRegistry.DestroyEntity(entity);
}

std::vector<rtr::Instance> Registry::GetAllComponentsRTR(Registry::Entity entity) {
	std::vector<rtr::Instance> componentsRTR;
	for (auto& [typeIndex, storage] : componentStorages) 
	{
		rtr::Instance component = storage->GetCompFromEntity(entity);
		if (component.isValid())
		{
			componentsRTR.push_back(component);

		}
	}
	return componentsRTR;
}

rtr::Instance Registry::GetComponentsRTR(Registry::Entity entity, rtr::TypeInfo type)
{
	
		auto it = componentStorages.find(type.Type());
		if (it != componentStorages.end())
		{

			rtr::Instance component = it->second->GetCompFromEntity(entity);
			return component;
		}
	
	return rtr::Instance();
}


void Registry::RemoveComponent(Registry::Entity ent, rtr::TypeInfo type)
{

	auto it = componentStorages.find(type.Type());
	
	if (it != componentStorages.end())
	{
		it->second->Remove(ent);
	}
}

std::vector<Registry::Entity> Registry::GetDummyEntities() {
	std::vector<Entity> result;
	auto storage = GetStorage<PrefabDummyMetatag>();
	if (!storage) return result;

	result = storage->GetEntities();

	return result;
}

// =====================================
// =     Start of Component Registry   =
// =====================================

std::vector<std::string> ComponentRegistry::GetComponentTypes() const
{
	std::vector<std::string> comp;
	for (auto& [type, func] : componentCreators)
	{
		comp.push_back(type);
	}

	return comp;
}

void ComponentRegistry::RemoveComponent(const std::string& name, EntityRegistry::Entity entity )
{
	if (componentRemovers.find(name) != componentRemovers.end()) {
		componentRemovers[name](entity);
	}
}

void ComponentRegistry::CreateComponent(const std::string& name, EntityRegistry::Entity entity) {
	if (componentCreators.find(name) != componentCreators.end()) {
		componentCreators[name](entity);
	}
}



void ComponentRegistry::UnregisterComponent(const std::string& name) {
	componentCreators.erase(name);
	componentRemovers.erase(name);
}