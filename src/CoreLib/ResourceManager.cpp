/**___________________________________________________________________________/
@file          ResourceManager.cpp
@author        j.junbo@digipen.edu(30%)
@co-author	   yukang.ou@digipen.edu(35%)
			   t.junjie@digipen.edu(35%)
@date          9/29/2025

A singleton resourcemanager class that handles the loading and unloading of
all the resources in the game with a simple interface. Yukang implemented
the animation loading, font loading. JunJie implemented texture loading.
General resourcemanager structure done by junbo


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "Platform.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "Components.h"

namespace {
#ifdef PLATFORM_WINDOWS
	const std::string AssetFilepath("Assets\\");
#endif
#ifdef PLATFORM_ANDROID
	const std::string AssetFilepath("");

	void AndroidMakesMeMad(std::string& s) {
		std::replace(s.begin(),s.end(),'\\','/');
	}
#endif
	const std::string TexPath("Textures\\");
	const std::string FontPath("Fonts\\");
	const std::string ShaderPath("Shaders\\");
	const std::string AudioPath("Sounds\\");
	const std::string AnimPath("Animations\\");
	const std::string TilesetPath("Tilesets\\");
	const std::string ErrorTex("ERROR");

	constexpr size_t errorwidth{ 160 };
}


static bool init{ false }; // To make sure this only runs once
void ResourceManager::Init() {
	if (init) 
	{
		LOGE("Initializing ResouceManager stopped, already initialized");
		return;
	}
	LOGI("Initializing ResouceManager");

	// init stuff

	CEO::Instance().GetManager<TextureManager>()->Init();
	CEO::Instance().GetManager<FontManager>()->Init();

	if (!CEO::Instance().GetManager<AudioManager>()->Init()) {
		LOGE("AudioManager init failed, audio will be disabled");
	}


	LoadError(); // loading error texture


	init = true;
}

void ResourceManager::Free() {
	LOGI("Cleaning ResourceManager");
	// free whatever that needs to be freed
	LOGI("Cleaning Textures");
	for (std::pair<const std::string, TextureObj>& p : TextureStore) {
		CEO::Instance().GetManager<TextureManager>()->FreeTex(p.second);
	}
	TextureStore.clear();

	LOGI("Cleaning Fonts");
	for (std::pair<const std::string, FontObj>& p : FontStore) {
		CEO::Instance().GetManager<FontManager>()->FreeFont(p.second);
	}
	FontStore.clear();

	LOGI("Cleaning Audio");
	for (std::pair<const std::string, AudioObj>& p : AudioStore) {
		CEO::Instance().GetManager<AudioManager>()->FreeAudio(p.second);
	}
	AudioStore.clear();

	LOGI("Cleaning Shaders");
	for (std::pair<const std::pair<std::string,std::string>,GLSLShader> &p : ShaderStore) {
		p.second.DeleteShaderProgram();
	}
	ShaderStore.clear();

	AnimationStore.clear();
	
	CEO::Instance().GetManager<AudioManager>()->Free();
	CEO::Instance().GetManager<FontManager>()->Free();
	CEO::Instance().GetManager<TextureManager>()->Free();
	
	init = false;
}

void ResourceManager::Update() {
	CEO::Instance().GetManager<AudioManager>()->Update();
}

TextureObj& ResourceManager::GetTexture(const std::string& name) {
	std::string filename = AssetFilepath + TexPath + name;
#ifdef PLATFORM_ANDROID
	AndroidMakesMeMad(filename);
#endif
	// If obj isnt loaded
	if (TextureStore.find(filename) == TextureStore.end() ||
		!TextureStore[filename].IsLoaded()) {

		TextureStore.insert_or_assign(filename, CEO::Instance().GetManager<TextureManager>()->LoadTex(filename));

		if (!TextureStore[filename].IsLoaded())
			return TextureStore[ErrorTex];
	}

	return TextureStore[filename];

}
std::string ResourceManager::GetTexPath(TextureObj const& obj) {
	for (auto const& [path, texture] : TextureStore) {
		if (texture.TexId() == obj.TexId()) {
			return std::string{path};
		}
	}
	return std::string{"ERROR"};
}


TextureObj& ResourceManager::GetErrorTex() { return TextureStore[ErrorTex]; }

void ResourceManager::LoadError() {
	unsigned char* img = new unsigned char[errorwidth * errorwidth * 3];
	// bottom layer
	img[0] = 0;     img[1] = 0;    img[2] = 0;
	img[240] = 240; img[241] = 52; img[242] = 236;

	for (size_t i{ 1 }; i < errorwidth / 2; i++) {
		std::memcpy(img + i * 3, img, 3);
		std::memcpy(img + 240 + i * 3, img + 240, 3);
	}

	// bottom half
	for (size_t i{ 1 }; i < errorwidth / 2; i++) {
		std::memcpy(img + i * errorwidth * 3, img, errorwidth * 3);
	}

	// top layer
	size_t topstart = 3 * errorwidth * errorwidth / 2;
	img[topstart] = 240;     img[topstart + 1] = 52;  img[topstart + 2] = 236;
	img[topstart + 240] = 0; img[topstart + 241] = 0; img[topstart + 242] = 0;

	for (size_t i{ 1 }; i < errorwidth / 2; i++) {
		std::memcpy(img + topstart + i * 3, img + topstart, 3);
		std::memcpy(img + topstart + 240 + i * 3, img + topstart + 240, 3);
	}

	// top half
	for (size_t i{ 1 }; i < errorwidth / 2; i++) {
		std::memcpy(img + topstart + i * errorwidth * 3, img + topstart, errorwidth * 3);
	}

	GLuint texId{};

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, errorwidth, errorwidth, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	delete[] img;

	TextureObj obj(texId, errorwidth, errorwidth, GL_FALSE, GL_TRUE);
	TextureStore[ErrorTex] = std::move(obj);
}

FontObj& ResourceManager::GetFont(const std::string& name, unsigned int resolution) {
	std::string filename = AssetFilepath + FontPath + name;
#ifdef PLATFORM_ANDROID
	AndroidMakesMeMad(filename);
#endif
	std::string realname = filename + std::to_string(resolution);
	if (FontStore.find(realname) == FontStore.end() ||
		!FontStore[realname].loaded) {
		FontStore.insert_or_assign(realname, CEO::Instance().GetManager<FontManager>()->LoadFont(filename, resolution));
	}

	return FontStore[realname];

}

GLSLShader& ResourceManager::GetShader(const std::string& name) {
	return GetShader(name + ".vert", name + ".frag");
}

GLSLShader& ResourceManager::GetShader(const std::string& vert, const std::string& frag) {
	std::string vertname = AssetFilepath + ShaderPath + vert;
	std::string fragname = AssetFilepath + ShaderPath + frag;
#ifdef PLATFORM_ANDROID
	AndroidMakesMeMad(vertname);
	AndroidMakesMeMad(fragname);
#endif
	std::pair<std::string, std::string> name{ std::make_pair(vertname,fragname) };
	if (ShaderStore.find(name) == ShaderStore.end()) {
		LoadShader(name);
	}
	return ShaderStore[name];
}

bool ResourceManager::LoadShader(const std::pair<std::string, std::string>& filepath) {
	std::vector<GLSLShader::ShaderPair> shds = { std::make_pair(GL_VERTEX_SHADER, filepath.first),
	std::make_pair(GL_FRAGMENT_SHADER, filepath.second) };

	if (GL_FALSE == ShaderStore[filepath].CompileLinkValidate(shds, true)) {
		LOGE("Shader compilation failed: %s", ShaderStore[filepath].GetLog().c_str());
		return false; 
	}
	return true;
}

Animation& ResourceManager::GetAnimation(const std::string& name)
{

	if (AnimationStore.find(name) == AnimationStore.end()) {
		
		if (!LoadAnimation(name)) {
			if (AnimationStore.find("error") == AnimationStore.end()) {
				AnimationStore.emplace("error", Animation{});
			}
			else {
				return AnimationStore.at("error");
			}
		}
	}

	return AnimationStore[name];
}

std::shared_ptr<Tileset> ResourceManager::GetTileset(const std::string& name)
{
	if (tilesetStore.find(name) == tilesetStore.end()) {

		std::string filename = AssetFilepath + TilesetPath + name;
#ifdef PLATFORM_ANDROID
		AndroidMakesMeMad(filename);
#endif
		std::optional<Tileset> tileSet = TilesetManager::DeserializeTileset(filename);
		if (!tileSet) {
			if (tilesetStore.find("error") == tilesetStore.end()) {
				tilesetStore.emplace("error", std::make_shared<Tileset>());
			}
			else {
				return tilesetStore.at("error");
			}
		}
		else
		{
			tileSet->path = name;
			tilesetStore.insert_or_assign(name, std::make_shared<Tileset>(std::move(tileSet.value())));
		}
	}

	return tilesetStore[name];
}

bool ResourceManager::SaveTileset(Tileset const& tileset, std::string name)
{
	if (name.find(".tileset") == std::string::npos)
	{
		name = name + ".tileset";
	}
	rapidjson::Document json = TilesetManager::SerializeTileset(tileset);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	json.Accept(writer);

	std::ofstream ofs(AssetFilepath + TilesetPath + name);
	if (!ofs) {
		LOGE("Error in saving %s", name.c_str());
		return false;
	}
	ofs << buffer.GetString();
	ofs.close();
	CEO::Get<EventsDispatcher>()->Dispatch<Events::SyncFile>(Events::SyncFile{ AssetFilepath + TilesetPath + name,"tilesets"});
	
	// new tileset, add to store
	if (tilesetStore.find(name) == tilesetStore.end())
	{
		tilesetStore[name] = std::make_shared<Tileset>(tileset);
		tilesetStore[name]->path = name;
	}

	return true;
}

bool ResourceManager::LoadAnimation(const std::string& name)
{
	LOGI("Loading animation: %s", name.c_str());
	std::string filename = AssetFilepath + AnimPath + name;
#ifdef PLATFORM_ANDROID
	AndroidMakesMeMad(filename);
#endif
	json j(filename);
	Animation anim;
	json::value* buf;

	anim.animName = name;

	if (buf = j.GetValue("spriteSheetName"); buf)
		anim.spriteSheetName = *buf->GetString();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "spriteSheetName",name.c_str());return false;}

	if (buf = j.GetValue("frameDelay"); buf)
		anim.frameDelay = *buf->GetFloat();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "frameDelay",name.c_str());return false;}

	if (buf = j.GetValue("frameCount"); buf)
		anim.frameCount = *buf->GetInt();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "frameCount",name.c_str());return false;}

	if (buf = j.GetValue("rows"); buf)
		anim.rows = *buf->GetInt();	
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "rows",name.c_str());return false;}

	if (buf = j.GetValue("cols"); buf)
		anim.cols = *buf->GetInt();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "cols",name.c_str());return false;}

	if (buf = j.GetValue("sheetOrigin"); buf)
		anim.sheetOrigin.first = *buf->GetIntVec()[0];
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "sheetOrigin",name.c_str());return false;}

	if (buf = j.GetValue("sheetOrigin"); buf)
		anim.sheetOrigin.second = *buf->GetIntVec()[1];
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "sheetOrigin",name.c_str());return false;}
	
	if (buf = j.GetValue("sheetSize"); buf)
	{
		anim.sheetSize.first = *buf->GetIntVec()[0];
		anim.sheetSize.second = *buf->GetIntVec()[1];

	}
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "sheetSize",name.c_str());return false;}
	
	if (buf = j.GetValue("doLooping"); buf)
		anim.doLooping= *buf->GetBool();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "doLooping",name.c_str());return false;}

	if (buf = j.GetValue("playOnStart"); buf)
		anim.playOnStart = *buf->GetBool();
	else{LOGE("Failed to load property \"%s\" in animation \"%s\"", "playOnStart",name.c_str());return false;}

	AnimationStore[name] = std::move(anim);
	return true;
}

//bool ResourceManager::LoadSceneEntities(Registry & registry, ComponentRegistry & componentRegistry, const std::string & sceneName, const std::string & projectName){
//	(void)componentRegistry;
//	json j(AssetFilepath + projectName + "_EntityRegistries.json");
//	if (std::vector<json::object*> objVec = j.GetObjVec(sceneName); objVec.size()) {
//		CEO::Instance().GetManager<SceneManager>()->LoadSceneEntities(registry, objVec);
//		return true;
//	}
//	else {
//		LOGE("Scene %s not found in project %s's entity registry file", sceneName.c_str(), projectName.c_str());
//		//std::cout << "Scene " << sceneName << " not found in project " << projectName << "'s entity registry file.\n";
//		return false;
//	}
//}

Registry::Entity ResourceManager::InstantiatePrefab(Registry& registry, const std::string& prefabName, Registry::Entity ent) {
	auto prefab = prefabStore.find(prefabName);
	if (prefab == prefabStore.end()) {
		std::optional<rapidjson::Document> rawPrefab = std::move(PrefabManager::LoadPrefab(prefabName));
		if (rawPrefab.has_value()) {
			prefabStore.emplace(prefabName, std::move(rawPrefab.value()));
			prefab = prefabStore.find(prefabName);
		}
		else {
			LOGE("Prefab %s not found in prefab folder", prefabName.c_str());
			return 0; // return invalid entity
		}
	}
	if (!ent) ent = registry.CreateEntity();
	SceneManager::DeserializeEntityHierarchy(registry, ent, prefab->second, SceneManager::DESERIALIZE_ACTION::LOAD_PREFAB);
	PrefabManager::AddPrefabComp(ent, prefabName);
	if (!ent) {
		LOGE("Prefab file %s is corrupted", prefabName.c_str());
		return 0; // return invalid entity
	}

	return ent;
}

Registry::Entity ResourceManager::LoadDummyPrefab(Registry& registry, const std::string& prefabName, Registry::Entity dummyEnt) {
	auto prefab = prefabStore.find(prefabName);
	if (prefab == prefabStore.end()) {
		std::optional<rapidjson::Document> rawPrefab = std::move(PrefabManager::LoadPrefab(prefabName));
		if (rawPrefab.has_value()) {
			prefabStore.emplace(prefabName, std::move(rawPrefab.value()));
			prefab = prefabStore.find(prefabName);
		}
		else {
			LOGE("Prefab %s not found in prefab folder", prefabName.c_str());
		}
	}
	if (!dummyEnt) dummyEnt = registry.CreateEntity();

	if (SceneManager::DeserializeEntityHierarchy(registry, dummyEnt, prefab->second, SceneManager::DESERIALIZE_ACTION::LOAD_PREFAB).empty()) {
		LOGE("Prefab file %s is corrupted", prefabName.c_str());
		return 0; // return invalid entity
	};

	if (registry.GetComponent<HierarchyComponnent>(dummyEnt)->firstChild) {
		auto addDummyComp = [&registry = registry](Registry::Entity ent) {
			return registry.AddComponent<PrefabDummyMetatag>(ent, PrefabDummyMetatag{});
			};
		HierarchyManager::Traverse(registry, dummyEnt, addDummyComp);
	}
	else {
		registry.AddComponent<PrefabDummyMetatag>(dummyEnt, {});
	}
	return dummyEnt;
}

void ResourceManager::OverwritePrefab(Registry& registry, const std::string& prefabName, Registry::Entity dummy, std::string const& basepath) {
	auto prefab = prefabStore.find(prefabName);
	if (prefab == prefabStore.end()) {
		std::optional<rapidjson::Document> rawPrefab = std::move(PrefabManager::LoadPrefab(prefabName));
		if (rawPrefab.has_value()) {
			prefabStore.emplace(prefabName, std::move(rawPrefab.value()));
			prefab = prefabStore.find(prefabName);
		}
		else {
			PrefabManager::CreatePrefab(prefabName);
			return;
		}
	}
	if (
		prefab->second.HasMember("entities") == false || prefab->second.HasMember("hierarchy") == false ||
		!(prefab->second["entities"].IsArray() && prefab->second["hierarchy"].IsArray()) ||
		prefab->second["hierarchy"].GetArray().Size() != prefab->second["entities"].GetArray().Size()
		) {
		LOGE("Prefab file %s is corrupted", prefabName.c_str());
	}

	prefab->second.SetObject();
	SceneManager::SerializeEntityHierarchy(registry, dummy, prefab->second, SceneManager::SERIALIZE_ACTION::CREATE_PREFAB);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	prefab->second.Accept(writer);

	std::ofstream ofs(basepath + prefabName + ".prefab");
	if (!ofs) {
		LOGE("Error in creating %s.prefab", prefabName.c_str());
		return;
	}
	ofs << buffer.GetString();
	ofs.close();
	return;
}

AudioObj& ResourceManager::GetAudio(const std::string& name, std::string type, bool loop, bool stream){
	std::string filename = AssetFilepath + AudioPath + name;
#ifdef PLATFORM_ANDROID
	AndroidMakesMeMad(filename);
#endif
	if (AudioStore.find(filename) == AudioStore.end() ||
		!AudioStore[filename].IsActive()) {
		AudioStore.insert_or_assign(filename, CEO::Instance().GetManager<AudioManager>()->LoadAudio(filename,type,loop,stream));
	}

	return AudioStore[filename];
}

void ResourceManager::SetGroupVars(const std::string& groupName, float volume, float pitch, bool mute){
	CEO::Instance().GetManager<AudioManager>()->SetGroupVars(groupName, volume, pitch, mute);
}

float ResourceManager::GetGroupVolume(const std::string& groupName) const {
	return CEO::Instance().GetManager<AudioManager>()->GetGroupVol(groupName);
}

float ResourceManager::GetGroupPitch(const std::string& groupName) const {
	return CEO::Instance().GetManager<AudioManager>()->GetGroupPitch(groupName);
}

bool ResourceManager::GetGroupMute(const std::string& groupName) const {
	return CEO::Instance().GetManager<AudioManager>()->GetGroupMute(groupName);
}

void ResourceManager::PauseAllAudio(bool pause){
	CEO::Instance().GetManager<AudioManager>()->PauseAll(pause);
}

void ResourceManager::StopAllAudio(){
	CEO::Instance().GetManager<AudioManager>()->StopAll();
}

void ResourceManager::ClearBGMQueue()
{
	while (queuedBGM.size() > 0)
	{
		queuedBGM.pop();

	}
	fadeOutCompleted = false;
	fadeInCompleted = false;
	isInitializedBGM = false;
}

void ResourceManager::UpdateBGMQueue(float deltaTime)
{
	if (queuedBGM.size() >= 2)
	{
		//fades outs if queued bgm has more than 1, once fade out complete play new bgm
		if (!fadeOutCompleted && CEO::Instance().GetManager<AudioManager>()->FadeOutBGM(channel,queuedBGM.front().fadeOutDuration,deltaTime))
		{
			StopAllAudio();


			queuedBGM.pop();
			isInitializedBGM = true;
			fadeOutCompleted = true;

			fadeInCompleted = false;
			if(queuedBGM.front().fadeInDuration > 0)
				channel = GetAudio(queuedBGM.front().path,"BGM",true,true).Play(0.0f,1.0f);
		}
		else
		{
			fadeOutCompleted = false;

		}
	}
	else if(queuedBGM.size() == 1)
	{
		//if queue was empty and bgm is just pushed it will start from there 
		if (!isInitializedBGM)
		{

			channel = GetAudio(queuedBGM.front().path, "BGM", true, true).Play(0.0f,1.0f);
			isInitializedBGM = true;
			fadeInCompleted = false;

		}
		//fades in bgm
		if (!fadeInCompleted && CEO::Instance().GetManager<AudioManager>()->FadeInBGM(channel,queuedBGM.front().fadeInDuration, deltaTime))
		{
			
			fadeOutCompleted = false;

			fadeInCompleted = true;
		}
		else
		{

		}
	}
	else if (queuedBGM.size() == 0)
	{
		isInitializedBGM = false;
	}
}

void ResourceManager::QueueBGM(std::string name, float fadeInTimer, float FadeOutTimer)
{
	//queue bgm
	queuedBGM.push({ name, fadeInTimer, FadeOutTimer });
	fadeOutCompleted = false;

}
size_t ResourceManager::QueueSize() const
{
	// TODO: insert return statement here
	return queuedBGM.size();
}

