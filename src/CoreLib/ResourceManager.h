#pragma once
/**___________________________________________________________________________/
@file          ResourceManager.h
@author        j.junbo@digipen.edu(80%)
@co-author	   yukang.ou@digipen.edu(20%)
@date          9/29/2025

A singleton resourcemanager class that handles the loading and unloading of
all the resources in the game with a simple interface. Yukang implemented
the animation integration


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "rapidjson/document.h"
#include "audio.h"
#include "texture.h"
#include "font.h"
#include "Animation.h"
#include "Tileset.h"
#include "glslshader.h"
#include <map>
#include <string>
#include "Json.h"
#include <memory>
#include <queue>

#include <unordered_map>
#include "Registry.h"

/*!
* \brief
*   Singleton class to manage resources.
* \brief
*	Just use the Get() for the resource you need and the rest will be handled by the resource manager
*/
class ResourceManager {
private:

public:
	ResourceManager() = default;

	~ResourceManager() = default;

	/*!
	* \brief
	*   Initializes whatever is required for loading of resources
	*
	* \param null
	* 
	* \return null
	*/
	void Init();

	/*!
	* \brief
	*   Frees the loaded resources
	*
	* \param null
	*
	* \return null
	*/
	void Free();

	/*!
	* \brief
	*   Runs in the update loop
	*
	* \param null
	*
	* \return null
	*/
	void Update(); 

	//ResourceManager& operator= (const ResourceManager) = delete;
	//ResourceManager(const ResourceManager&) = delete;


	/*!
	* \brief
	*   Gets a texture obj with the given name
	*
	* \param [std::string&] the name of the file (without the filepath)
	*
	* \return [TextureObj&] a reference to the texture object. Returns a default error tex of loading fails
	*/
	TextureObj& GetTexture(const std::string& name);

	std::string GetTexPath(TextureObj const& obj);

	/*!
	* \brief
	*   Gets the default error tex
	*
	* \param null
	*
	* \return [TextureObj&] a reference to the default error texture
	*/
	TextureObj& GetErrorTex();

	/*!
	* \brief
	*   Gets a font obj with the given name
	*
	* \param [std::string&] the name of the file (without the filepath)
	* \param [unsigned int = 32] the required resolution for the font. Default value of 64.
	*
	* \return [FontObj&] a reference to the font object. Returns a default initialized fontobj on failure
	*/
	FontObj& GetFont(const std::string& name, unsigned int resolution = 64U);

	/*!
	* \brief
	*   Gets a glslshader with the given name. The vertex and fragment file will be presumed to have the same name
	*
	* \param [std::string&] the name of the file (without the filepath and the extension)
	*
	* \return [GLSLShader&] a reference to the shader. Returns a default initialized GLSLShader on failure
	*/
	GLSLShader& GetShader(const std::string& name);

	/*!
	* \brief
	*   Gets a glslshader with the vertex and fragment shader filename
	*
	* \param [std::string&] the name of the file (without the filepath)
	*
	* \return [GLSLShader&] a reference to the shader. Returns a default initialized GLSLShader on failure
	*/
	GLSLShader& GetShader(const std::string& vert, const std::string& frag);

	/*!
	* \brief
	*   Gets an Animation with the given name
	*
	* \param [std::string&] the name of the file (without the filepath and the extension)
	*
	* \return [Animation&] a reference to the animation. Returns a default initialized GLSLShader on failure
	*/
	Animation& GetAnimation(const std::string& name);

	/*!
	* \brief
	*   Gets a tileset with the given name
	*
	* \param [std::string&] the name of the file (without the filepath)
	*
	* \return [Tileset&] a reference to the tileset. Returns a default initialized tileset on failure
	*/
	std::shared_ptr<Tileset> GetTileset(const std::string& name);

	/*!
	* \brief
	*   Saves a tileset with the given name
	*
	* \param [Tileset const&] the tileset to be saved
	* 
	* \param [std::string&] the name of the file (without the filepath)
	*
	* \return [FontObj&] a reference to the font object. Returns a default initialized fontobj on failure
	*/
	bool SaveTileset(Tileset const& tileset,std::string name);

