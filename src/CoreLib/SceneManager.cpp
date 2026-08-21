/*!
@file       SceneManager.cpp
@author     Kaeden Tan (kaedenjiawei.tan) 100%
@date       07/10/2025
@brief      Implementation of the SceneManager class, which handles loading
            and managing scene entities from JSON data into the ECS registry.
            Contains the core deserialization logic for converting JSON scene
            data into entities with their respective components.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#define RAPIDJSON_HAS_STDSTRING 1
#include "pch.h"
#include "SceneManager.h"
#include "components.h"
#include "ScriptingAPI.h"
#include "UpdateStackManager.h"
#undef GetObject

void SceneManager::DeserializeEntityRJson(Registry & registry, Registry::Entity ent, rapidjson::Value& entObj, Registry::Entity offset, DESERIALIZE_ACTION deserializeAction) {
	// Iterate through all known component types.
	registry.AddComponent<HierarchyComponnent>(ent, {});
	registry.AddComponent<ActiveComponent>(ent, {});
	registry.AddComponent<NameComponent>(ent, {});
	registry.AddComponent<UpdateStackComponent>(ent, {});
	// Special handler for if entity is a prefab
	if (entObj.GetObject().HasMember("PrefabComponent") && 
		entObj.GetObject()["PrefabComponent"].IsObject() &&
		entObj.GetObject()["PrefabComponent"].HasMember("name")&&
		entObj.GetObject()["PrefabComponent"]["name"].IsString() &&
		entObj.GetObject()["PrefabComponent"].HasMember("root") &&
		entObj.GetObject()["PrefabComponent"]["root"].IsBool()
		)
	{
		rapidjson::Value& rawPrefabObj = entObj.GetObject()["PrefabComponent"];
		std::string prefabName = rawPrefabObj["name"].GetString();
		if (prefabName.empty())
			return;
		CEO::Instance().GetManager<ResourceManager>()->InstantiatePrefab(registry, prefabName, ent);
		PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(ent);

		// for backwards compatibility
		if (rawPrefabObj.HasMember("overriddenComponents")) {
		// Deserialization of flags overriden components
			for (auto val = rawPrefabObj["overriddenComponents"].GetArray().begin(); val < rawPrefabObj["overriddenComponents"].GetArray().end(); val++) {
				if (!val->IsObject() ||
					!val->GetObject().HasMember("component") || !val->GetObject().HasMember("overrideFlag") ||
					!val->GetObject()["component"].IsString() || !val->GetObject()["overrideFlag"].IsUint()) {
					LOGE("Prefab component invalid at %d", ent);
					continue;
				}
				rtr::TypeInfo overriddenTi = rtr::TypeInfo::GetByName(val->GetObject()["component"].GetString());

				if (!overriddenTi.isValid()) {
					LOGE("Prefab Override Component Type Invalid: %s, at %d", val->GetObject()["component"].GetString(), ent);
					continue;
				}
				prefabComp->overriddenComponents[val->GetObject()["component"].GetString()]
					= static_cast<rtr::TypeInfo::MemberFlag>(val->GetObject()["overrideFlag"].GetUint());
			};
		}
		// Deserialize entity hierarchy if it exists (for loading of scene)
 		if (entObj.GetObject().HasMember("HierarchyComponnent")) {	
			rapidjson::Value& hComp = entObj.GetObject()["HierarchyComponnent"];
			registry.GetComponent<HierarchyComponnent>(ent)->parent = hComp.GetObject()["parent"].GetUint();
			registry.GetComponent<HierarchyComponnent>(ent)->nextSibling = hComp.GetObject()["nextSibling"].GetUint();
		}

	};
	// Loop through each component
	for (rapidjson::Value::ConstMemberIterator compObj = entObj.GetObject().MemberBegin();
		 compObj != entObj.GetObject().MemberEnd(); ++compObj)
	{ 
		if (compObj->value.IsObject()) {
			// Gets typeinfo and check if it is serializable
			rtr::TypeInfo ti = rtr::TypeInfo::GetByName(std::string{ compObj->name.GetString() });
			if (!ti.isValid()) {
				LOGE("Component type at %d, %s is not registered in the component loader", ent, compObj->name.GetString());
				return;
			}
			else if (!ti.IsSerializable()) {
				continue;
			}

			// Gets registry instance of type
			ComponentRegistry& componentRegistry = *CEO::Instance().GetManager<ComponentRegistry>();
			rtr::Instance inst = registry.GetComponentsRTR(ent, ti);

			// If instance cannot be gotten, create new component and get instance again
			if (!inst.isValid()) {
				componentRegistry.CreateComponent(compObj->name.GetString(), ent);
				inst = registry.GetComponentsRTR(ent, ti);
				if (!inst.isValid()) {
					LOGE("Could not get instance of component type at %d, %s", ent, compObj->name.GetString());
					return;
				}
			}

			// Loop through all memebers of component
			for (std::string memberName : inst.GetType().Members()) {
				switch (deserializeAction) {
				case DESERIALIZE_ACTION::LOAD_PREFAB:
					break;
				// If loading scene or copying entity, deserialize only overrident members of prefab instances
				case DESERIALIZE_ACTION::COPY:
				case DESERIALIZE_ACTION::LOAD_SCENE:
					PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(ent);
					if (prefabComp) {
						auto search = prefabComp->overriddenComponents.find(ti.Name());
						if (search == prefabComp->overriddenComponents.end()) {
							continue;
						}
						if (!(search->second & ti.GetMemberFlag(memberName))) {
							continue;
						}
					}
					break;
				}

				// error message construction
				std::string error =
					std::string("Entity deserialization error at ") +
					"Entity "+ std::to_string(ent)+
					"', Component: '" + compObj->name.GetString() + "," + memberName+
					"' is INVALID or CANNOT BE DESERIALIZED";
				// Get the type of the current member.
				std::type_index memberType{ inst.GetMemberType(memberName) };
				// Check if a value for this member exists in the JSON object.
				if (!compObj->value.GetObject().HasMember(memberName.c_str())){
					continue;
				}
				// deserializes memebers based on member type
				rapidjson::Value const& val = compObj->value.GetObject()[memberName.c_str()];
				if (!val.IsNull()) {
					if (std::type_index(typeid(std::string)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, std::string{ val.GetString() });
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(int)) == memberType) {
						if (val.IsInt())
							inst.SetVal(memberName, val.GetInt());
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(unsigned int)) == memberType) {
						if (val.IsUint())
							inst.SetVal(memberName, val.GetUint());
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(float)) == memberType) {
						if (val.IsFloat())
							inst.SetVal(memberName, val.GetFloat());
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(double)) == memberType) {
						if (val.IsDouble())
							inst.SetVal(memberName, val.GetDouble());
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(bool)) == memberType) {
						if (val.IsBool())
							inst.SetVal(memberName, val.GetBool());
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(GameObject)) == memberType) {
						GameObject go;
						if (val.IsUint())
							go = val.GetUint() + offset;
						else if (val.IsString())
							go = val.GetString();
						else
							LOGE("%s", error.c_str());

						if (go.IsValid())
							inst.SetVal(memberName, go);
				
					}
					else if (std::type_index(typeid(Shape)) == memberType) {
						if (val.IsInt())
							inst.SetVal(memberName, static_cast<Shape>(val.GetInt()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(TextRendererComponent::Alignment)) == memberType) {
						if (val.IsInt())
							inst.SetVal(memberName, static_cast<TextRendererComponent::Alignment>(val.GetInt()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(LightComponent::LightType)) == memberType) {
						if (val.IsInt())
							inst.SetVal(memberName, static_cast<LightComponent::LightType>(val.GetInt()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(Mat3)) == memberType) {
						// Create a temporary Mat3 object.
						Mat3 matTemp;
						// Validate that the array has 9 elements for a 3x3 matrix.
						if (!val.IsArray() || val.GetArray().Size() != 9) {
							LOGE("%s", error.c_str());
							continue;
						}
						// Populate the matrix from the float vector.
						for (int i{}; i < 9; i++) {
							if (val.GetArray()[i].IsFloat()) {
								matTemp.m[i] = val.GetArray()[i].GetFloat();
							}
							else {
								LOGE("%s", error.c_str());
							}
						}
						// Set the matrix value on the component instance.
						inst.SetVal(memberName, matTemp);
					}
					// If the member is a Vec2, deserialize it from a float array.
					else if (std::type_index(typeid(Vec2)) == memberType) {
						// Create a temporary Vec2 object.
						Vec2 vecTemp;
						// Validate that the array has 2 elements for a 2D vector.
						if (!val.IsArray() || val.GetArray().Size() != 2) {
							LOGE("%s", error.c_str());
							continue;
						}
						// Populate the vector from the float array.
						if (val.GetArray()[0].IsFloat() && val.GetArray()[1].IsFloat()) {
							vecTemp.x = val.GetArray()[0].GetFloat();
							vecTemp.y = val.GetArray()[1].GetFloat();
						}
						else {
							LOGE("%s", error.c_str());
						}
						// Set the vector value on the component instance.
						inst.SetVal(memberName, vecTemp);
					}
					else if (std::type_index(typeid(Color)) == memberType) {
						// Create a temporary Vec4 object.
						Color colorTemp;
						// Validate that the array has 4 elements for a color.
						if (!val.IsArray() || val.GetArray().Size() != 4) {
							LOGE("%s", error.c_str());
							continue;
						}
						// Populate the color from the float array.
						if (val.GetArray()[0].IsFloat() && val.GetArray()[1].IsFloat() && 
							val.GetArray()[2].IsFloat() && val.GetArray()[3].IsFloat()) {
							colorTemp.r = val.GetArray()[0].GetFloat();
							colorTemp.g = val.GetArray()[1].GetFloat();
							colorTemp.b = val.GetArray()[2].GetFloat();
							colorTemp.a = val.GetArray()[3].GetFloat();
						}
						else {
							LOGE("%s", error.c_str());
						}
						// Set the vector value on the component instance.
						inst.SetVal(memberName, colorTemp);
					}
					else if (std::type_index(typeid(Animation*)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(val.GetString()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(TextureObj*)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetTexture(val.GetString()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(AudioObj*)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetAudio(val.GetString()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(FontObj*)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetFont(val.GetString()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(std::shared_ptr<Tileset>)) == memberType) {
						if (val.IsString())
							inst.SetVal(memberName, CEO::Instance().GetManager<ResourceManager>()->GetTileset(val.GetString()));
						else
							LOGE("%s", error.c_str());
					}
					else if (std::type_index(typeid(std::map<ChunkCoordinates, TilemapComponent::TilemapChunk>)) == memberType) {
						if (val.IsObject())
						{
							auto& chunks = *inst.GetPtr< std::map<ChunkCoordinates, TilemapComponent::TilemapChunk>>(memberName);
							ChunkCoordinates chunkCoord;
							for (rapidjson::Value::ConstMemberIterator chunk = val.MemberBegin(); chunk != val.MemberEnd(); ++chunk)
							{
								std::stringstream stream{ chunk->name.GetString() };
								stream >> chunkCoord.chunkX;
								stream.ignore(1);
								stream >> chunkCoord.chunkY;
								int count = 0;
								for (rapidjson::Value::ConstValueIterator tile = chunk->value.Begin(); tile != chunk->value.End(); ++tile)
								{
									chunks[chunkCoord][count].spriteID = tile->GetInt();
									++count;
								}
							}

							inst.SetVal(memberName, std::move(chunks));
						}
						else
							LOGE("%s", error.c_str());
							}
					else if (std::type_index(typeid(unsigned long long)) == memberType) {
						if (val.IsUint64())
							inst.SetVal(memberName, val.GetUint64());
						else
							LOGE("%s", error.c_str());
					}
					// If the member type is unknown, throw an exception.
					else {
						LOGE("%s", error.c_str());
					}
				}
				//else if (obj->GetObjVec(prop.get_name().to_string()).size()) {
				//	prop.set_value(inst, obj->GetValue(prop.get_name().to_string()));
				//}
				//else if (obj->GetObj(prop.get_name().to_string())) {
				//}
			}
		}
	}
	auto layer{ registry.GetComponent<LayerComponent>(ent) };
	if (layer) {
		CEO::Get<EventsDispatcher>()->QueueEvent<Events::EntityModified>(Events::EntityModified{ ent, layer->layer,
		Events::EntityModified::MODIFICATION::ADD_ENTITY });
	}
}

rapidjson::Value SceneManager::SerializeEntityRJson(Registry& registry, Registry::Entity ent, unsigned serializedIdx, rapidjson::Document& doc, SERIALIZE_ACTION serializeAction) {
	rapidjson::Value toInsert(rapidjson::kObjectType);

	auto& allocator = doc.GetAllocator();
	// Add the entity's index to the JSON object.
	toInsert.AddMember("entIdx", serializedIdx, allocator);
	PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(ent);
	if (prefabComp) {
		rapidjson::Value outObj(rapidjson::kObjectType);
		rapidjson::Value overrideArr(rapidjson::kArrayType);

		switch (serializeAction) {
		// if save scene, serialize only root entity
		case SERIALIZE_ACTION::SAVE_SCENE:
			if (!prefabComp->root) {
				toInsert.SetNull();
				return toInsert;
			}
			outObj.AddMember("name", prefabComp->name, allocator);
			outObj.AddMember("root", prefabComp->root, allocator);
			for (auto const& [compName, flag] : prefabComp->overriddenComponents)
			{
				rapidjson::Value overrideComp(rapidjson::kObjectType);
				overrideComp.AddMember("component", rapidjson::Value(compName.c_str(), allocator), allocator);
				overrideComp.AddMember("overrideFlag", flag, allocator);
				overrideArr.PushBack(overrideComp, allocator);
			}
			outObj.AddMember("overriddenComponents", overrideArr, allocator);
			toInsert.AddMember("PrefabComponent", outObj, allocator);

			if(HierarchyComponnent* hierarchyComp = registry.GetComponent<HierarchyComponnent>(ent))
			{
				outObj.SetObject();
				outObj.AddMember("parent", hierarchyComp->parent, allocator);
				Registry::Entity firstNonPrefabChild = hierarchyComp->firstChild;
				while (registry.HasComponent<PrefabComponent>(firstNonPrefabChild)) {
					firstNonPrefabChild = registry.GetComponent<HierarchyComponnent>(firstNonPrefabChild)->nextSibling;
				}
				outObj.AddMember("firstChild", firstNonPrefabChild, allocator);
				outObj.AddMember("nextSibling", hierarchyComp->nextSibling, allocator);
				toInsert.AddMember("HierarchyComponnent", outObj, allocator);
			}
			break;
		// if creating prefab, custom serialize children that are also prefabs
		case SERIALIZE_ACTION::CREATE_PREFAB:
			// for backwards compatability
			if (prefabComp->root) {
				EntityRegistry::Entity curr = registry.GetComponent<HierarchyComponnent>(ent)->parent;
				while (curr != 0) {
					if (registry.HasComponent<PrefabComponent>(curr)) {
						prefabComp->root = false;
						break;
					}
					curr = registry.GetComponent<HierarchyComponnent>(curr)->parent;
				}
			}
			// only serialize prefab component if entity is a prefab, or has overrides
			if (!prefabComp->root && (!prefabComp->name.empty() || !prefabComp->overriddenComponents.empty())) {
				outObj.AddMember("name", prefabComp->name, allocator);
				outObj.AddMember("root", prefabComp->root, allocator);
				for(auto const&[compName, flag] : prefabComp->overriddenComponents)
				{
					rapidjson::Value overrideComp(rapidjson::kObjectType);
					overrideComp.AddMember("component", rapidjson::Value(compName.c_str(), allocator), allocator);
					overrideComp.AddMember("overrideFlag", flag, allocator);
					overrideArr.PushBack(overrideComp, allocator);
				}
				outObj.AddMember("overriddenComponents", overrideArr, allocator);
				toInsert.AddMember("PrefabComponent", outObj, allocator);
				
				if(prefabComp->overriddenComponents.empty()) 
					return toInsert;
			}
			break;
		case SERIALIZE_ACTION::COPY:
			outObj.AddMember("name", prefabComp->name, allocator);
			outObj.AddMember("root", prefabComp->root, allocator);
			for (auto const& [compName, flag] : prefabComp->overriddenComponents)
			{
				rapidjson::Value overrideComp(rapidjson::kObjectType);
				overrideComp.AddMember("component", rapidjson::Value(compName.c_str(), allocator), allocator);
				overrideComp.AddMember("overrideFlag", flag, allocator);
				overrideArr.PushBack(overrideComp, allocator);
			}
			outObj.AddMember("overriddenComponents", overrideArr, allocator);
			toInsert.AddMember("PrefabComponent", outObj, allocator);
			break;
		}
		// do not serialize entity if serializing a scene and is a child of prefab
	}
	// Iterate through all components attached to the entity using reflection.
	for (rtr::Instance comp : registry.GetAllComponentsRTR(ent)) {
		//--------------------------------------
		// ANY DEPENDENCY COMPONENT SKIP LOGIC GOES HERE
		//--------------------------------------
		if (!comp.GetType().IsSerializable()) {
			continue;
		}
		
		switch (serializeAction) {
		case SERIALIZE_ACTION::CREATE_PREFAB:
			if (comp.GetType().Name() == "HierarchyComponnent") {
				continue;
			}
			else if (prefabComp && !prefabComp->root && 
					!prefabComp->name.empty() && 
					prefabComp->overriddenComponents.find(comp.GetType().Name()) == prefabComp->overriddenComponents.end()) {
				continue;
			};
			break;
		case SERIALIZE_ACTION::SAVE_SCENE:
			if (prefabComp && prefabComp->overriddenComponents.find(comp.GetType().Name()) == prefabComp->overriddenComponents.end()) {
				continue;
			};
			break;
		case SERIALIZE_ACTION::COPY:
			break;
		}
		// Create a JSON object for the current component.
		rapidjson::Value compObj(rapidjson::kObjectType);
		// Iterate through all members (properties) of the component.
		for (std::string memberName : comp.GetType().Members()) {
			rapidjson::Value key(memberName, allocator);
			if (std::type_index(typeid(std::string)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<std::string>(memberName), allocator);
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(int)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<int>(memberName));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(unsigned int)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<unsigned int>(memberName));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(float)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<float>(memberName));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(double)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<double>(memberName));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(bool)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<bool>(memberName));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(GameObject)) == comp.GetMemberType(memberName)) {
				if (comp.GetVal<GameObject>(memberName).GetEntityID())
				{
					rapidjson::Value val(comp.GetVal<GameObject>(memberName).GetEntityID());
					compObj.AddMember(key, val, allocator);
				}
				else
				{
					rapidjson::Value val(comp.GetVal<GameObject>(memberName).GetPrefabName(), allocator);
					compObj.AddMember(key, val, allocator);
				}
				compObj.AddMember("inHierarchy", rapidjson::Value(false), allocator);
			}
			else if (std::type_index(typeid(Shape)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(static_cast<int>(comp.GetVal<Shape>(memberName)));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(TextRendererComponent::Alignment)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(static_cast<int>(comp.GetVal<TextRendererComponent::Alignment>(memberName)));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(LightComponent::LightType)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(static_cast<int>(comp.GetVal<LightComponent::LightType>(memberName)));
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(Mat3)) == comp.GetMemberType(memberName)) {
				// Create a temporary vector to hold the matrix elements.
				rapidjson::Value val;
				val.SetArray();
				// Resize the vector to hold 9 floats for a 3x3 matrix.
				// Iterate through the matrix elements and add them to the vector.
				for (float compVal : comp.GetVal<Mat3>(memberName).m) {
					val.PushBack(compVal, allocator);
				}
				// Add the vector of floats to the component's JSON object.
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(Vec2)) == comp.GetMemberType(memberName)) {
				// Create a temporary vector to hold the vector components.
				rapidjson::Value val;
				val.SetArray();
				// Store the x and y components in the vector.
				val.GetArray().PushBack(comp.GetVal<Vec2>(memberName).x, allocator);
				val.GetArray().PushBack(comp.GetVal<Vec2>(memberName).y, allocator);
				// Add the vector of floats to the component's JSON object.
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(Color)) == comp.GetMemberType(memberName)) {
				// Create a temporary vector to hold the color components.
				rapidjson::Value val;
				val.SetArray();
				// Store the rgba components in the vector.
				val.GetArray().PushBack(comp.GetVal<Color>(memberName).r, allocator);
				val.GetArray().PushBack(comp.GetVal<Color>(memberName).g, allocator);
				val.GetArray().PushBack(comp.GetVal<Color>(memberName).b, allocator);
				val.GetArray().PushBack(comp.GetVal<Color>(memberName).a, allocator);
				// Add the vector of floats to the component's JSON object.
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(Animation*)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<Animation*>(memberName)->animName, allocator);
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(AudioObj*)) == comp.GetMemberType(memberName)) {
				if (comp.GetVal<AudioObj*>(memberName))
				{
					std::string filepath = comp.GetVal<AudioObj*>(memberName)->GetPath();
					if (!filepath.empty()) {
						rapidjson::Value val(filepath.substr(filepath.find("Sounds\\") + std::string("Sounds\\").size()), allocator);
						compObj.AddMember(key, val, allocator);
					}
				}
			}
			else if (std::type_index(typeid(FontObj*)) == comp.GetMemberType(memberName)) {
				if (comp.GetVal<FontObj*>(memberName))
				{
					std::string filepath = comp.GetVal<FontObj*>(memberName)->path;
					if (!filepath.empty()) {
						rapidjson::Value val(filepath.substr(filepath.find("Fonts\\") + std::string("Fonts\\").size()), allocator);
						compObj.AddMember(key, val, allocator);
					}
				}
			}
			else if (std::type_index(typeid(std::shared_ptr<Tileset>)) == comp.GetMemberType(memberName)) {
				std::shared_ptr<Tileset> tileset = comp.GetVal<std::shared_ptr<Tileset>>(memberName);
				if (tileset)
				{
					rapidjson::Value val(comp.GetVal<std::shared_ptr<Tileset>>(memberName)->path, allocator);
					compObj.AddMember(key, val, allocator);
				}
			}
			else if (std::type_index(typeid(TilemapComponent::ChunkMap)) == comp.GetMemberType(memberName))
			{
				rapidjson::Value chunkMap;
				chunkMap.SetObject();
				for (auto const& chunk:comp.GetVal<TilemapComponent::ChunkMap>(memberName))
				{
					rapidjson::Value tileArray;
					tileArray.SetArray();
					for (Tile tile:chunk.second)
					{
						tileArray.PushBack(tile.spriteID, allocator);
					}
					rapidjson::Value chunkCoordString;
					chunkCoordString.SetString(std::to_string(chunk.first.chunkX) + ',' + std::to_string(chunk.first.chunkY), allocator);
					chunkMap.AddMember(chunkCoordString, tileArray, allocator);
				}
				compObj.AddMember(key, chunkMap, allocator);
			}
			else if (std::type_index(typeid(TextureObj*)) == comp.GetMemberType(memberName)) {
				std::string filepath = CEO::Instance().GetManager<ResourceManager>()->GetTexPath(*comp.GetVal<TextureObj*>(memberName));
				std::string name{};
				if (filepath != "ERROR")
				{
					name = filepath.substr(filepath.find("Assets\\Textures\\") + std::string("Assets\\Textures\\").size());
				}
				rapidjson::Value val(name, allocator);
				compObj.AddMember(key, val, allocator);
			}
			else if (std::type_index(typeid(unsigned long long)) == comp.GetMemberType(memberName)) {
				rapidjson::Value val(comp.GetVal<uint64_t>(memberName));
				compObj.AddMember(key, val, allocator);
			}
		}
		toInsert.AddMember(rapidjson::Value(comp.GetType().Name(), allocator), compObj, allocator);
	}
	return toInsert;
}

void SceneManager::SerializeEntityHierarchy(Registry& registry, Registry::Entity ent, rapidjson::Document& doc, SERIALIZE_ACTION serializeAction) {
	auto& allocator = doc.GetAllocator();
	if(!(doc.HasMember("entities") && doc["entities"].IsArray()))
		doc.AddMember("entities", rapidjson::Value(rapidjson::kArrayType), allocator);
	
	if (!(doc.HasMember("hierarchy") && doc["hierarchy"].IsArray()))
		doc.AddMember("hierarchy", rapidjson::Value(rapidjson::kArrayType), allocator);
	
	rapidjson::Value& outArray = doc["entities"];
	rapidjson::Value& hierarchyArray = doc["hierarchy"];

	// Iterate through all components attached to the entity using reflection.
	if (registry.GetComponent<HierarchyComponnent>(ent)->firstChild) {
		auto getHierarchy = [&registry = registry](Registry::Entity ent) {
			return registry.GetComponent<HierarchyComponnent>(ent);
			};

		//out param
		std::vector<Registry::Entity> entities;

		//functor
		auto serializeEntities = [&registry = registry](Registry::Entity ent, std::vector<Registry::Entity>& entVec) {
			entVec.push_back(ent);
			if (PrefabComponent* comp = registry.GetComponent<PrefabComponent>(ent)) {
				comp->root = false;
			}
			};

		//call traverse
		HierarchyManager::Traverse<std::vector<Registry::Entity>>(registry, ent, std::function{serializeEntities}, entities);

		if (PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(entities[0])) {
			prefabComp->root = true;
		}
		// Serialize all entities collected
		for (unsigned newIdx{}; newIdx < entities.size(); newIdx++) {
			// reindex the prefab component
			rapidjson::Value toInsert = SceneManager::SerializeEntityRJson(registry, entities[newIdx], newIdx + 1, doc, serializeAction);
			if (toInsert.IsNull()) continue;

			// loop through comps
			for (rtr::Instance& inst : registry.GetAllComponentsRTR(entities[newIdx])) {
				if(!inst.GetType().IsSerializable()) continue;

				// get member names that are game objects
				std::vector<std::string> gameObjs = inst.GetType().GetMembersWithType<GameObject>();
				
				/// loop through members that are gameobjs
				for (std::string const& member : gameObjs) {
					// get value of referencing data
					GameObject const& obj{ inst.GetVal<GameObject>(member) };

					// check for if obj is ref an entity
					if (GameObject::Entity objID = obj.GetEntityID()) {
						// if entity exists in hierarchy, reindex if not saving scene
						if (auto const& search = std::find(entities.begin(), entities.end(), objID); search != entities.end()) {
							Registry::Entity newObjRefIdx = static_cast<Registry::Entity>(std::distance(entities.begin(), search));
							switch (serializeAction) {
							case SERIALIZE_ACTION::SAVE_SCENE:
								break;
							case SERIALIZE_ACTION::CREATE_PREFAB:
							case SERIALIZE_ACTION::COPY:
								if (!toInsert.HasMember(inst.GetType().Name()) ||
									!toInsert[inst.GetType().Name()].HasMember(member) ||
									!toInsert[inst.GetType().Name()][member].IsUint())
									continue;
								toInsert[inst.GetType().Name()][member].SetUint(newObjRefIdx+1);
								toInsert[inst.GetType().Name()]["inHierarchy"].SetBool(true);
								break;
							}
						}
						// if entity not in hierarchy, handle based on action
						else {
							switch (serializeAction) {
							case SERIALIZE_ACTION::SAVE_SCENE:
								break;
							case SERIALIZE_ACTION::CREATE_PREFAB:
								toInsert[inst.GetType().Name()][member].SetUint(0);
								break;
							case SERIALIZE_ACTION::COPY:
								break;
							}
						}
					}
				}
			}

			HierarchyComponnent* comp = getHierarchy(entities[newIdx]);

			rapidjson::Value hierarchyComp(rapidjson::kObjectType);
			hierarchyComp.AddMember("parent", getNewEntIndex(entities, comp->parent), allocator);
			hierarchyComp.AddMember("firstChild", getNewEntIndex(entities, comp->firstChild), allocator);
			hierarchyComp.AddMember("nextSibling", getNewEntIndex(entities, comp->nextSibling), allocator);

			hierarchyArray.PushBack(hierarchyComp, allocator);
			outArray.PushBack(toInsert.Move(), allocator);
		}
	}
	else {
		rapidjson::Value toInsert = SceneManager::SerializeEntityRJson(registry, ent, 1, doc, serializeAction);

		if (toInsert.IsNull()) return;

		rapidjson::Value hierarchyComp(rapidjson::kObjectType);
		hierarchyComp.AddMember("parent", 0, allocator);
		hierarchyComp.AddMember("firstChild", 0, allocator);
		hierarchyComp.AddMember("nextSibling", 0, allocator);

		hierarchyArray.PushBack(hierarchyComp, allocator);
		outArray.PushBack(toInsert.Move(), allocator);
	}
}

std::vector<Registry::Entity> SceneManager::DeserializeEntityHierarchy(Registry& registry, Registry::Entity ent, rapidjson::Document& doc, DESERIALIZE_ACTION deserializeAction) {
	std::vector<Registry::Entity> entIdx{};

	if (
		doc.HasMember("entities") == false || doc.HasMember("hierarchy") == false ||
		!(doc["entities"].IsArray() && doc["hierarchy"].IsArray()) ||
		doc["hierarchy"].GetArray().Size() != doc["entities"].GetArray().Size()
		) {
		return std::vector<Registry::Entity>{}; // return empty
	}
	entIdx.push_back(ent);
	SceneManager::DeserializeEntityRJson(registry, ent, *doc["entities"].Begin(), 0, deserializeAction);

	rapidjson::GenericArray<false, rapidjson::Value> hierarchy = doc["hierarchy"].GetArray();

	for (rapidjson::Value* val = doc["entities"].Begin() +1; val < doc["entities"].End(); val++) {
		entIdx.push_back(registry.CreateEntity());
		SceneManager::DeserializeEntityRJson(registry, entIdx.back(), *val, 0, deserializeAction);
	}
	for (rapidjson::SizeType i{}; i < entIdx.size(); i++) {
		PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(entIdx[i]);
		HierarchyComponnent* comp = registry.GetComponent<HierarchyComponnent>(entIdx[i]);
		if (!comp) {
			registry.AddComponent<HierarchyComponnent>(entIdx[i], HierarchyComponnent{});
			comp = registry.GetComponent<HierarchyComponnent>(entIdx[i]);
		}
		if (hierarchy[i].HasMember("parent") && hierarchy[i]["parent"].IsUint()) {
			comp->parent =
				hierarchy[i]["parent"].GetUint() ? entIdx[hierarchy[i]["parent"].GetUint() - 1] : 0;
		}
		if (hierarchy[i].HasMember("firstChild") && hierarchy[i]["firstChild"].IsUint()) {
			comp->firstChild =
				hierarchy[i]["firstChild"].GetUint() ? entIdx[hierarchy[i]["firstChild"].GetUint() - 1] : 0;

		}
		if (hierarchy[i].HasMember("nextSibling") && hierarchy[i]["nextSibling"].IsUint()) {
			comp->nextSibling =
				hierarchy[i]["nextSibling"].GetUint() ? entIdx[hierarchy[i]["nextSibling"].GetUint() - 1] : 0;
		}

		for (rtr::Instance& inst : registry.GetAllComponentsRTR(entIdx[i])) {
			if (!inst.GetType().IsSerializable()) continue;

			// get member names that are game objects
			std::vector<std::string> gameObjs = inst.GetType().GetMembersWithType<GameObject>();

			for (std::string const& member : gameObjs) {
				if (prefabComp) {
					auto search = prefabComp->overriddenComponents.find(inst.GetType().Name());

					if (search == prefabComp->overriddenComponents.end()) {
						continue;
					}
					if (!(search->second & inst.GetType().GetMemberFlag(member))) {
						continue;
					}
				}

				// get value of referencing data
				GameObject const& obj{ inst.GetVal<GameObject>(member) };
				doc["entities"].GetArray()[i].HasMember(inst.GetType().Name());

				// check for if obj is ref an entity
				if (GameObject::Entity objID = obj.GetEntityID()) {
					if (doc["entities"].GetArray()[i][inst.GetType().Name()].HasMember("inHierarchy") &&
						doc["entities"].GetArray()[i][inst.GetType().Name()]["inHierarchy"].IsBool() &&
						doc["entities"].GetArray()[i][inst.GetType().Name()]["inHierarchy"].GetBool())
					{
						inst.SetVal(member, GameObject{ entIdx[objID - 1] });
					}
				}
			}
		}
	}
	return entIdx;
}

rapidjson::Document SceneManager::SerializeSceneRJson(Registry& registry, std::string const& sceneName, std::string const&) {
	rapidjson::Document doc;
	doc.SetObject();
	doc.AddMember(rapidjson::Value("entities"), rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());

	std::set<Registry::Entity> excludeSet, finalSet;
	for (const auto& ve : CEO::Instance().GetManager<SceneManager>()->sceneCache) {
		std::copy(ve.second.begin(), ve.second.end(), std::inserter(excludeSet, excludeSet.end()));
	}
	std::vector<Registry::Entity> ve = registry.GetAllEntity();
	std::copy_if(ve.begin(), ve.end(), std::inserter(finalSet, finalSet.end()), [&excludeSet](Registry::Entity e) { return excludeSet.find(e) == excludeSet.end(); });

	for (Registry::Entity ent : finalSet) {
		rapidjson::Value toInsert = SerializeEntityRJson(registry, ent, ent, doc, SERIALIZE_ACTION::SAVE_SCENE);
		if (!toInsert.IsNull()) {
			doc["entities"].PushBack(toInsert, doc.GetAllocator());
		}
	};

	SaveToFile(doc, sceneName);
	return doc;
}

void SceneManager::SaveToFile(rapidjson::Document& doc, std::string const& sceneName, std::string const& basepath) {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	std::ofstream ofs(basepath + sceneName + ".scene");
	if (!ofs) {
		LOGE("Error in opening %s.scene", sceneName.c_str());
		return;
	}
	ofs << buffer.GetString();
}

void SceneManager::DeserializeSceneRJson(Registry& registry, std::string const& sceneName, std::string const& basepath, bool isPush, unsigned offset) {
	if (!isPush) {
		DeleteScriptInstance(registry, *CEO::Instance().GetManager<ComponentRegistry>());
		registry.DestroyAllEntities();
		registry.ClearAllComponentStorages();
		registry.RestartEntityCount();
	}

	rapidjson::Document doc;
#ifdef PLATFORM_WINDOWS
	std::ifstream ifs(basepath + sceneName + ".scene");
	if (!ifs) {
		LOGE("Error in opening %s%s.scene", basepath.c_str(),sceneName.c_str());
		return;
	}
#else
	std::stringstream ifs = CEO::Instance().GetManager<FileManager>()->ReadFile(basepath + sceneName + ".scene", false, std::ios_base::binary | std::ios_base::in);
	if (!ifs) {
		LOGE("Error in opening %s%s.scene", basepath.c_str(), sceneName.c_str());
		return;
	}
#endif
	std::stringstream buffer;
	buffer << ifs.rdbuf();
#ifdef PLATFORM_WINDOWS
	ifs.close();
#endif
	doc.Parse(buffer.str());
	if (doc.HasParseError()) {
		LOGE("Scene file %s is corrupted or invalid!", (basepath + sceneName + ".scene").c_str());
		return;
	}
	if (!doc.HasMember("entities") || !doc["entities"].IsArray()) {
		LOGE("Scene file %s is corrupted or invalid!", (basepath + sceneName + ".scene").c_str());
		return;
	}

	std::vector<Registry::Entity> ve;
	for (auto curr = doc["entities"].GetArray().Begin(); 
		curr != doc["entities"].GetArray().End(); 
		curr++) {
		if(curr->IsObject()) {
			auto entObj = curr->GetObject();
			Registry::Entity ent{};
			if(entObj["entIdx"].IsUint()){
				 ent = registry.CreateEntity(entObj["entIdx"].GetUint() + offset);
			}
			else {
				LOGE("Scene file %s is corrupted or invalid!", (basepath + sceneName + ".scene").c_str());
				continue;
			}
			DeserializeEntityRJson(registry, ent, *curr, offset, DESERIALIZE_ACTION::LOAD_SCENE);
			if (isPush) ve.push_back(ent);

			int currStack = CEO::Instance().GetManager<UpdateStackManager>()->GetStack();
			auto* usc = registry.GetComponent<UpdateStackComponent>(ent);
			if (usc) { usc->stack = currStack; }
			else { registry.AddComponent<UpdateStackComponent>(ent, UpdateStackComponent(currStack)); }
			HierarchyComponnent* hc = registry.GetComponent<HierarchyComponnent>(ent);
			if (hc->firstChild) hc->firstChild += offset;
			if (hc->nextSibling) hc->nextSibling += offset;
			if (hc->parent) hc->parent += offset;

			if (isPush) {
				if (!usc) usc = registry.GetComponent<UpdateStackComponent>(ent);
				registry.GetComponent<LayerComponent>(ent)->renderPriority += 50 * usc->stack; // We should never have 20 stacked senes,
																								// Nor should a scene have 50 layers. Right?
			}

		}
	}
	if (isPush) {
		CEO::Instance().GetManager<SceneManager>()->sceneCache.emplace(sceneName,std::move(ve));
	}
	else {
		CEO::Get<UpdateStackManager>()->UpdateStorages();
	}
}

void SceneManager::CreateScene(std::string const& sceneName, std::string const& basepath) {
	if (sceneName.empty()) {
		LOGE("Name for scene is empty");
		return;
	}
	
	try {
		// Iterate over directory entries
		for (const auto& entry : std::filesystem::directory_iterator(basepath)) {
			// Check if the entry is a regular file
			if (std::filesystem::is_regular_file(entry.path())) {
				if(entry.path().stem().string() == sceneName && entry.path().extension() == ".scene") {
					LOGE("Scene %s already exists!", sceneName.c_str());
					return;
				}
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		(void)e; // For some reason this counted as an unreferenced local variable.
		LOGE("Error accessing directory: %s", e.what());
	}

	if(sceneName.find_first_of("\\/:*?\"<>|") != std::string::npos) {
		LOGE("Scene name %s contains invalid characters!", sceneName.c_str());
		return;
	}

	rapidjson::Document doc;
	doc.SetObject();
	doc.AddMember("entities", rapidjson::Value(rapidjson::kArrayType), doc.GetAllocator());
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	std::ofstream ofs(basepath + sceneName + ".scene");
	if (!ofs) {
		LOGE("Error in creating %s.scene", sceneName.c_str());
		return;
	}
	ofs << buffer.GetString();
	ofs.close();
}

void SceneManager::CopyEntity(Registry& registry, Registry::Entity source, Registry::Entity target) {
	rapidjson::Document doc;
	doc.SetObject();
	SerializeEntityHierarchy(registry, source, doc, SERIALIZE_ACTION::COPY);
	DeserializeEntityHierarchy(registry, target, doc, DESERIALIZE_ACTION::COPY);
}

void SceneManager::QueueSceneAction(std::string sceneName, SCENE_ACTION action)
{
	SceneManager* sm = CEO::Instance().GetManager<SceneManager>();
	if (sm->nextSceneAction == NONE) {
		sm->nextSceneName = sceneName;
		sm->nextSceneAction = action;
	}
}

void SceneManager::PostRenderUpdate()
{
	// next scene specified
	if (nextSceneAction == NONE) return;
	switch (nextSceneAction)
	{
	case CHANGE:

		CEO::Instance().GetManager<UpdateStackManager>()->SetStack(0);
		ChangeScene(nextSceneName);
		nextSceneName = "";
		nextSceneAction = NONE;
		sceneCache.clear();
		CEO::Get<EventsDispatcher>()->Dispatch<Events::EntityModified>(Events::EntityModified{ 0, 0, Events::EntityModified::MODIFICATION::REMOVE_ALL_ENTITY });
		CEO::Get<EventsDispatcher>()->Dispatch<Events::SceneChanged>(Events::SceneChanged{});
		break;
	case PUSH:
		if (nextSceneName == sceneName.top()) {
			LOGE("TRYING TO PUSH SAME SCENE TWICE!!");
			nextSceneName = "";
			nextSceneAction = NONE;
			break;
		}

		CEO::Instance().GetManager<UpdateStackManager>()->IncrementStack();
		if (sceneCache.find(nextSceneName) != sceneCache.end()) {
			std::vector<Registry::Entity>& ve = sceneCache[nextSceneName];
			Registry& r = *CEO::Instance().GetManager<Registry>();
			for (Registry::Entity e : ve) {

				UpdateStackComponent* usc = r.GetComponent<UpdateStackComponent>(e);
				ActiveComponent* ac = r.GetComponent<ActiveComponent>(e);
				LayerComponent* lc = r.GetComponent<LayerComponent>(e);

				if (usc->stack != -1) ac->isActiveSelf = true;
				usc->stack = CEO::Instance().GetManager<UpdateStackManager>()->GetStack();
				lc->renderPriority += 50 * usc->stack;
			}
		}
		else {

			Registry& reg = *CEO::Instance().GetManager<Registry>();
			std::vector<EntityRegistry::Entity> ve = reg.GetAllEntity();
			EntityRegistry::Entity offset = *std::max_element(ve.begin(), ve.end());
#ifdef PLATFORM_WINDOWS
            DeserializeSceneRJson(reg, nextSceneName, "Assets\\Scenes\\", true, offset);
#else
            DeserializeSceneRJson(reg, nextSceneName, "Scenes/", true, offset);

#endif
		}
		sceneName.push(nextSceneName);
		nextSceneName = "";
		nextSceneAction = NONE;
		CEO::Get<EventsDispatcher>()->Dispatch<Events::ScenePushed>(Events::ScenePushed{});

		//CEO::Get<EventsDispatcher>()->Dispatch<Events::SceneChanged>(Events::ScenePushed{}); 
		break;
	case POP:
		if (sceneName.size() <= 1) {
			LOGE("TRYING TO POP LAST SCENE! USE CHANGE INSTEAD IF YOU ARE REPLACING WITH ANOTHER SCENE");
			nextSceneName = "";
			nextSceneAction = NONE;
			break;
		}

		CEO::Instance().GetManager<UpdateStackManager>()->DecrementStack();
		std::vector<Registry::Entity>& ve = sceneCache[sceneName.top()];
		Registry& r = *CEO::Instance().GetManager<Registry>();
		for (Registry::Entity e : ve) {

			ActiveComponent* ac = r.GetComponent<ActiveComponent>(e);
			UpdateStackComponent* usc = r.GetComponent<UpdateStackComponent>(e);
			LayerComponent* lc = r.GetComponent<LayerComponent>(e);

			lc->renderPriority -= 50 * usc->stack;
			if(ac->isActiveSelf) ac->isActiveSelf = false;
			else usc->stack = -1;
		}
		sceneName.pop();
		nextSceneName = "";
		nextSceneAction = NONE;
		//CEO::Get<EventsDispatcher>()->Dispatch<Events::SceneChanged>(Events::ScenePopped{});
		CEO::Get<EventsDispatcher>()->Dispatch<Events::ScenePop>(Events::ScenePop{});

		break;
	}
}

void SceneManager::ChangeScene(std::string m_sceneName)
{
	Registry& reg = *CEO::Instance().GetManager<Registry>();
	ComponentRegistry& compReg = *CEO::Instance().GetManager<ComponentRegistry>();
	DeleteScriptInstance(reg, compReg);
	reg.DestroyAllEntities();
	reg.ClearAllComponentStorages();
	reg.RestartEntityCount();
#ifdef PLATFORM_WINDOWS
	DeserializeSceneRJson(reg, m_sceneName);
#else
    DeserializeSceneRJson(reg, m_sceneName, "Scenes/");

#endif
	auto& stack = CEO::Instance().GetManager<SceneManager>()->sceneName;
	while (!stack.empty()) stack.pop();
	stack.push(m_sceneName);
	CEO::Instance().GetManager<SceneManager>()->baseScene = m_sceneName;
	CEO::Instance().GetManager<SceneManager>()->isSceneChanged = true;
}

void SceneManager::Init() {
	// empty for now
}

void SceneManager::Free() {
	// empty for now
}


