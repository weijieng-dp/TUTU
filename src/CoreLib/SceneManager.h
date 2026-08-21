/*!
@file       SceneManager.h
@author     Kaeden Tan (kaedenjiawei.tan) 80%
@co-author  yukang.ou@digipen.edu(yukang)(20%)
@date       07/10/2025
@brief      Implementation of the SceneManager class, which handles loading
            and managing scene entities from JSON data into the ECS registry.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#pragma once
#include "rapidjson/document.h"
#include "Registry.h"
#include "Json.h"
#include <stack>

/*!
* \brief
*   Manages scene loading and entity creation from JSON data.
*   Singleton class that handles the conversion of JSON scene data
*   into actual entities and components in the ECS registry.
*/
class SceneManager {
	friend class ResourceManager;

public:
	SceneManager() = default;
	~SceneManager() = default;

	enum SCENE_ACTION {
		PUSH,POP,CHANGE,NONE
	};

	enum class SERIALIZE_ACTION {
		SAVE_SCENE, CREATE_PREFAB, COPY
	};

	enum class DESERIALIZE_ACTION {
		LOAD_SCENE, LOAD_PREFAB, COPY
	};

	/*!
	 * \brief
	 *   Creates a new scene file on disk.
	 *
	 *   Initializes and saves an empty or default scene structure using the given
	 *   scene name and base path.
	 *
	 * \param[in] sceneName
	 *   Name of the scene to create (without file extension).
	 * \param[in] basepath
	 *   Directory where the scene file will be saved.
	 */
	static void CreateScene(std::string const& sceneName,
		std::string const& basepath = "Assets\\Scenes\\");

	/*!
	 * \brief
	 *   Serializes a single entity and its components into a RapidJSON value.
	 *
	 *   Converts the specified entity and all serializable components into
	 *   a JSON object suitable for scene or prefab storage.
	 *
	 * \param[in] registry
	 *   ECS registry containing the entity.
	 * \param[in] ent
	 *   Entity to serialize.
	 * \param[in] serializedIdx
	 *   Index used to uniquely identify the serialized entity.
	 * \param[in,out] doc
	 *   RapidJSON document used for memory allocation.
	 * \param[in] serializeAction
	 *   Serialization mode (e.g. scene, prefab, override).
	 *
	 * \return
	 *   JSON value representing the serialized entity.
	 */
	static rapidjson::Value SerializeEntityRJson(Registry& registry,
		Registry::Entity ent,
		unsigned serializedIdx,
		rapidjson::Document& doc,
		SERIALIZE_ACTION serializeAction);

	/*!
	 * \brief
	 *   Deserializes an entity and its components from a RapidJSON object.
	 *
	 *   Reconstructs or updates an entity using serialized component data.
	 *
	 * \param[in,out] registry
	 *   ECS registry where the entity will be populated.
	 * \param[in] ent
	 *   Target entity to deserialize into.
	 * \param[in] entObj
	 *   JSON object containing serialized entity data.
	 * \param[in] offset
	 *   Entity offset used for remapping entity references.
	 * \param[in] deserializeAction
	 *   Deserialization mode (e.g. scene load, prefab instantiation).
	 */
	static void DeserializeEntityRJson(Registry& registry,
		Registry::Entity ent,
		rapidjson::Value& entObj,
		Registry::Entity offset,
		DESERIALIZE_ACTION deserializeAction);

	/*!
	 * \brief
	 *   Serializes an entity hierarchy starting from a root entity.
	 *
	 *   Recursively serializes the entity and all its children into the document.
	 *
	 * \param[in] registry
	 *   ECS registry containing the entity hierarchy.
	 * \param[in] ent
	 *   Root entity of the hierarchy.
	 * \param[in,out] doc
	 *   RapidJSON document used for memory allocation.
	 * \param[in] prefab
	 *   Serialization mode controlling hierarchy behavior.
	 */
	static void SerializeEntityHierarchy(Registry& registry,
		Registry::Entity ent,
		rapidjson::Document& doc,
		SERIALIZE_ACTION prefab);

	/*!
	 * \brief
	 *   Deserializes an entity hierarchy from a RapidJSON document.
	 *
	 *   Recreates entities and their parent-child relationships.
	 *
	 * \param[in,out] registry
	 *   ECS registry where entities will be created.
	 * \param[in] ent
	 *   Root entity to attach the hierarchy to.
	 * \param[in] doc
	 *   JSON document containing serialized hierarchy data.
	 * \param[in] deserializeAction
	 *   Deserialization mode controlling how entities are instantiated.
	 *
	 * \return
	 *   Vector of newly created entities in the hierarchy.
	 */
	static std::vector<Registry::Entity> DeserializeEntityHierarchy(
		Registry& registry,
		Registry::Entity ent,
		rapidjson::Document& doc,
		DESERIALIZE_ACTION deserializeAction);