	/*!
	* \brief
	*   loads all entities in a scene from a json file with the given name
	*
	* \param [std::string&] the name of the file (without the filepath and the extension)
	*
	* \return returns true if scene loaded successfully, false otherwise
	*/
	//bool LoadSceneEntities(Registry& registry, ComponentRegistry& componentRegistry,const std::string& sceneName, const std::string& projectName="project");

	/*!
	* \brief
	*   loads prefab from a json file with the given name
	*
	* \param [Registry&]
	*   ECS Registry to load the prefab into
	*
	* \param [std::string&]
	*   the name of the prefab (without the filepath and the extension)
	*
	* \return
	*   returns entity ID of the prefab create, if prefab loading fails,
	*   returns 0 (invalid entity)
	*/
	Registry::Entity InstantiatePrefab(Registry& registry, const std::string& prefabName, Registry::Entity ent = 0);

	/*!
	* \brief
	*   loads a dummy prefab from a json file with the given name
	*
	* \param [Registry&]
	*   ECS Registry to load the dummy prefab into
	*
	* \param [std::string&]
	*   the name of the prefab (without the filepath and the extension)
	*
	* \return
	*   returns entity ID of the dummy prefab created, if loading fails,
	*   returns 0 (invalid entity)
	*/
	Registry::Entity LoadDummyPrefab(Registry& registry, const std::string& prefabName, Registry::Entity ent = 0);

	/*!
	* \brief
	*   overwrites an existing prefab json file with updated data
	*
	* \param [Registry&]
	*   ECS Registry containing the dummy prefab entity
	*
	* \param [std::string&]
	*   the name of the prefab (without the filepath and the extension)
	*
	* \param [Registry::Entity]
	*   root entity of the dummy prefab to serialize
	*/
	void OverwritePrefab(Registry& registry, const std::string& prefabName, Registry::Entity dummy, std::string const& basepath = "Assets\\Prefabs\\");

	/*!
	* \brief
	*   Gets a const reference to the underlying texture storage
	*
	* \param null
	*
	* \return [std::map<std::string, TextureObj>&] a reference to the underlying storage
	*/
	const std::map<std::string, TextureObj>& TextureStorage() const noexcept { return TextureStore; }

	/*!
	* \brief
	*   Gets a const reference to the underlying font storage
	*
	* \param null
	*
	* \return [std::map<std::string, FontObj>&] a reference to the underlying storage
	*/
	const std::map<std::string, FontObj>& FontStorage() const noexcept { return FontStore; }

	/*!
	* \brief
	*   Gets a const reference to the underlying shader storage
	*
	* \param null
	*
	* \return [std::map<std::pair<std::string, std::string>, GLSLShader>&] a reference to the underlying storage
	*/
	const std::map<std::pair<std::string, std::string>, GLSLShader>& ShaderStorage() const noexcept { return ShaderStore; }

	/*!
	* \brief
	*   Gets a const reference to the underlying audio storage
	*
	* \param null
	*
	* \return [std::map<std::string, AudioObj>&] a reference to the underlying storage
	*/
	const std::map<std::string, AudioObj>& AudioStorage() const noexcept { return AudioStore; }

	/*!
	* \brief
	*   Gets a const reference to the underlying animation storage
	*
	* \param null
	*
	* \return [std::map<std::string, Animation>&] a reference to the underlying storage
	*/
	const std::map<std::string, Animation>& AnimationStorage() const noexcept { return AnimationStore; }
	
	/*!
	* \brief
	*   Gets a const reference to the underlying prefab storage
	*
	* \param null
	*
	* \return [std::map<std::string, rapidjson::Document>&] a reference to the underlying storage
	*/
	const std::map<std::string, rapidjson::Document>& PrefabStorage() const noexcept { return prefabStore; }

