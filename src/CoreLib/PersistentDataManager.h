/**___________________________________________________________________________/
@file       PersistentDataManager.h
@author     t.junjie@digipen.edu
@date       04/3/2026	(DD/MM/YYYY)
@brief		Manager to handle / store persistent data.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include <unordered_map>
#include <any>
#include <string>

class PersistentDataManager {
public:
	/*!
	* \brief Add/Update data to data store.
	* \param[in] key	- The key (name) of the data to add / update.
	* \param[in] value	- The value of the data to add / update.
	*/
	template <typename T>
	void Set(const std::string& key, T&& value) { data[key] = std::forward<T>(value); }

	/*!
	* \brief Get the specified data from data store.
	* \param[in] key - The key (name) of the data to get.
	* \return A pointer to the specified data, nullptr if data doesn't exist.
	*/
	template <typename T>
	T* Get(const std::string& key) {
		auto it{ data.find(key) };
		if (it == data.end()) return nullptr;
		return std::any_cast<T>(&it->second);
	}

	/*!
	* \brief Get the specified data from data store, create if data doesn't exit.
	* \param[in] key - The key (name) of the data to get.
	* \return A reference to the specified data.
	*/
	template <typename T>
	T GetOrCreate(const std::string& key) {
		auto it{ data.find(key) };
		if (it == data.end()) data[key] = T{};
		return *std::any_cast<T>(&data[key]);
	}

	/*!
	* \brief Check whether data exists in store.
	* \param[in] key - The key (name) of the data to check.
	* \return True if data exists, false otherwise.
	*/
	bool Has(const std::string& key) const { return data.find(key) != data.end(); }

	/*!
	* \brief Remove specified data from persistent data store.
	* \param[in] key - The key (name) of the data to remove.
	*/
	void Remove(const std::string& key) { data.erase(key); }

	/*!
	* \brief Clear persistent data store.
	*/
	void ClearAll() { data.clear(); }
private:
	std::unordered_map<std::string, std::any> data;		// unordered map of persistent data, key being name of the data
};