	/*!
	 * \brief
	 *   Serializes an entire scene into a RapidJSON document.
	 *
	 *   Converts all entities and their components into a JSON representation
	 *   and prepares it for saving to disk.
	 *
	 * \param[in] registry
	 *   ECS registry containing the scene entities.
	 * \param[in] sceneName
	 *   Name of the scene to serialize.
	 * \param[in] basepath
	 *   Directory where the scene file will be saved.
	 *
	 * \return
	 *   RapidJSON document representing the serialized scene.
	 */
	static rapidjson::Document SerializeSceneRJson(
		Registry& registry,
		std::string const& sceneName,
		std::string const& basepath = "Assets\\Scenes\\");

	/*!
	 * \brief
	 *   Deserializes a scene from disk into the ECS registry.
	 *
	 *   Loads entities, components, and hierarchies from a scene file.
	 *
	 * \param[in,out] registry
	 *   ECS registry where the scene will be loaded.
	 * \param[in] sceneName
	 *   Name of the scene to load.
	 * \param[in] basepath
	 *   Directory containing the scene file.
	 * \param[in] isPush
	 *   If true, loads the scene additively instead of replacing existing entities.
	 * \param[in] offset
	 *   Entity offset used for remapping entity references.
	 */
	static void DeserializeSceneRJson(Registry& registry,
		std::string const& sceneName,
		std::string const& basepath = "Assets\\Scenes\\",
		bool isPush = false,
		unsigned offset = 0);

	/*!
	 * \brief
	 *   Copies all components from one entity to another.
	 *
	 *   Performs a deep copy of component data without affecting hierarchy.
	 *
	 * \param[in,out] registry
	 *   ECS registry containing the entities.
	 * \param[in] source
	 *   Entity to copy components from.
	 * \param[in] target
	 *   Entity to copy components to.
	 */
	static void CopyEntity(Registry& registry,
		Registry::Entity source,
		Registry::Entity target);


	/*!
	* \brief
	*   Queue scene specified by name to be loaded at the end of frame
	*
	* \param[in] sceneName
	*   name of scene to be loaded
	*/
	static void QueueSceneAction(std::string sceneName, SCENE_ACTION action);

	void PostRenderUpdate();

	const std::stack<std::string>& SceneStack() const { return sceneName; }
	const std::string& BaseScene() const { return baseScene; }

private:
	/*!
	* \brief
	*   Deleted copy constructor to prevent copying of singleton instance.
	*/
	SceneManager(const SceneManager&) = delete;
	
	/*!
	* \brief
	*   Deleted assignment operator to prevent copying of singleton instance.
	*/
	SceneManager& operator=(const SceneManager&) = delete;



	/*!
	* \brief
	*   Initializes the SceneManager and prepares it for scene loading operations.
	*/
	void Init();

	/*!
	* \brief
	*   Cleans up resources and shuts down the SceneManager.
	*/
	void Free();

	/*!
	* \brief
	*   Clear current entity and component registry and load everything from
	*	scene specified by name
	*
	* \param[in] sceneName
	*   name of scene to be loaded
	*/
	void ChangeScene(std::string sceneName);

	/*!
	 * \brief
	 *   Saves a RapidJSON document to disk as a scene file.
	 *
	 *   Writes the serialized scene data contained in the document to a file
	 *   using the specified scene name and base path.
	 *
	 * \param[in] doc
	 *   RapidJSON document containing serialized scene data.
	 * \param[in] sceneName
	 *   Name of the scene file (without extension).
	 * \param[in] basepath
	 *   Directory where the scene file will be saved.
	 */
	static void SaveToFile(rapidjson::Document& doc,
		std::string const& sceneName,
		std::string const& basepath = "Assets\\Scenes\\");

	/*!
	 * \brief
	 *   Converts an existing entity handle into a new serialized entity index.
	 *
	 *   Searches for the entity in the provided entity vector and returns its
	 *   corresponding 1-based index. Returns 0 if the entity is invalid or not found.
	 *
	 *   This function is typically used during serialization to remap entity
	 *   references into a compact index-based format.
	 *
	 * \param[in] registry
	 *   ECS registry containing the entity.
	 * \param[in] entVec
	 *   Vector of entities used as the serialization reference list.
	 * \param[in] ent
	 *   Entity to convert into a serialized index.
	 *
	 * \return
	 *   1-based serialized entity index, or 0 on failure.
	 */
	static Registry::Entity getNewEntIndex(
		std::vector<Registry::Entity> entVec,
		Registry::Entity ent)
	{
		if (ent)
		{
			if (auto it = std::find(entVec.begin(), entVec.end(), ent);
				it != entVec.end())
			{
				return static_cast<Registry::Entity>(
					std::distance(entVec.begin(), it) + 1);
			}
			else
			{
				LOGE("Error in converting entity index");
			}
		}
		return 0;
	};


	SCENE_ACTION nextSceneAction{ NONE };
	std::string nextSceneName;
	std::string baseScene;
	std::stack<std::string> sceneName;
	std::map<std::string, std::vector<Registry::Entity>> sceneCache; // Stores the IDs of the entities for a scene
	
	std::pair<std::string,rapidjson::Document> currentScene;

	bool exitSignal = false;
public:
	bool isSceneChanged = false;
};