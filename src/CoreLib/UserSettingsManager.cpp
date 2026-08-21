/*!
@file       UserSettingsManager.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       25/03/2026
@brief		Handles saving and retrieval of persistant user data


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "UserSettingsManager.h"
#include "FileManager.h"

void UserSettingsManager::Init(std::string const& _configPath) 
{
	static bool isInit = false;
	// early return if already initialized
	if (isInit)
		return;
	configPath = _configPath;
	std::stringstream ifs = CEO::Get<FileManager>()->ReadFile(configPath, true);
	if (ifs.str().size() > 0)
		settings.Parse(ifs.str());
	else
		settings.Parse("{}");
}

void UserSettingsManager::SetBool(std::string const& name, bool value)
{
	if (settings.HasMember(name))
		settings[name].SetBool(value);
	else
		settings.AddMember(rapidjson::Value(name.c_str(), static_cast<rapidjson::SizeType>(name.size()), settings.GetAllocator()).Move(), value, settings.GetAllocator());
}
void UserSettingsManager::SetInt(std::string const& name, bool value)
{
	if (settings.HasMember(name))
		settings[name].SetInt(value);
	else
		settings.AddMember(rapidjson::GenericStringRef{ name.c_str() }, value, settings.GetAllocator());
}
void UserSettingsManager::SetFloat(std::string const& name, bool value)
{
	if (settings.HasMember(name))
		settings[name].SetFloat(value);
	else
		settings.AddMember(rapidjson::GenericStringRef{ name.c_str() }, value, settings.GetAllocator());
}

void UserSettingsManager::SaveSettings()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	settings.Accept(writer);

	std::stringstream stringStream(configPath);
	if (!stringStream) {
		LOGE("Error in opening %s", configPath.c_str());
		return;
	}
	stringStream << buffer.GetString();
	CEO::Get<FileManager>()->WriteFile(configPath, stringStream);
}

void UserSettingsManager::Free() {
	// empty
}