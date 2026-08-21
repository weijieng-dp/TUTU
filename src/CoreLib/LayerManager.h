/*!
@file       LayerManager.h
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Manages up to 64 named layers for the engine.
			Each layer currently has:
				- Collision masks: which other layers can collide with
			(render mask is done via the camera component).

			Provides helper functions for checking and getting layers.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#pragma once
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include "Json.h"

class LayerManager {
	public:
		using LayerIndex = uint32_t;
		static constexpr int MAX_LAYERS{ 64 };		// the max number of named layers

		/*!
		* \brief 
		*	Initializes the LayerManager from config json. Creates a few default layers
		*	no layers are set up in config json.
		* 
		* \param[in, out] config
		*	- A reference to the json object where the layers information are stored in.
		*/
		void Init(json& config);

		/*!
		* \brief Create a new named layer at the specified index. Will override any existing layer at index.
		* \param[in] index - The index to create the layer at. Acceptable values: [0, MAX_LAYERS].
		* \param[in] name - The name to associate with the layer.
		* \return - Whether the layer was successfully created. False if index is out of range.
		*/
		bool CreateLayer(LayerIndex index, const std::string& name);

		/*!
		* \brief Create a new named layer at the first available index.
		* \param[in] name - The name to associate with the layer.
		* \return - The index of the newly created index. -1 if all layers are taken.
		*/
		int CreateLayer(const std::string& name);

		/*!
		* \brief Create a new named layer at the specified index. Will override any existing layer at index.
		* \param[in] index - The index to create the layer at. Acceptable values: [0, MAX_LAYERS].
		* \param[in] name - The name to associate with the layer.
		* \return - Whether the layer was successfully created. False if index is out of range.
		*/
		bool RenameLayer(LayerIndex index, const std::string& name);

		/*!
		* \brief
		*	Create a range of named layer, starting from the specified index. Will
		*	override any existing layer at index. Naming convention used is 
		*	[name] + [current count]. Example: CreateLayerRange(0, 3, Player) will create:
		*		- Player0
		*		- Player1
		*		- Player2
		*
		* \param[in] startIndex - The starting index to create the layers at. Acceptable values: [0, MAX_LAYERS - count].
		* \param[in] count - The number of layers to create.
		* \param[in] name - The name to associate with the layer.
		* \return - A vector of the indices of the created layers.
		*/
		std::vector<LayerIndex> CreateLayerRange(LayerIndex startIndex, LayerIndex count, const std::string& name);

		/*!
		* \brief Get the index of the layer via specified name. Name must match exactly.
		* \param[in] name - The name of the layer to get the index of.
		* \return - The index of the specified layer, -1 if no layer is found.
		*/
		int GetLayerIndex(const std::string& name);

		/*!
		* \brief
		*	Get the index of the layer via specified name. Name will be the prefix:
		*	Example: GetLayerIndices("Player") to get all player layers:
		*		- Player0
		*		- Player1
		*		- Player2
		*
		* \param[in] name - The name / prefix of the layers to get the indices of.
		* \return - A vector containing all the indices found.
		*/
		std::vector<LayerIndex> GetLayerIndices(const std::string& name);

		/*!
		* \brief Get the name of the layer by its index.
		* \param[in] index - The index of the layer to get the name of.
		* \return - An std::string obj with the name of the object.
		*/
		std::string GetLayerName(LayerIndex index) const;

		/*!
		* \brief Converts the index into a bitmask value.
		* \param[in] index - The index to convert to bitmask value.
		* \return - The bitmask value, as uint64_t.
		*/
		inline uint64_t GetLayerMask(LayerIndex index) const { return (static_cast<uint64_t>(1u) << index); }

		/*!
		* \brief
		*	Get the bitmask value of the specified named layer.
		*
		* \param[in] name
		*	- The name of the layer to convert their index to bitmask value.
		*
		* \return
		*	- The bitmask value, as uint64_t. 0 if no layer with specified name was found.
		*/
		uint64_t GetLayerMaskByName(const std::string& name);

		/*!
		* \brief
		*	Get the combined bitmask value of the layers with the specified prefix name.
		*
		* \param[in] name
		*	- The prefix name of the layers to convert their index to bitmask value.
		*	
		* \return
		*	- The combined bitmask value, as uint64_t. 0 if no layers with the specified
		*	prefix name was found.
		*/
		uint64_t GetLayerMasksByName(const std::string& name);

		/*!
		* \brief Set the collision for 2 specified collision masks.
		* \param[in] aIndex - The collision mask to set collision with bIndex.
		* \param[in] bIndex - The collision mask to set collision with aIndex.
		* \param[in] enabled - Whether to enable or disable collision between the 2 specified collision masks.
		*/
		void SetCollisionEnabled(LayerIndex aIndex, LayerIndex bIndex, bool enabled);

		/*!
		* \brief Whether collision are enabled between the 2 specified collision masks.
		* \param[in] aIndex - To check whether collision is enabled with bIndex.
		* \param[in] bIndex - To check whether collision is enabled with aIndex.
		*/
		bool CollisionEnabled(LayerIndex aIndex, LayerIndex bIndex);

		/*!
		* \brief
		*	Updates layers information in config json object with current layers.
		*	Doesn't serialise the layers, call Serialize() on config.json
		*	after this function to seralize layers.
		*
		* \param[in, out] config - A reference to the json object where the layers information are stored in.
		*/
		void UpdateLayers(json& config);

		/*!
		* \brief Check whether a layer is visible on specified camera.
		* \param[in] camMask	- The camera mask to check visibility of layer with.
		* \param[in] layer		- The layer index of the layer to check.
		* \return - True if layer is visible, false otherwise.
		*/
		inline bool IsVisibleOnCam(uint64_t camMask, LayerIndex layer) const { return (camMask & GetLayerMask(layer)) != 0; }

		/*!
		* \brief Updates transparent layer index. Will search for the first layer with name that contains "Transparent".
		*/
		void UpdateTransparentIndex();

		/*!
		* \brief Add specified entity to entity id layer cache.
		* \param[in] layerIndex	- The layer index the new entity belongs to.
		* \param[in] ent		- The entity id to add to cache.
		*/
		void AddEntityToCache(LayerIndex layerIndex, EntityRegistry::Entity ent);

		/*!
		* \brief Remove specified entity from entity id layer cache.
		* \param[in] layerIndex	- The layer index the entity (to be removed) belongs to.
		* \param[in] ent		- The entity id remove from cache.
		*/
		void RemoveEntityFromCache(LayerIndex layerIndex, EntityRegistry::Entity ent);

		/*!
		* \brief Change which layer the specified entity belongs to in entity id layer cache.
		* \param[in] newLayer	- The new layer index the entity should be moved to.
		* \param[in] ent		- The entity id to modify.
		*/
		void ChangeCachedEntity(LayerIndex newLayer, EntityRegistry::Entity ent);

		/*!
		* \brief Clears entity id layer cache.
		*/
		void ClearEntityCache();

		/*!
		* \brief Gets all the cached entity id that belongs to specified layerIndex.
		* \param[in] layerIndex	- The layer index to retrieve the entities from.
		* \return - All cached entities that belongs to layerIndex, else return empty vector.
		*/
		std::vector<EntityRegistry::Entity> GetCachedEntitiesOnLayer(LayerIndex layerIndex) const;

		/*!
		* \brief Gets all the cached entity id that belongs to specified layer by name.
		* \param[in] name - The name of the layer to retrieve the entities from.
		* \return - All cached entities that belongs to layer with specified name, else return empty vector.
		*/
		std::vector<EntityRegistry::Entity> GetCachedEntitiesOnLayerByName(const std::string& name) const;

		/*!
		* \brief Gets all the cached entity id that are visible on specified camera.
		* \param[in] camMask - The camera mask of the camera to check visiblity with.
		* \return - All cached entities that are visible on specified camera, else return empty vector.
		*/
		std::vector<EntityRegistry::Entity> GetEntitiesVisible(uint64_t camMask) const;

		/*!
		* \brief Gets all the cached entity id that are visible on specified camera and have specified component.
		* \param[in] camMask - The camera mask of the camera to check visiblity with.
		* \return - All cached entities that are visible on specified camera and have specified component, else return empty vector.
		*/
		template <typename Component>
		std::vector<EntityRegistry::Entity> GetEntitiesVisibleWithComponent(uint64_t camMask) {
			std::vector<EntityRegistry::Entity> result;
			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layers
				if (GetLayerName(layer).empty() || !IsVisibleOnCam(camMask, layer)) continue;	// skip if layer is not visible or name is empty (empty name means disabeld layer)
				for (auto ent : cache[layer]) {								// loop through entities in current layer
					if (!registry.HasComponent<Component>(ent)) continue;	// if entity doesn't have specified component, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is visible and have specified component
				}
			}
			return result;
		}
		/*!
		* \brief 
		*	Gets all transparent/translucent or opaque cached entity id that are visible on specified camera with specified component.
		*	If transparent flag is true, will start checking for entities to retrieve from transparent index onwards.
		*	If transparent flag is false, will start checking for entities from start till transparent index.
		* \param[in] camMask		- The camera mask of the camera to check visiblity with.
		* \param[in] transparent	- Whether to get the transparent/translucent or opaque visible cached entities.
		* \return - All cached entities that are visible on specified camera with specified component, and specified name, else return empty vector.
		*/
		template <typename Component>
		std::vector<EntityRegistry::Entity> GetEntitiesVisibleWithComponent(uint64_t camMask, bool transparent) {
			std::vector<EntityRegistry::Entity> result;
			if (transparentIndex == -1) {
				if (transparent) return result;
				else return GetEntitiesVisibleWithComponent<Component>(camMask);
			}
			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			LayerIndex layer{}, end{};
			if (transparent) layer = static_cast<LayerIndex>(transparentIndex);
			else end = static_cast<LayerIndex>(transparentIndex);
			for (; layer < end; ++layer) {	// loop through all layers
				if (GetLayerName(layer).empty() || !IsVisibleOnCam(camMask, layer)) continue;	// skip if layer is not visible or name is empty (empty name means disabled layer)
				for (auto ent : cache[layer]) {								// loop through entities in current layer
					if (!registry.HasComponent<Component>(ent)) continue;	// if entity doesn't have specified component, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is visible and have specified component
				}
			}
			return result;
		}

		/*!
		* \brief Gets all the cached entity id that are visible on specified camera and have specified components.
		* \param[in] camMask - The camera mask of the camera to check visiblity with.
		* \return - All cached entities that are visible on specified camera and have specified components, else return empty vector.
		*/
		template <typename... Components>
		std::vector<EntityRegistry::Entity> GetEntitiesVisibleWithComponents(uint64_t camMask) {
			std::vector<EntityRegistry::Entity> result;
			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layers
				if (GetLayerName(layer).empty() || !IsVisibleOnCam(camMask, layer)) continue;	// skip if layer is not visible or name is empty (empty name means disabeld layer)
				for (auto ent : cache[layer]) {											// loop through entities in current layer
					if (!(registry.HasComponent<Components>(ent) && ...)) continue;		// if entity doesn't have specified components, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is visible and have specified components
				}
			}
			return result;
		}
		/*!
		* \brief Gets all the cached entity id that are visible on specified camera and have specified components from layer with specified name.
		* \param[in] camMask	- The camera mask of the camera to check visiblity with.
		* \param[in] name		- The layer to get entities visible from.
		* \param[in] exact		- Whether to find layers that has the exact name or contains the name.
		* \return - All cached entities that are visible on specified camera, have specified components, and specified name, else return empty vector.
		*/
		template <typename... Components>
		std::vector<EntityRegistry::Entity> GetEntitiesVisibleWithComponentsByLayerName(uint64_t camMask, const std::string& name, bool exact = true) {
			std::vector<EntityRegistry::Entity> result;
			if (exact) {
				auto it{ std::find(layers.begin(), layers.end(), name) };
				if (it == layers.end()) return result;
				LayerIndex layer{ static_cast<LayerIndex>(it - layers.begin()) };
				if (!IsVisibleOnCam(camMask, layer)) return result;
				// Else this layer exist
				Registry& registry{ *CEO::Get<Registry>() };							// get the entity registry
				for (auto ent : cache[layer]) {							// loop through entities in layer with specified name
					if (!(registry.HasComponent<Components>(ent) && ...)) continue;		// if entity doesn't have specified components, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is visible and have specified components
				}
			} 
			else {
				Registry& registry{ *CEO::Get<Registry>() };							// get the entity registry
				for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layers
					const auto& layerName{ GetLayerName(layer) };
					if (layerName.empty() || !IsVisibleOnCam(camMask, layer) || layerName.find(name) == std::string::npos) continue;
					for (auto ent : cache[layer]) {											// loop through entities in current layer
						if (!(registry.HasComponent<Components>(ent) && ...)) continue;		// if entity doesn't have specified components, skip
#ifdef PLATFORM_WINDOWS
						if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
						result.push_back(ent);							// add to result if entity is visible and have specified components
					}
				}
			}
			return result;
		}
		/*!
		* \brief
		*	Gets all transparent/translucent or opaque cached entity id that are visible on specified camera and have specified components.
		*	If transparent flag is true, will start checking for entities to retrieve from transparent index onwards.
		*	If transparent flag is false, will start checking for entities from start till transparent index.
		* \param[in] camMask		- The camera mask of the camera to check visiblity with.
		* \param[in] transparent	- Whether to get the transparent/translucent or opaque visible cached entities.
		* \return - All cached entities that are visible on specified camera, have specified components, and specified name, else return empty vector.
		*/
		template <typename... Components>
		std::vector<EntityRegistry::Entity> GetEntitiesVisibleWithComponents(uint64_t camMask, bool transparent) {
			std::vector<EntityRegistry::Entity> result;
			if (transparentIndex == -1) {
				if (transparent) return result;
				else return GetEntitiesVisibleWithComponents<Components...>(camMask);
			}

			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			LayerIndex layer{}, end{ MAX_LAYERS };
			if (transparent) layer = static_cast<LayerIndex>(transparentIndex);
			else end = static_cast<LayerIndex>(transparentIndex);
			for (; layer < end; ++layer) {	// loop through all layers
				if (GetLayerName(layer).empty() || !IsVisibleOnCam(camMask, layer)) continue;	// skip if layer is not visible or name is empty (empty name means disabled layer)
				for (auto ent : cache[layer]) {											// loop through entities in current layer
					if (!(registry.HasComponent<Components>(ent) && ...)) continue;		// if entity doesn't have specified components, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is visible and have specified components
				}
			}
			return result;
		}

		/*!
		* \brief Gets all the cached entity id that are collidable with specified layer with layerIndex.
		* \param[in] layerIndex - The collision layer index to check with.
		* \return All cached entities that are collidable with specified layer, else return empty vector.
		*/
		std::vector<EntityRegistry::Entity> GetEntitiesCollidable(LayerIndex layerIndex);

		/*!
		* \brief 
		*	Gets all the cached entity id that are collidable with specified layer with layerIndex
		*	and have the specified component.
		* 
		* \param[in] layerIndex - The collision layer index to check with.
		* 
		* \return 
		*	All cached entities that are collidable with specified layer and have the specified
		*	component, else return empty vector.
		*/
		template <typename Component>
		std::vector<EntityRegistry::Entity> GetEntitiesCollidableWithComponent(LayerIndex layerIndex) {
			if (layerIndex >= MAX_LAYERS) return std::vector<EntityRegistry::Entity>();	// check validity of layerIndex
			std::vector<EntityRegistry::Entity> result;
			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layer
				if (GetLayerName(layer).empty() || !CollisionEnabled(layerIndex, layer)) continue;	// skip if layer is not collidable or name is empty (empty name means disabeld layer)
				for (auto ent : cache[layer]) {								// loop through entities in current layer 
					if (registry.HasComponent<Component>(ent)) continue;	// if entity doesn't have specified component, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is collidable and have specified component
				}
			}
			return result;
		}

		template <typename... Components>
		std::vector<EntityRegistry::Entity> GetEntitiesCollidableWithComponents(LayerIndex layerIndex) {
			if (layerIndex >= MAX_LAYERS) return std::vector<EntityRegistry::Entity>();	// check validity of layerIndex
			std::vector<EntityRegistry::Entity> result;
			Registry& registry{ *CEO::Get<Registry>() };			// get the entity registry
			for (LayerIndex layer{}; layer < MAX_LAYERS; ++layer) {	// loop through all layer
				if (GetLayerName(layer).empty() || !CollisionEnabled(layerIndex, layer)) continue;	// skip if layer is not collidable or name is empty (empty name means disabeld layer)
				for (auto ent : cache[layer]) {										// loop through entities in current layer 
					if (!(registry.HasComponent<Components>(ent) && ...)) continue;	// if entity doesn't have specified components, skip
#ifdef PLATFORM_WINDOWS
					if (registry.HasComponent<PrefabDummyMetatag>(ent)) continue;
#endif
					result.push_back(ent);							// add to result if entity is collidable and have specified component
				}
			}
			return result;
		}
	private:
		std::array<std::string, MAX_LAYERS> layers{};			// array of size MAX_LAYERS that holds the layer names
		std::vector<std::bitset<MAX_LAYERS>> collisionMasks;	// array the collision bitmask

		std::array<std::vector<EntityRegistry::Entity>, MAX_LAYERS> cache{};	// entity id cache group by layers

		int transparentIndex{ -1 };	// the index to the first layer with transpacency (treats everything after this to be transparent and before to be opaque)
};