/*!
@file       UserSettingsManager.h
@author     Ou Yukang (yukang.ou) 100%
@date       25/03/2026
@brief		Handles saving and retrieval of persistant user data

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "MathLib.h"

class UserSettingsManager {


	UserSettingsManager(const UserSettingsManager&) = delete;
	UserSettingsManager& operator=(const UserSettingsManager&) = delete;

	std::string configPath;
public:

	UserSettingsManager() = default;
	~UserSettingsManager() = default;

	/*!
	* \brief
	*	initialize the singleton instance
	* \param
	*	_registry - registry of ecs
	* \param
	*	_compRegistry - component registry of ecs
	*/
	void Init(std::string const& configPath);

	/*!
	* \brief
	*	set value of settings and save to file
	* \param
	*	name - name of setting
	* \param
	*	value - new value to set
	*/
	void SetBool(std::string const& name, bool value);
	void SetInt(std::string const& name, bool value);
	void SetFloat(std::string const& name, bool value);

	void SaveSettings();
	/*!
	* \brief
	*	free any resources used by UI Manager
	*/
	void Free();

	rapidjson::Document settings;
};