/**___________________________________________________________________________/
@file       AchivementManager.h
@author		d.lorenzoyongoyong@digipen.edu
@date       03/03/2026	(DD/MM/YYYY)
@brief		Manager that handles achievements, tracks ingame events and saves
			achievement as game data.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include <string>
#include <sstream>
#include <unordered_map>
#include "GameObjects.h"
#include "FileManager.h"

// Enum for the different types unlock conditions
enum class UnlockType {
	ENEMY = 0,				// Enemy killed unlocks
	TILE,					// Tile placed unlocks
	ITEM,					// Item picked up unlocks
	LOSE,					// Lose counter unlocks
	WIN,					// Win counter unlocks
	ENDGAME					// Last Achievement
};
// Enum for different types of rewards
enum class RewardType {
	ENEMY = 0,				// Enemy Combination unlocks
	TILE,					// Tile choice unlocks
	GIMMICK,				// Gimmick choice unlocks
	ENDGAME					// Last Achievement Unlocks
};
// Struct for Achievement archetype read from achievementData
struct Achievement
{
	std::string achievementName{};						// Name of Achievement condition
	UnlockType unlocktype{ UnlockType::ENDGAME };		// Unlock condition
	int unlockCount{ 0 };								// Amount of times an unlock condition has to be triggered to unlock achievement
	RewardType rewardType{ RewardType::ENDGAME };		// Type of reward to award to the player
	std::vector<std::string> rewardItem{};				// List of reward to award to the player
	std::vector<std::string> requirements{};			// Prerequisite achievements before unlocking
	std::string flavourText{};							// Flavour text for tooltip
	std::string unlockText{};							// Unlock requirement text for tooltip
};



class AchievementManager {
public:

	/*!
	* \brief Initialize MapManager.
	* \param[in] achievementData - The JSON file to read the achievement conditions from.
	*/
	void Init(const std::string& achievementData);
	/*!
	* \brief Loads in the tile data from JSON into memory.
	*/
	bool LoadAchievementData();

	
public:
	
	/*!
	* \brief Get all achievements
	* \return - A vector of all Achievements
	*/
	const std::vector<Achievement>& GetAllAchievements();
	
	/*!
	* \brief Get unlocked achievements
	* \return - A vector of unlocked Achievements
	*/
	const std::vector<Achievement>& GetUnlockedAchievements();
	
	/*!
	* \brief Get unlocked achievements based on reward type
	* \param[in] rewardType - The reward type to check for
	* \return - A vector of unlocked Achievements based on a reward type
	*/
	std::vector<Achievement> GetUnlockedAchievements(RewardType rewardType);
	
	/*!
	* \brief Get locked achievements
	* \return - A vector of locked Achievements
	*/
	const std::vector<Achievement>& GetLockedAchievements();
	
	/*!
	* \brief Get locked achievements based on reward type
	* \param[in] rewardType - The reward type to check for
	* \return - A vector of locked Achievements based on a reward type
	*/
	std::vector<Achievement> GetLockedAchievements(RewardType rewardType);
	
	/*!
	* \brief Save unlock tracker to rollback tracker
	*/
	void SaveTracker();
	
	/*!
	* \brief Set rollback tracker values to unlock tracker in case of aborted run
	*/
	void RollbackTracker();
	
	/*!
	* \brief To add instance of unlock events to unlock tracker
	* \param[in] unlockType - Type of unlock condition event
	*/
	void UpdateTracker(std::string unlockType);

	/*!
	* \brief Get Achievement archetype from all achievement data
	* \param[in] achievementName - The achievement name to check for
	* \return - Achievement archetype based on achievement name
	*/
	Achievement GetAchievement(std::string achievementName);

	/*!
	* \brief Unlock an Achievement
	* \param[in] newAchievement - Achievement archetype to unlock
	*/
	void UnlockAchievement(Achievement newAchievement);

	/*!
	* \brief Reset all achievement data including save
	*/
	void ResetData();

	std::queue<Achievement> popUpQueue;			// Achievement popup queue

	bool showEndCutscene{ false };				// Whether to show ending cutscene
	bool shown{ false };						// Check to see if ending cutscene has been shown
	bool firstPlay{ true };						// Check to see if it is player's first run
	
private:

	/*!
	* \brief Check through locked achievements if unlock tracker meets requirements
	* \param[in] unlockType - Unlock condition to check for from the unlock tracker
	*/
	void CheckAchievements(UnlockType unlockType);
	
	/*!
	* \brief Save achievement player data and write to file
	*/
	void SaveUnlockData();
	
	/*!
	* \brief Load player achievement data from the save path and populate unlocked achievement vector
	*/
	void LoadUnlockData();

private:
	bool initialized{ false };								// whether the manager has been initialized
	std::vector<Achievement> allAchievements;				// all achievement archetypes
	std::vector<Achievement> unlockedAchievements;			// unlocked achievement archetypes
	std::vector<Achievement> lockedAchievements;			// locked achievement archetypes
	std::unordered_map<UnlockType, int> unlockTracker		
	{
		{UnlockType::ENEMY, 0},
		{UnlockType::TILE, 0},
		{UnlockType::LOSE, 0},
		{UnlockType::WIN, 0},
		{UnlockType::ENDGAME, 0}

	};

	std::unordered_map<UnlockType, int> rollbackTracker
	{
		{UnlockType::ENEMY, 0},
		{UnlockType::TILE, 0},
		{UnlockType::LOSE, 0},
		{UnlockType::WIN, 0},
		{UnlockType::ENDGAME, 0}
	};

	std::string achievementFilePath{};											// achievement archetypes json path
	std::string achievementSavePath{ "Assets/GameData/Achievements.save" };		// player achievements save data path
};