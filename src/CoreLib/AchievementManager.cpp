/**___________________________________________________________________________/
@file       AchievementManager.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       03/03/2026	(DD/MM/YYYY)
@brief		Manager that handles manages and spawns a grid-based map based on
			user input. Handles spawning of enemies and applying gimmick for
			each tile.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "AchievementManager.h"

namespace {
#ifdef PLATFORM_WINDOWS
	const std::string basePath("Assets\\");
	void ReplaceSlashes(std::string& s) {
		std::replace(s.begin(), s.end(), '/', '\\');
	}
#endif
#ifdef PLATFORM_ANDROID
	const std::string basePath("");
#endif
}

void AchievementManager::Init(const std::string& achievementData) {
	if (initialized) return;							// skip if already initialized
	achievementFilePath = basePath + achievementData;
	if (allAchievements.empty() && !LoadAchievementData())
	{
		LOGE("Failed to load Achievement Data");
	}
	else
	{
		LOGI("Loaded Achievement Data");
		LoadUnlockData();
	}
	initialized = true;									// set initialized to true
}



bool AchievementManager::LoadAchievementData() {
	if (!allAchievements.empty()) return false;			
	rapidjson::Document doc;
	std::stringstream ifs{ CEO::Instance().GetManager<FileManager>()->ReadFile(achievementFilePath) };
	if (!ifs) {
		LOGE("Error in reading %s achievementData", achievementFilePath.c_str());
		return false;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	
	doc.Parse(buffer.str());
	if (doc.HasParseError()) {
		LOGE("AchievementData json is corrupted or invalid!");
		return false;
	}
	else if (!doc.HasMember("Achievement") || !doc["Achievement"].IsArray()) {
		LOGE("AchievementData json does not have Achievement array!");
		return false;
	}

	const rapidjson::Value& ach{ doc["Achievement"] };
	for (rapidjson::SizeType i{}; i < ach.Size(); ++i) {
		const rapidjson::Value& t{ ach[i] };
		Achievement achievement;

		achievement.achievementName = t["achievementName"].GetString();
		
		std::string unlockType{ t["unlockType"].GetString() };
		if (unlockType == "ENEMY") achievement.unlocktype = UnlockType::ENEMY;
		else if (unlockType == "TILES") achievement.unlocktype = UnlockType::TILE;
		else if (unlockType == "ITEM") achievement.unlocktype = UnlockType::ITEM;
		else if (unlockType == "LOSE") achievement.unlocktype = UnlockType::LOSE;
		else if (unlockType == "WIN") achievement.unlocktype = UnlockType::WIN;
		else achievement.unlocktype = UnlockType::ENDGAME;

		achievement.unlockCount = t["unlockCount"].GetInt();

		std::string rewardType{ t["rewardType"].GetString()};
		if (rewardType == "ENEMY") achievement.rewardType = RewardType::ENEMY;
		else if (rewardType == "TILES") achievement.rewardType = RewardType::TILE;
		else if (rewardType == "GIMMICK") achievement.rewardType = RewardType::GIMMICK;
		else achievement.rewardType = RewardType::ENDGAME;

		const rapidjson::Value& rewardItems{ t["rewardItem"] };
		for (rapidjson::SizeType j{}; j < rewardItems.Size(); ++j)
		{
			achievement.rewardItem.emplace_back(rewardItems[j].GetString());
		}

		const rapidjson::Value& requirements{ t["requirements"] };
		for (rapidjson::SizeType j{}; j < requirements.Size(); ++j)
		{
			achievement.requirements.emplace_back(requirements[j].GetString());
		}

		achievement.flavourText = t["flavourText"].GetString();
		achievement.unlockText = t["unlockText"].GetString();

		allAchievements.push_back(std::move(achievement));
	}
	return true;
}

const std::vector<Achievement>& AchievementManager::GetAllAchievements()
{
	return allAchievements;
}

const std::vector<Achievement>& AchievementManager::GetUnlockedAchievements()
{
	return unlockedAchievements;
}

std::vector<Achievement> AchievementManager::GetUnlockedAchievements(RewardType rewardType) {
	std::vector<Achievement> achieve;
	for (const auto& unlocked : unlockedAchievements) {
		if (unlocked.rewardType == rewardType) achieve.push_back(unlocked);
	}
	return achieve;
}

const std::vector<Achievement>& AchievementManager::GetLockedAchievements()
{
	return lockedAchievements;
}

std::vector<Achievement> AchievementManager::GetLockedAchievements(RewardType rewardType) {
	std::vector<Achievement> achieve;
	for (const auto& locked : lockedAchievements) {
		if (locked.rewardType == rewardType) achieve.push_back(locked);
	}
	return achieve;
}

void AchievementManager::UpdateTracker(std::string unlockType)
{
	if (unlockType.empty()) return;
	LOGI("%s", unlockType.c_str());
	UnlockType uType{};
	if (unlockType == "ENEMY") uType = UnlockType::ENEMY;
	else if (unlockType == "TILES") uType = UnlockType::TILE;
	else if (unlockType == "ITEM") uType = UnlockType::ITEM;
	else if (unlockType == "LOSE") uType = UnlockType::LOSE;
	else if (unlockType == "WIN") uType = UnlockType::WIN;
	else uType = UnlockType::ENDGAME;

	unlockTracker[uType]++;

	CheckAchievements(uType);
}

void AchievementManager::CheckAchievements(UnlockType unlockType)
 {
	if (lockedAchievements.size() == 0 && !shown && unlockType == UnlockType::WIN) // if all achievements unlocked and end cutscene not shown
	{
		showEndCutscene = true;
		return;
	}
	
	if (lockedAchievements.size() == 0) return;
	bool unlockAchievement = false;
	Achievement newAchievement{};
 	for (int i = 0; i < lockedAchievements.size(); i++)
	{
		if (lockedAchievements[i].unlocktype != unlockType) continue;
		if (lockedAchievements[i].unlockCount > unlockTracker[unlockType]) continue;
		std::queue<Achievement> tempQueue = popUpQueue;
		bool alreadyQueued = false;
		while (!tempQueue.empty())
		{
			if (tempQueue.front().achievementName == lockedAchievements[i].achievementName)
			{
				alreadyQueued = true;
				break;
			}
			tempQueue.pop();
		}
		if (alreadyQueued) continue;

		if (lockedAchievements[i].requirements.size() == 1 && lockedAchievements[i].requirements[0].empty())
		{
			newAchievement = lockedAchievements[i];
			//unlockedAchievements.push_back(lockedAchievements[i]);
			//lockedAchievements.erase(lockedAchievements.begin() + i);
			unlockAchievement = true;
			break;
		}
		bool requirementsMet = true;
		for (int j = 0; j < lockedAchievements[i].requirements.size(); j++)
		{
			auto it = std::find_if(unlockedAchievements.begin(), unlockedAchievements.end(), [&](const Achievement& ach) {
				return ach.achievementName == lockedAchievements[i].requirements[j];
				});
			if (it == unlockedAchievements.end())
			{
				std::queue<Achievement> tempReqQueue = popUpQueue;
				bool foundInQueue = false;
				while (!tempReqQueue.empty())
				{
					if (tempReqQueue.front().achievementName == lockedAchievements[i].requirements[j])
					{
						foundInQueue = true;
						break;
					}
					tempReqQueue.pop();
				}
				if (!foundInQueue) requirementsMet = false;
			}
		}

		if (requirementsMet)
		{
			newAchievement = lockedAchievements[i];
			//unlockedAchievements.push_back(lockedAchievements[i]);
			//lockedAchievements.erase(lockedAchievements.begin() + i);
			unlockAchievement = true;
		}
	}

	if (unlockAchievement)
	{
		popUpQueue.push(newAchievement);
		if ((popUpQueue.size() + unlockedAchievements.size()) == 8 && unlockType == UnlockType::WIN) UpdateTracker("ENDGAME");
		LOGI("%s", newAchievement.achievementName.c_str());
		//SaveUnlockData();
	}
}

void AchievementManager::LoadUnlockData()
{
	std::stringstream achievementNames = CEO::Instance().GetManager<FileManager>()->ReadFile(achievementSavePath, true);
	std::vector <std::string> achNames;
	if (achievementNames.rdbuf()->in_avail() == 0)
	{
		std::stringstream newFile;
		newFile.clear();
		newFile.str("");
		CEO::Instance().GetManager<FileManager>()->WriteFile(achievementSavePath, newFile);
		LOGI("No Save File, Written a new blank one");
		for (int i = 0; i < allAchievements.size(); i++)
		{
			lockedAchievements.push_back(allAchievements[i]);
		}
	}
	else
	{
		std::string token;
		while (std::getline(achievementNames, token, ','))
		{
			achNames.push_back(token);
		}

		for (int i = 0; i < allAchievements.size(); i++)
		{
			bool found = std::find(achNames.begin(), achNames.end(),
				allAchievements[i].achievementName) != achNames.end();

			if (found)														// Split achievements to locked and unlocked  
				unlockedAchievements.push_back(allAchievements[i]);
			else
				lockedAchievements.push_back(allAchievements[i]);
		}

		LOGI("Loaded unlocked Achievements from save file");
		if (lockedAchievements.size() == 0)
		{
			shown = true;
		}
	}
}

Achievement AchievementManager::GetAchievement(std::string achievementName)
{
	auto it = std::find_if(allAchievements.begin(), allAchievements.end(), [&](const Achievement& ach)
		{
			return ach.achievementName == achievementName;
		});

	return *it;
}

void AchievementManager::UnlockAchievement(Achievement newAchievement)
{
	auto it = std::find_if(lockedAchievements.begin(), lockedAchievements.end(),
		[&](const Achievement& ach) {
			return ach.achievementName == newAchievement.achievementName;
		});

	if (it != lockedAchievements.end())
	{
		unlockedAchievements.push_back(*it);
		lockedAchievements.erase(it);
	}

	SaveUnlockData();
}

void AchievementManager::ResetData()
{
	unlockedAchievements.clear();
	for (auto& [key, val] : unlockTracker)
		val = 0;
	shown = false;
	showEndCutscene = false;
	firstPlay = true;
	SaveUnlockData();
	LoadUnlockData();
	LOGI("Reset Save File");
}

void AchievementManager::SaveUnlockData()
{
 	std::stringstream achievementNames;
	for (int i = 0; i < unlockedAchievements.size(); i++)
	{
		if (i > 0) achievementNames << ",";
		achievementNames << unlockedAchievements[i].achievementName;
	}
	CEO::Instance().GetManager<FileManager>()->WriteFile(achievementSavePath, achievementNames);
}

void AchievementManager::SaveTracker()
{
	rollbackTracker[UnlockType::ENEMY] = unlockTracker[UnlockType::ENEMY];
	rollbackTracker[UnlockType::TILE] = unlockTracker[UnlockType::TILE];
	rollbackTracker[UnlockType::ITEM] = unlockTracker[UnlockType::ITEM];
}

void AchievementManager::RollbackTracker()
{
	unlockTracker[UnlockType::ENEMY] = rollbackTracker[UnlockType::ENEMY];
	unlockTracker[UnlockType::TILE] = rollbackTracker[UnlockType::TILE];
	unlockTracker[UnlockType::ITEM] = rollbackTracker[UnlockType::ITEM];
	std::queue<Achievement> empty;
	std::swap(popUpQueue, empty);
}
