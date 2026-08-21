/*!
@file       PrefabManager.h
@author     Kaeden Tan (kaedenjiawei.tan) (100%)
@date       26/12/2025

Declaration of the PrefabManager class, responsible for
creating, loading, and managing prefab-related operations
such as prefab instantiation and member overrides.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/

#pragma once
#include "CEO.h"
#include "Components.h"
#include "HierarchyManager.h"

/*!
 * @class PrefabManager
 * @brief
 *   Manages prefab creation, loading, and override tracking.
 *
 *   PrefabManager provides static helper functions to serialize entity
 *   hierarchies into prefabs, load prefab assets from disk, and manage
 *   overridden component members for prefab instances.
 */
class PrefabManager {
	friend class ResourceManager;
	public:
		PrefabManager() = default;
		~PrefabManager() = default;

        /*!
		* @brief
		*   Creates a prefab asset from an entity hierarchy.
		*
		*   Serializes the specified entity and its children into a prefab
		*   JSON file stored on disk.
		*
		* @param[in] prefabName
		*   Name of the prefab asset (without file extension).
		* @param[in] entID
		*   Root entity to serialize. A value of 0 typically indicates
		*   the currently selected entity.
		* @param[in] basepath
		*   Directory where prefab files are saved.
		*
		* @return
		*   True if the prefab was successfully created, false otherwise.
		*/
		static bool CreatePrefab(std::string const& prefabName, Registry::Entity entID = 0, std::string const& basepath = "Assets\\Prefabs\\");
		
		/*!
		* @brief
		*   Marks a component member as overridden for a prefab instance.
		*
		*   Overridden members are excluded from future prefab updates,
		*   allowing instance-specific customization.
		*
		* @param[in,out] prefabComp
		*   Prefab component associated with the entity instance.
		* @param[in] component
		*   Name of the component containing the overridden member.
		* @param[in] memberName
		*   Name of the overridden member variable.
		*/
		static void OverrideMember(PrefabComponent& prefabComp, std::string const& component, std::string const& memberName);

	private:
		PrefabManager(const PrefabManager&) = delete;
		PrefabManager& operator=(const PrefabManager&) = delete;
#ifdef PLATFORM_WINDOWS
		/*!
		* @brief
		*   Loads a prefab JSON document from disk (Windows path format).
		*
		* @param[in] prefabName
		*   Name of the prefab asset (without file extension).
		* @param[in] basepath
		*   Directory where prefab files are stored.
		*
		* @return
		*   Parsed RapidJSON document if successful, std::nullopt otherwise.
		*/
		static std::optional<rapidjson::Document> LoadPrefab(std::string const& prefabName, std::string const& basepath = "Assets\\Prefabs\\");
#else
		/*!
		* @brief
		*   Loads a prefab JSON document from disk (non-Windows path format).
		*
		* @param[in] prefabName
		*   Name of the prefab asset (without file extension).
		* @param[in] basepath
		*   Directory where prefab files are stored.
		*
		* @return
		*   Parsed RapidJSON document if successful, std::nullopt otherwise.
		*/
		static std::optional<rapidjson::Document> LoadPrefab(std::string const& prefabName, std::string const& basepath = "Prefabs/");

#endif
		/*!
		* @brief
		*   Attaches and initializes a PrefabComponent on an entity.
		*
		*   Sets the prefab name and prepares override tracking data
		*   for the instantiated prefab entity.
		*
		* @param[in] rootEnt
		*   Root entity of the prefab instance.
		* @param[in] prefabName
		*   Name of the associated prefab asset.
		*/	
		static void AddPrefabComp(Registry::Entity rootEnt, std::string const& prefabName);
};