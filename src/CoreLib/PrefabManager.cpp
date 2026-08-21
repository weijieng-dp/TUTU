/*!
@file       PrefabManager.cpp
@author     Kaeden Tan (kaedenjiawei.tan) (100%)
@date       26/12/2025

Implementation of the PrefabManager class, responsible for
creating, loading, and managing prefab-related operations
such as prefab instantiation and member overrides.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/

#define RAPIDJSON_HAS_STDSTRING 1
#include "pch.h"
#include "PrefabManager.h"
#include "SceneManager.h"
#include "components.h"

std::optional<rapidjson::Document> PrefabManager::LoadPrefab(
	std::string const& prefabName,
	std::string const& basepath)
{
	rapidjson::Document doc; // JSON document to store parsed prefab data

	// Read prefab file into a stringstream using FileManager
	std::stringstream ifs =
		CEO::Instance().GetManager<FileManager>()
		->ReadFile(basepath + prefabName + ".prefab",
			false,
			std::ios_base::binary | std::ios_base::in);

	// Fail if file cannot be opened
	if (!ifs) {
		LOGE("Error in opening %s%s.scene", basepath.c_str(), prefabName.c_str());
		return std::nullopt;
	}

	std::stringstream buffer;

	// Copy file contents into buffer
	buffer << ifs.rdbuf();

	// Parse JSON from buffer string
	doc.Parse(buffer.str());

	// Validate JSON parsing
	if (doc.HasParseError()) {
		LOGE("Prefab file %s is corrupted or invalid!",
			(basepath + prefabName + ".scene").c_str());
		return std::nullopt;
	}

	// Return parsed document
	return std::move(doc);
}

bool PrefabManager::CreatePrefab(
	std::string const& prefabNameRaw,
	Registry::Entity entID,
	std::string const& basepath)
{
	std::string prefabName = prefabNameRaw;

	// Reject empty prefab names
	if (prefabName.empty()) {
		LOGE("Name for prefab is empty");
		return false;
	}

	// Reject invalid filename characters
	if (prefabName.find_first_of("\\/:*?\"<>|") != std::string::npos) {
		LOGE("Prefab name %s contains invalid characters!", prefabName.c_str());
		return false;
	}

	std::vector<std::filesystem::directory_entry> prefabEntries;
	size_t count{};

	try {
		// Iterate through prefab directory
		for (const std::filesystem::directory_entry& entry :
			std::filesystem::directory_iterator(basepath))
		{
			// Only consider regular .prefab files
			if (std::filesystem::is_regular_file(entry.path()) &&
				entry.path().extension() == ".prefab")
			{
				prefabEntries.push_back(entry);

				// Detect name collision with existing prefab
				if (entry.path().stem().string() == prefabName) {
					count = 1;
				}
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		// Directory access failure
		LOGE("Error accessing directory: %s", e.what());
		return false;
	}

	// If prefab name already exists, find the next available numbered name
	if (count == 1) {
		for (;
			std::find_if(
				prefabEntries.begin(),
				prefabEntries.end(),
				[prefabName, count](std::filesystem::directory_entry const& entry)
				{
					return entry.path().stem().string() ==
						prefabName + "(" + std::to_string(count) + ")";
				})
			!= prefabEntries.end();
			count++);

		prefabName += "(" + std::to_string(count) + ")";
	}

	Registry& reg = *CEO::Instance().GetManager<Registry>();

	// Rename root entity to match prefab name
	reg.GetComponent<NameComponent>(entID)->name = prefabName;

	rapidjson::Document doc;
	doc.SetObject(); // Root JSON object

	if (entID != 0) {
		// Serialize entity hierarchy into prefab format
		SceneManager::SerializeEntityHierarchy(
			reg,
			entID,
			doc,
			SceneManager::SERIALIZE_ACTION::CREATE_PREFAB);

		// Attach PrefabComponent metadata to entity hierarchy
		AddPrefabComp(entID, prefabName);
	}
	else {
		// Create empty prefab structure
		doc.AddMember("entities",
			rapidjson::Value(rapidjson::kArrayType),
			doc.GetAllocator());

		doc.AddMember("hierarchy",
			rapidjson::Value(rapidjson::kArrayType),
			doc.GetAllocator());
	}

	// Serialize JSON document into string buffer
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	// Write prefab file to disk
	std::ofstream ofs(basepath + prefabName + ".prefab");
	if (!ofs) {
		LOGE("Error in creating %s.prefab", prefabName.c_str());
		return false;
	}

	ofs << buffer.GetString();
	ofs.close();

	return true;
}

void PrefabManager::AddPrefabComp(
	Registry::Entity rootEnt,
	std::string const& prefabName)
{
	Registry& reg = *CEO::Instance().GetManager<Registry>();

	// Lambda to attach or update PrefabComponent for each entity
	auto addPrefabComp =
		[&reg, rootEnt](Registry::Entity currEnt,
			std::string const& prefabName)
		{
			// Fetch existing PrefabComponent if present
			PrefabComponent* prefabComp =
				reg.GetComponent<PrefabComponent>(currEnt);

			// Add PrefabComponent if missing
			if (!prefabComp) {
				reg.AddComponent<PrefabComponent>(currEnt, PrefabComponent{});
				prefabComp = reg.GetComponent<PrefabComponent>(currEnt);
			}

			// Root entity stores prefab name and is marked as root
			if (rootEnt == currEnt) {
				prefabComp->name = prefabName;
				prefabComp->root = true;
			}
			else {
				// Child entities are not prefab roots
				prefabComp->root = false;
			}
		};

	// Traverse entire hierarchy and apply PrefabComponent
	HierarchyManager::Traverse<std::string const>(
		reg,
		rootEnt,
		std::function{ addPrefabComp },
		prefabName);
}

void PrefabManager::OverrideMember(
	PrefabComponent& prefabComp,
	std::string const& component,
	std::string const& memberName)
{
	// Check if component already has overridden members
	if (auto search =
		prefabComp.overriddenComponents.find(component);
		search != prefabComp.overriddenComponents.end())
	{
		// Add member override flag using bitwise OR
		search->second |=
			rtr::TypeInfo::GetByName(component)
			.GetMemberFlag(memberName);
	}
	else {
		// Insert new component override entry
		prefabComp.overriddenComponents.emplace(
			component,
			rtr::TypeInfo::GetByName(component)
			.GetMemberFlag(memberName));
	}
}