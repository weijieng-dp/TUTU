/*!
@file       LayerManager.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Manages up to 64 named layers for the engine.
			Each layer currently has:
				- Collision masks: which other layers can collide with
			(render mask is done via the camera component).

			Provides helper functions for checking and getting layers.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "LayerManager.h"

void LayerManager::Init(json& config) {
	collisionMasks.resize(MAX_LAYERS);

	auto obj{ config.GetObj("Layers Configuration") };
	auto layerNames{ obj->GetValue("Layer Names")->GetStringVec() };
	auto collisionLayerMasks{ obj->GetValue("Collision Layers Mask")->GetULLVec() };

	if (!layerNames.empty() && !collisionLayerMasks.empty()) {
		for (int i{}; i < MAX_LAYERS; ++i) {	// iterate MAX_LAYERS times
			if (layerNames[i] == nullptr || *layerNames[i] == "") continue;
			std::string name{ *layerNames[i]};					// get name of the layer at index i from config json
			CreateLayer(i, name);								// create the layer
			if (collisionLayerMasks[i] == nullptr) continue;
			collisionMasks[i] = *collisionLayerMasks[i];		// set up collision bitmask according to info from config json
		}
	}
	else {		// Create the default layers
		CreateLayer(0, "Default");
		CreateLayerRange(1, 5, "UI");
		CreateLayerRange(6, 2, "Player");
		CreateLayerRange(8, 10, "Enemy");
	}

	CEO::Get<EventsDispatcher>()->Subscribe<Events::EntityModified>([](const Events::EntityModified& e) {
			switch (e.modification) {
				case Events::EntityModified::MODIFICATION::ADD_ENTITY:
				case Events::EntityModified::MODIFICATION::MODIFY_ENTITY_LAYER:
					//CEO::Get<LayerManager>()->AddEntityToCache(e.layerIndex, e.ent);
					CEO::Get<LayerManager>()->ChangeCachedEntity(e.layerIndex, e.ent);
					break;
				case Events::EntityModified::MODIFICATION::REMOVE_ENTITY:
					CEO::Get<LayerManager>()->RemoveEntityFromCache(e.layerIndex, e.ent);
					break;
				case Events::EntityModified::MODIFICATION::REMOVE_ALL_ENTITY:
					CEO::Get<LayerManager>()->ClearEntityCache();
					break;
				
			}
		}
	);

	CEO::Get<EventsDispatcher>()->Subscribe<Events::UpdateLayersEvent>([](const auto&) {
			CEO::Get<LayerManager>()->UpdateTransparentIndex();
		}
	);


	CEO::Get<EventsDispatcher>()->Dispatch<Events::UpdateLayersEvent>({});
}

bool LayerManager::CreateLayer(LayerIndex index, const std::string& name) {
	if (index >= MAX_LAYERS) {		// check if index is valid [0, MAX_LAYERS)
#ifdef _DEBUG
		LOGI("Index out of range in CreateLayer()");
#endif
		return false; // if index is out of range
	}
	layers[index] = name;				// set the name of layer at index to be given name
	collisionMasks[index].set(index);	// layer can collide with itself

	return true;
}

int LayerManager::CreateLayer(const std::string& name) {
	for (int i{}; i < MAX_LAYERS; ++i) {	// look through every layer
		if (layers[i].empty()) {			// if layer at index i is not set up (no name)
			layers[i] = name;				// set the name of layer at index to be given name
			collisionMasks[i].set(i);		// layer can collide with itself
			return i;						// return the layer index
		}
	}
	return -1;	// otherwise, -1 to indicate no available layer
}

bool LayerManager::RenameLayer(LayerIndex index, const std::string& name)
{
	if (index >= MAX_LAYERS) {		// check if index is valid [0, MAX_LAYERS)
#ifdef _DEBUG
		LOGI("Index out of range in CreateLayer()");
#endif
		return false; // if index is out of range
	}
	layers[index] = name;				// set the name of layer at index to be given name

	return true;
}

std::vector<LayerManager::LayerIndex> LayerManager::CreateLayerRange(LayerIndex startIndex, LayerIndex count, const std::string& name) {
	if (startIndex >= MAX_LAYERS || count > MAX_LAYERS - startIndex)	// check that index is valid
		return std::vector<LayerIndex>();	// return empty vector if index is invalid
	std::vector<LayerIndex> indices;		// to save the created layers' indices
	for (LayerIndex i{}; i < count && i < MAX_LAYERS; ++i) {	// iterate count number of times
		LayerIndex index{ i + startIndex };	// calculate the actual index
		layers[index] = name + std::to_string(i);	// set layer name as name + current index
		indices.push_back(index);					// add current index into indices container

		collisionMasks[index].set(index);			// layer can collide with itself
	}
	return indices;					// return indices container
}

int LayerManager::GetLayerIndex(const std::string& name) {
	if (!name.empty()) {
		for (int i{}; i < MAX_LAYERS; ++i) {	// loop through all the layers
			if (layers[i] == name)	// check if the name match
				return i;			// return the index if name matches
		}
	}	
	return -1;	// return -1 to indicate no layer was found
}

std::vector<LayerManager::LayerIndex> LayerManager::GetLayerIndices(const std::string& name) {
	std::vector<LayerIndex> indices;
	if (!name.empty()) {
		for (LayerIndex i{}; i < MAX_LAYERS; ++i) {					// loop through all layers
			if (layers[i].compare(0, name.size(), name) == 0) {	// check if the prefix of the layer matches given name
				if (layers[i].size() > name.size()) {
					std::string postfix{ layers[i].substr(name.size()) };	// strip the name prefix
					try {	// check that after name prefix is a number
						int postfixInteger{ stoi(postfix) };	// not used, just to check if it is an integer
						(void)postfixInteger;
						indices.push_back(i);
					}
					catch (...) { continue; }
				}
				else indices.push_back(i);
			}
		}
	}
	return indices;
}

std::string LayerManager::GetLayerName(LayerIndex index) const {
	if (index >= MAX_LAYERS) return std::string();	// return empty string if index is invalid
	return layers[index];
}

uint64_t LayerManager::GetLayerMaskByName(const std::string& name) {
	int id{ GetLayerIndex(name) };
	return id >= 0 ? (static_cast<uint64_t>(1u) << static_cast<uint64_t>(id)) : 0;
}

uint64_t LayerManager::GetLayerMasksByName(const std::string& name) {
	uint64_t mask{};
	if (!name.empty()) {
		for (int i{}; i < MAX_LAYERS; ++i) {					// loop through all layers
			if (layers[i].compare(0, name.size(), name) == 0) {	// check if the prefix of the layer matches given name
				if (layers[i].size() > name.size()) {
					std::string postfix{ layers[i].substr(name.size()) };	// strip the name prefix
					try {	// check that after name prefix is a number
						int postfixInteger{ std::stoi(postfix) };	// not used, just to check if it is an integer
						(void)postfixInteger;
						mask |= (static_cast<uint64_t>(1u) << i);
					}
					catch (...) { continue; }
				}
				else mask |= (static_cast<uint64_t>(1u) << i);;
			}
		}
	}
	return mask;
}

void LayerManager::SetCollisionEnabled(LayerIndex aIndex, LayerIndex bIndex, bool enabled) {
	if (aIndex >= MAX_LAYERS || bIndex >= MAX_LAYERS) {	// check if indices are out of range
#ifdef _DEBUG
		LOGI("Supplied Indices to SetCollisionEnabled() are out of range");
#endif
		return;
	}
	collisionMasks[aIndex].set(bIndex, enabled);
	collisionMasks[bIndex].set(aIndex, enabled);
}

bool LayerManager::CollisionEnabled(LayerIndex aIndex, LayerIndex bIndex) {
	if (aIndex >= MAX_LAYERS || bIndex >= MAX_LAYERS) {	// check if indices are out of range 
		return false;
	}
	return collisionMasks[aIndex].test(bIndex);
}

void LayerManager::UpdateLayers(json& config) {
	auto obj{ config.GetObj("Layers Configuration") };					// get layers config object
	auto layerNames{obj->GetValue("Layer Names")->GetStringVec()};		// get the names of the layers
	auto collisionLayerMasks{ obj->GetValue("Collision Layers Mask")->GetULLVec() };	// get the collision masks of all the layers

	for (int i{}; i < MAX_LAYERS; ++i) {			// iterate through all the layers
		if (layerNames[i] != nullptr) {				// nullptr check
			*layerNames[i] = layers[i];				// update layer name with current layer name
		}

		if (collisionLayerMasks[i] != nullptr) {	// nullptr check
			*collisionLayerMasks[i] = collisionMasks[i].to_ullong();	// update collision mask with current collision mask
		}
	}
}

void LayerManager::AddEntityToCache(LayerIndex layerIndex, EntityRegistry::Entity ent) {
	if (layerIndex >= MAX_LAYERS) return;
	cache[layerIndex].push_back(ent);
}

void LayerManager::RemoveEntityFromCache(LayerIndex layerIndex, EntityRegistry::Entity ent) {
	if (layerIndex >= MAX_LAYERS) return;
	
	auto& entityVec{ cache[layerIndex] };
	auto it{ std::find(entityVec.begin(), entityVec.end(), ent) };

	if (it != entityVec.end()) {
		*it = entityVec.back();
		entityVec.pop_back();
	}
}

void LayerManager::ClearEntityCache() {
	for (auto& c : cache) c.clear();
}

std::vector<EntityRegistry::Entity> LayerManager::GetCachedEntitiesOnLayer(LayerIndex layerIndex) const {
	if (layerIndex >= MAX_LAYERS) return std::vector<EntityRegistry::Entity>();
	return cache[layerIndex];
}

std::vector<EntityRegistry::Entity> LayerManager::GetCachedEntitiesOnLayerByName(const std::string& name) const {
	auto it{ std::find(layers.begin(), layers.end(), name) };
	if (it == layers.end()) return std::vector<EntityRegistry::Entity>();
	return cache[it - layers.begin()];
}

void LayerManager::ChangeCachedEntity(LayerIndex newLayer, EntityRegistry::Entity ent) {
	
	for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// removing ent from prev layer
		auto& cached{ cache[layer] };
		auto it{ std::find(cached.begin(), cached.end(), ent) };
		if(it != cached.end()) {
			if (layer == newLayer) return;

			*it = cached.back();
			cached.pop_back();
			break;
		}
	}
	AddEntityToCache(newLayer, ent);		// Add ent to new layer
}

std::vector<EntityRegistry::Entity> LayerManager::GetEntitiesVisible(uint64_t camMask) const {
	std::vector<EntityRegistry::Entity> result;
	auto& registry{ *CEO::Get<Registry>() };				// get the entity registry
	for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layers
		if (GetLayerName(layer).empty() || !IsVisibleOnCam(camMask, layer)) continue;	// skip if layer is not visible or name is empty (empty name means disabeld layer)
		result.insert(result.end(), cache[layer].begin(), cache[layer].end());			// append entities on this layer to result
#ifdef PLATFORM_WINDOWS
		result.erase(
			std::remove_if(
				result.begin(),
				result.end(),
				[&](EntityRegistry::Entity entity) { return (registry.HasComponent<PrefabDummyMetatag>(entity)); }
			),
			result.end()
		);
#endif
	}
	return result;
}

std::vector<EntityRegistry::Entity> LayerManager::GetEntitiesCollidable(LayerIndex layerIndex) {
	if(layerIndex >= MAX_LAYERS) return std::vector<EntityRegistry::Entity>();	// check validity of layerIndex
	std::vector<EntityRegistry::Entity> result;
	auto& registry{ *CEO::Get<Registry>() };				// get the entity registry
	for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layer
		if (GetLayerName(layer).empty() || !CollisionEnabled(layerIndex, layer)) continue;	// skip if layer is not collidable or name is empty (empty name means disabeld layer)
		result.insert(result.begin(), cache[layer].begin(), cache[layer].end());			// append entities on this layer to result
#ifdef PLATFORM_WINDOWS
		result.erase(
			std::remove_if(
				result.begin(),
				result.end(),
				[&](EntityRegistry::Entity entity) { return (registry.HasComponent<PrefabDummyMetatag>(entity)); }
			),
			result.end()
		);
#endif
	}
	return result;
}

void LayerManager::UpdateTransparentIndex() {
	for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layer
		const auto& name{ GetLayerName(layer) };
		if (name.find("Transparent") != std::string::npos) {
			transparentIndex = layer;
			break;
		}
	}
}