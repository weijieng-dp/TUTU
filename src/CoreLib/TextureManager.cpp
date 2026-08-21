/**___________________________________________________________________________/
@file          TextureManager.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple private class to handle the loading and unloading of openGL textures
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "TextureManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void TextureManager::Init() {
	// empty for now
}

void TextureManager::Free() {
	// empty for now
}

TextureObj TextureManager::LoadTex(const std::string& filePath) {
	GLsizei width{}, height{};
	GLint channels{};		// number of colour channels of the image read (3 for RGB, 4 for RGBA)
	GLuint texId{};
	GLboolean useBlend;

	stbi_set_flip_vertically_on_load(true);	// flip the texture vertically
	std::stringstream sstr = CEO::Instance().GetManager<FileManager>()-> ReadFile(filePath, false, std::ios_base::binary | std::ios_base::in);

	LOGI("Loading texture: %s", filePath.c_str());

	unsigned char* img{ stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(sstr.str().c_str()),
		static_cast<int>(sstr.str().size()), &width, &height, &channels, 0) };
	// error check
	if (img == NULL) {
		LOGE("Error in Loading Texture: %s", filePath.c_str());
		return TextureObj();
	}

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	if (channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		useBlend = GL_FALSE;
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		useBlend = GL_TRUE;
	}

	TextureObj obj(texId, width, height, useBlend, true);

	obj.pixelData.assign(img,img + (width * height * channels)); //get pixel data and channels for fitting of collision box
	obj.channels = channels;

	stbi_image_free(img);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	return obj;
}

TextureObj TextureManager::LoadTexFromMemory(const unsigned char* data, int size)
{
	GLsizei width{}, height{};
	GLint channels{};		// number of colour channels of the image read (3 for RGB, 4 for RGBA)
	GLuint texId{};
	GLboolean useBlend;

	stbi_set_flip_vertically_on_load(true);	// flip the texture vertically

	unsigned char* img{ stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data),
		size, &width, &height, &channels, 0) };
	// error check
	if (img == NULL) {
		LOGE("Error Loading Texture from memory");
		return TextureObj();
	}

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	if (channels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		useBlend = GL_FALSE;
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
		useBlend = GL_TRUE;
	}

	TextureObj obj(texId, width, height, useBlend, true);

	obj.pixelData.assign(img, img + (width * height * channels)); //get pixel data and channels for fitting of collision box
	obj.channels = channels;

	stbi_image_free(img);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);

	return obj;
}

void TextureManager::FreeTex(TextureObj& obj) {
	if (obj.IsLoaded()) {
		LOGI("Deleting texture id: %d", obj.TexId());
#ifdef PLATFORM_ANDROID
		if (eglGetCurrentContext() != EGL_NO_CONTEXT)
#endif
		glDeleteTextures(1, &obj.TexId());	// delete texture
		obj.texId = 0;						// reset id back to 0
		obj.isLoaded = false;				// set flag for loaded to false
	}
}