	/*!
	* \brief
	*   Gets a AudioObj with the given name.
	*
	* \param [std::string&] the name of the file (without the filepath)
	*
	* \return [AudioObj&] a reference to the AudioObj. Returns a default initialized AudioObj on failure
	*/
	AudioObj& GetAudio(const std::string& name, std::string type = "SFX", bool loop = false, bool stream = false);

	/*!
	* \brief
	*   Sets the volume, pitch, and mute state of an audio group.
	*
	* \param [const std::string&] groupName - the name of the audio group
	* \param [float] volume - the volume multiplier for the group
	* \param [float] pitch - the pitch multiplier for the group
	* \param [bool] mute - whether the group should be muted
	*
	* \return null
	*/
	void SetGroupVars(const std::string& groupName, float volume = 1.f, float pitch = 1.f, bool mute = false);

	/*!
	* \brief
	*   Retrieves the volume level of an audio group.
	*
	* \param [const std::string&] groupName - the name of the audio group
	*
	* \return [float] the current volume multiplier of the group
	*/
	float GetGroupVolume(const std::string& groupName) const;

	/*!
* \brief
*   Retrieves the pitch multiplier of an audio group.
*
* \param [const std::string&] groupName - the name of the audio group
*
* \return [float] the current pitch multiplier of the group
*/
	float GetGroupPitch(const std::string& groupName) const;

	/*!
* \brief
*   Retrieves the mute state of an audio group.
*
* \param [const std::string&] groupName - the name of the audio group
*
* \return [bool] true if the group is muted, false otherwise
*/
	bool GetGroupMute(const std::string& groupName) const;
	/*!
* \brief
*   Pauses or resumes all currently playing audio.
*
* \param [bool] pause - whether all audio should be paused
*
* \return null
*/
	void PauseAllAudio(bool pause);

	/*!
* \brief
*   Stops all currently playing audio immediately.
*
* \param null
*
* \return null
*/
	void StopAllAudio();

	/*!
* \brief
*   Updates the background music queue, handling fade-out of the current track
*   and fade-in of the next queued track.
*
* \param [float] deltaTime - time elapsed since the previous frame
*
* \return null
*/
	void UpdateBGMQueue(float deltaTime);
	/*!
* \brief
*   Adds a background music track to the playback queue.
*
* \param [std::string] name - the name or filepath of the background music
* \param [float] fadeInTimer - the duration of the fade-in effect
* \param [float] FadeOutTimer - the duration of the fade-out effect
*
* \return null
*/
	void QueueBGM(std::string name, float fadeInTimer = 0.5f, float FadeOutTimer = 0.5f);
	/*!
* \brief
*   Gets the current number of background music tracks in the queue.
*
* \param null
*
* \return [size_t] the number of queued background music tracks
*/
	size_t QueueSize() const;

	/*!
* \brief
*   Clears the background music queue and resets all related state flags.
*
* \param null
*
* \return null
*/ 
	void ClearBGMQueue();

private:

	/*!
	* \brief
	*   private function to load in the shader if not yet loaded
	*/
	bool LoadShader(const std::pair<std::string, std::string>& name);

	/*!
	* \brief
	*   private function to load in the animation if not yet loaded
	*/
	bool LoadAnimation(const std::string& name);

	/*!
	* \brief
	*   special function to load in the default error texture
	*/
	void LoadError();

	std::map<std::string, TextureObj> TextureStore{};
	std::map<std::string, FontObj> FontStore{};
	std::map<std::pair<std::string,std::string>, GLSLShader> ShaderStore{};
	std::map<std::string, AudioObj> AudioStore{};
	std::map<std::string, Animation> AnimationStore{};
	std::map<std::string, rapidjson::Document> prefabStore{};
	std::map<std::string, std::shared_ptr<Tileset>> tilesetStore{};

	struct BGMEntry {
		std::string path;
		float fadeInDuration;
		float fadeOutDuration;
	};

	std::queue<BGMEntry> queuedBGM;
	bool isInitializedBGM = false;
	bool fadeInCompleted = false;
	bool fadeOutCompleted = false;
	FMOD::Channel* channel;
};