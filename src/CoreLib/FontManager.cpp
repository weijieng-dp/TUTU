/**___________________________________________________________________________/
@file          FontManager.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple private class to handle the loading and unloading of font textures
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "FontManager.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#ifdef PLATFORM_ANDROID
#include "CEO.h"
#endif

void FontManager::Init() {
	// empty for now
}

void FontManager::Free() {
	// empty for now
}

FontObj FontManager::LoadFont(const std::string& filepath, unsigned int resolution) {
	std::string realname = filepath + std::to_string(resolution);
	FT_Library ftLib;
	FT_Face fontFace;
	FT_Error error = FT_Init_FreeType(&ftLib);
	if (error)
	{
		LOGE("Failed to load font library");
        FT_Done_FreeType(ftLib);
		return FontObj();
	}

#ifdef PLATFORM_ANDROID
	std::stringstream stream = CEO::Instance().GetManager<FileManager>()->ReadFile(filepath, std::ios_base::binary | std::ios_base::out);
	if (stream.str().empty()) {
		LOGE("Unable to find font file");
        FT_Done_FreeType(ftLib);
		return FontObj();
	}
	FT_Byte* binaryFontData = new FT_Byte[stream.str().size()];
	memcpy(binaryFontData, stream.str().c_str(), stream.str().size());
	error = FT_New_Memory_Face(ftLib, binaryFontData, stream.str().length(), 0, &fontFace);
#endif
#ifdef PLATFORM_WINDOWS
	error = FT_New_Face(ftLib, filepath.c_str(), 0, &fontFace);
#endif
	if (error)
	{
		LOGE("Failed to load font, error code: %d", error);
        //FT_Done_Face(fontFace);
        //FT_Done_FreeType(ftLib);
		return FontObj();
	}
	error = FT_Set_Pixel_Sizes(fontFace, resolution, resolution);
	if (error)
	{
		LOGE("Failed to set font resolution to %d error code: %d", resolution, error);
	}

	// OpenGL requires that textures all have a 4-byte alignment, disable for char textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	FontObj fmap;
	fmap.resolution = resolution;

	LOGI("Loading font: %s", filepath.c_str());
	// creates a texture for most keyboard characters (unicode 32 to 126)
	// taken from https://learnopengl.com/In-Practice/Text-Rendering
	for (unsigned char c = 32; c < 127; c++)
	{
		// load character glyph 
		error = FT_Load_Char(fontFace, c, FT_LOAD_RENDER);
		if (error)
		{
			LOGE("Failed to load font character: %c, error code: %d", c, error);
			continue;
		}
		//LOGI("Loading glyph:%c, sizeX: %u, sizeY: %u, bearingX: %d, bearingY: %d, advance: %u", c, fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows, fontFace->glyph->bitmap_left, fontFace->glyph->bitmap_top, fontFace->glyph->advance.x);

		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
            GL_R8,
			fontFace->glyph->bitmap.width,
			fontFace->glyph->bitmap.rows,
			0,
            GL_RED,
			GL_UNSIGNED_BYTE,
			fontFace->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// now store character for later use
		FontObj::Character character = {
			texture,
			static_cast<int>(fontFace->glyph->bitmap.width), static_cast<int>(fontFace->glyph->bitmap.rows),
			fontFace->glyph->bitmap_left, fontFace->glyph->bitmap_top,
			static_cast<int>(fontFace->glyph->advance.x >> 6) // advance is number of 1/64 pixels
		};
		//LOGI("Loading char:%c, sizeX: %u, sizeY: %u, bearingX: %d, bearingY: %d, advance: %u",c, character.sizeX, character.sizeY, character.bearingX, character.bearingY, character.advance);
		fmap.map[c] = character;
	}
	fmap.loaded = true;
	fmap.path = filepath;
	FT_Done_Face(fontFace);
	FT_Done_FreeType(ftLib);

#ifdef PLATFORM_ANDROID
	delete [] binaryFontData;
#endif
	return fmap;
}

void FontManager::FreeFont(FontObj& obj) {
#ifdef PLATFORM_ANDROID
	bool contextExists = (eglGetCurrentContext() != EGL_NO_CONTEXT);
#endif
	for (std::pair<const unsigned char, FontObj::Character>& p : obj.map) {
		if (p.second.texID != 0) {
#ifdef PLATFORM_ANDROID
			if(contextExists) // check if egl context exists before deleting
#endif
			glDeleteTextures(1, &p.second.texID);
		}
			p.second.texID = 0;
	}
	obj.loaded = false;
}