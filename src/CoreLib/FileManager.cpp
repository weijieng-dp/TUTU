/**___________________________________________________________________________/
@file          FileManager.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Filemanager implementation. (writing of files to android not implemented yet)

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "FileManager.h"
#ifdef PLATFORM_WINDOWS
#include "shlobj_core.h"
#endif


void FileManager::Initialize(void* javaAssetManager) {
	(void)javaAssetManager;
#ifdef PLATFORM_ANDROID
	if (!javaAssetManager) {
		LOGE("erm not good");
		failLoad = true;
	}
    assetManager = static_cast<AAssetManager*>(javaAssetManager);
#else
	wchar_t* wbuf{ nullptr };
	char buf[1024];
	size_t bufLen{ 0 };
	mbstate_t state{};	// Bro wtf is this

	// Getting filepath to Documents
	SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &wbuf);

	// Determining the length of the buffer
	while (*(wbuf + bufLen)) bufLen++;

	// Freeing windows allocated memory
	wcsrtombs_s(&bufLen, buf, 1024, (const wchar_t**) &wbuf, 1024, &state);

	// Updating PCWriteFilepath
	PCWriteFilepath = buf;
	PCWriteFilepath += "\\GetMeOutOfHell\\";

#endif
}

FileManager::~FileManager() {
#ifdef PLATFORM_ANDROID
	// free assetmanager if need be
#endif // PLATFORM_ANDROID
}

std::stringstream FileManager::ReadFile(const std::string& s, bool fromWritePath, std::ios_base::openmode mode) {
#ifdef PLATFORM_WINDOWS
	std::ifstream ifs;
	if (!fromWritePath) {
		ifs.open(s, mode);
	}
	else {
		// Open the file from the write folder
		ifs.open(PCWriteFilepath + s, mode);
		
		// If it failed to open, aka doesnt exist, find for default within current directory:
		if (!ifs.is_open()) {
			ifs.open(s, mode);

			if (!ifs.is_open()) {
				LOGE("Cannot find file in write or default directory: %s", s.c_str());
				return {};
			}
		}
	}
	if (!ifs) return {};

	std::stringstream sstr;
	sstr << ifs.rdbuf();
	return sstr;
#endif
#ifdef PLATFORM_ANDROID
	if (failLoad) {
		return {};
	}

	AAsset* asset;
	if (!fromWritePath) {
		asset = AAssetManager_open(assetManager, s.c_str(), AASSET_MODE_BUFFER);

		if (asset == nullptr) {
			LOGE("%s", ("Couldn't find file: " + s).c_str());
			return {};
		}
	}
	else {
		std::ifstream ifs;
		ifs.open(AndroidWriteFilepath + s, mode);
		
		if (ifs) {
			std::stringstream sstrin;
			sstrin << ifs.rdbuf();
            std::stringstream sstr(sstrin.str(),mode);
			return sstr;
		}
		else {

			asset = AAssetManager_open(assetManager, s.c_str(), AASSET_MODE_BUFFER);

			if (asset == nullptr) {
				LOGE("%s", ("Couldn't find file: " + s).c_str());
				return {};
			}
		}
	}
	const char* buffer = static_cast<const char*>(AAsset_getBuffer(asset));
	size_t length = AAsset_getLength(asset);
	LOGI("Asset: %s buffered. Size: %zu",s.c_str(),length);

	mode |= std::ios_base::out; // just to be safe

    std::string buf; buf.assign(buffer,length);
	std::stringstream sstr(buf,mode);

	AAsset_close(asset);
	return sstr;
#endif
}
void FileManager::WriteFile(const std::string& s, const std::stringstream& sstr, std::ios_base::openmode mode) {
#ifdef PLATFORM_WINDOWS
	mode |= std::ios_base::trunc; 
	std::filesystem::path filePath(PCWriteFilepath + s);
	filePath = filePath.parent_path();

	// Create a directory if it doesnt exist.
	if (!std::filesystem::exists(filePath)) {
		std::filesystem::create_directories(filePath);
	}

	std::fstream ofs(PCWriteFilepath + s,mode);
	if (!ofs) {
		LOGE("ERROR OPENING FILE %s", s.c_str());
		return;
	}

	ofs.write(sstr.str().c_str(), sstr.str().size());
	ofs.close();
#endif
#ifdef PLATFORM_ANDROID
	mode |= std::ios_base::trunc;
	std::filesystem::path filePath(AndroidWriteFilepath + s);
	filePath = filePath.parent_path();

	// Create a directory if it doesnt exist.
	if (!std::filesystem::exists(filePath)) {
		std::filesystem::create_directories(filePath);
	}

	std::fstream ofs(AndroidWriteFilepath + s, mode);
	if (!ofs) {
		LOGE("ERROR OPENING FILE %s", s.c_str());
		return;
	}

	ofs.write(sstr.str().c_str(), sstr.str().size());
	ofs.close();
#endif
}

#ifdef EditorFlag
void FileManager::EditorWriteFile(const std::string& s, const std::stringstream& sstr, std::ios_base::openmode mode) {
	mode |= std::ios_base::trunc;
	std::fstream ofs(s, mode);
	if (!ofs) {
		std::cout << "ERROR OPENING FILE: " << s << std::endl;
		return;
	}

	ofs.write(sstr.str().c_str(), sstr.str().size());
	ofs.close();
}
#endif