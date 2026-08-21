/*!
@file       Texture.h
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for texture object. Loading and unloading is handled
			by the ResourceManager.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#ifdef  PLATFORM_WINDOWS
#include <GL/glew.h>		// for access to OpenGL API declarations
#else
#include <GLES3/gl3.h>

#endif
#include <string>
#include <vector>

class TextureObj {
	public:
		struct WrapMode {
			WrapMode() = default;
			WrapMode(GLint _s, GLint _t) : s(_s), t(_t) {}
			GLint s{GL_REPEAT};
			GLint t{GL_REPEAT};
		};
		struct FilterMode {
			FilterMode() = default;
			FilterMode(GLint _min, GLint _mag) : min(_min), mag(_mag) { }
			GLint min{ GL_NEAREST };
			GLint mag{ GL_NEAREST };
		};

	public:
		/*!
		* \brief 
		*	Default Constructor, doesn't load texture. Use Load() or ResourceManager::LoadTexture()
		*	to load the texture into opengl.
		*/
		TextureObj() : texId(0), wrapMode(), filterMode(), width(0), height(0), isLoaded(GL_FALSE), 
			useBlend(GL_TRUE) { }

		/**
		* \brief
		*	Creates a texture object with the specified file name, wrap & filter mode, width, and height.
		*	Whether this texture object uses blend function is defaulted to true.
		* 
		* \param [GLuint] The texID
		* \param [GLsizei] Width of the image
		* \param [GLsizei] Height of the image
		* \param [GLboolean] If the image is using blend
		* \param [GLboolean] If the image is loaded (default false)
		*/
		TextureObj(GLuint _texId, GLsizei _width, GLsizei _height, GLboolean _useBlend, GLboolean _isLoaded = GL_FALSE) : texId{ _texId }, wrapMode{}, filterMode{},
			width{ _width }, height{ _height }, isLoaded{ _isLoaded }, useBlend{ _useBlend } {}


		/*! \brief Defaulted Destructor	*/
		~TextureObj() = default;

		TextureObj(const TextureObj&) = delete;				// delete copy constructor to prevent copying
		TextureObj& operator=(const TextureObj&) = delete;	// delete copy assignment to prevent copying

		TextureObj(TextureObj&& other) noexcept;
		TextureObj& operator=(TextureObj&& other) noexcept;

		/*!
		* \brief Sets the wrap mode of texture object to the ones specified in wm.
		* \param[in] wm - The wrap mode to set.
		*/
		void SetWrapMode(const WrapMode& wm);
		/*!
		* \brief Sets the wrap mode of texture coordinate s for the current texture object.
		* \param[in] s - The wrap mode to set for texture coordinate s.
		*/
		void SetWrapModeS(GLint s);
		/*!
		* \brief Sets the wrap mode of texture coordinate t for the current texture object.
		* \param[in] t - The wrap mode to set for texture coordinate t.
		*/
		void SetWrapModeT(GLint t);

		/*!
		* \brief 
		*	Sets what texture minifying and magnifying function to use for this 
		*	texture object.
		*
		*	Allowed minifying filter mode: [GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
		*	GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR].
		* 
		*	Allowed mangnifying filter mode: [GL_NEAREST, GL_LINEAR].
		*
		* \param[in] fm - The filter mode to set.
		*/
		void SetFilterMode(const FilterMode& fm);
		/*!
		* \brief Sets what texture minifying function to use for this texture object.
		*
		*	Allowed filter mode: [GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
		*	GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR].
		*
		* \param[in] fm - The texture minifying function to set.
		*/
		void SetMinFilterMode(GLint fm);
		/*!
		* \brief Sets what texture magnification function to use for this texture object.
		*
		*	Allowed filter mode: [GL_NEAREST, GL_LINEAR].
		*
		* \param[in] fm - The filter mode to use for texture magnification.
		*/
		void SetMagFilterMode(GLint fm);

		/*!
		* \brief Set whether this texture uses blend functions.
		* \param[in] blend - GL_TRUE if to use blend function with this texture, otherwise GL_FALSE.
		*/
		void SetBlend(GLboolean blend);

		/*! \brief Binds the current texture. */
		void BindTexture() const { glBindTexture(GL_TEXTURE_2D, texId); }
		/*! \brief Unbinds all current texture. */
		static void UnbindTexture() { glBindTexture(GL_TEXTURE_2D, 0); }

		/*! \brief Apply blend function (use glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)) */
		bool ApplyBlend(float alpha = 1.f) const;

		/*!
		* \brief Apply all the texture parameters of this texture object.
		* 
		* \param[in] texBound - Whether texture was bound outside of this
		*	function, by default it's set to true. Pass in false to let
		*	function handle binding and unbinding.
		*/
		void ApplyTextureParam(bool texBound = true) const;

		/*!
		* \brief Getter for tex id.
		* \return A const reference to the texture id of the current texture object.
		*/
		const GLuint& TexId() const { return texId; }
		/*!
		* \brief Getter for width of current texture object.
		* \return The width of the current texture object, of type GLsizei.
		*/
		GLsizei Width() const { return width; }
		/*!
		* \brief Getter for height of current texture object.
		* \return The height of the current texture object, of type GLsizei.
		*/
		GLsizei Height() const { return height; }
		/*!
		* \brief Getter for whether current texture object is loaded into memory.
		* \return GL_TRUE if loaded, GL_FALSE otherwise.
		*/
		GLboolean IsLoaded() const { return isLoaded; }
		/*!
		* \brief Getter for whether to use blend function for current texture object.
		* \return GL_TRUE if to use blend, GL_FALSE otherwise.
		*/
		GLboolean IsUseBlend() const { return useBlend; }


		/*!
		* \brief
		*    Retrieves a read-only pointer to the texture’s pixel data buffer.
		*
		* \param
		*    [None]
		*
		* \return
		*    [const unsigned char*] Pointer to the first byte of pixel data, or
		*    nullptr if no pixel data is available.
		*/
		const unsigned char* PixelData() const { return pixelData.empty() ? nullptr : pixelData.data(); }

		/*!
		* \brief
		*    Retrieves a writable pointer to the texture’s pixel data buffer.
		*
		* \param
		*    [None]
		*
		* \return
		*    [unsigned char*] Pointer to the first byte of pixel data, or
		*    nullptr if no pixel data is available.
		*/
		unsigned char* PixelData() { return pixelData.empty() ? nullptr : pixelData.data(); }

		/*!
		* \brief
		*    Returns the number of color channels contained in the texture.
		*
		* \param
		*    [None]
		*
		* \return
		*    [GLint] The number of channels (e.g., 3 for RGB, 4 for RGBA).
		*/
		GLint Channels() const { return channels; }

		/*!
		* \brief
		*    Indicates whether the texture contains any loaded pixel data.
		*
		* \param
		*    [None]
		*
		* \return
		*    [bool] True if pixel data exists, false if the texture buffer is empty.
		*/
		bool HasPixelData() const { return !pixelData.empty(); }

	private:	// private functins
		/*!
		* \brief Check whether wm is a valid wrap mode.
		* 
		*	Valid wrap modes: [GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE,
		*	GL_CLAMP_TO_BORDER (only on Windows)]
		*
		* \return GL_TRUE if valid, GL_FALSE otherwise.
		*/
		static bool IsValidWrapMode(GLint wm);
		/*!
		* \brief Check whether fm is a valid minifying filter mode.
		*
		*	Allowed filter mode: [GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
		*	GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR].
		*
		* \return GL_TRUE if valid, GL_FALSE otherwise.
		*/
		static bool IsValidMinFilterMode(GLint fm);
		/*!
		* \brief Check whether fm is a valid minifying filter mode.
		*
		*	Allowed filter mode: [GL_NEAREST, GL_LINEAR].
		*
		* \return GL_TRUE if valid, GL_FALSE otherwise.
		*/
		static bool IsValidMagFilterMode(GLint fm);

	private:
		GLuint		texId;						// the texture id
		WrapMode	wrapMode;					// Texture Wrap modes for this texture object
		FilterMode	filterMode;					// Texture Filter modes for this texture object
		GLsizei		width, height;				// width & height of the texture
		GLboolean	isLoaded;					// whether texture is currently loaded or not	
		GLboolean	useBlend;					// whether to use blend function
		std::vector<unsigned char> pixelData;	// stores pixel data to fit collision box
		GLint channels = 0;						// stores channel count for use in fitting collision box

		friend class TextureManager;
};
