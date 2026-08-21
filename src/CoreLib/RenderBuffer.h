/*!
@file       RenderBuffer.h
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for interacting OpenGL buffer data.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include "Platform.h"
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#endif

class RenderBuffer {
	public:
		/*!
		* \brief 
		*	Default constructs a render buffer object, bufferType is defaulted as GL_ARRAY_BUFFER
		*	and bufferUsage is defaulted to GL_STATIC_DRAW. It doesn't generate a opengl buffer 
		*	upon call, use CreateRenderBuffer() or SetBuffer() for that.
		*/
		RenderBuffer() : vboid(0), bufferType(GL_ARRAY_BUFFER), bufferUsage(GL_STATIC_DRAW) { }
		/*!
		* \brief
		*	Single-argument constructor for a render buffer object, bufferUsage is defaulted as
		*	GL_STATIC_DRAW. It doesn't generate a opengl buffer upon call, use CreateRenderBuffer()
		*	or SetBuffer() for that.
		* 
		* \param[in] target
		*	- The intended type of the buffer, as a GLenum. 
		*	GL_ARRAY_BUFFER for vertex buffer | GL_ELEMENT_ARRAY_BUFFER for index buffer (ebo) | etc...
		*/
		RenderBuffer(GLenum target) : vboid(0), bufferType(target), bufferUsage(GL_STATIC_DRAW) { }
		/*!
		* \brief
		*	Double-argument constructor for a render buffer object. It doesn't generate a opengl 
		*	buffer upon call, use CreateRenderBuffer() or SetBuffer() for that.
		*
		* \param[in] target
		*	- The intended type of the buffer, as a GLenum.
		*	GL_ARRAY_BUFFER for vertex buffer | GL_ELEMENT_ARRAY_BUFFER for index buffer (ebo) | etc...
		* 
		* \param[in] usage
		*	- The usage pattern of the buffer, as a GLenum.
		*	GL_STATIC_DRAW if buffer contents will be modified once and used multiple times.
		*	GL_DYNAMIC_DRAW if buffer contents wil be modified repeatedly and used multiple times.
		*/
		RenderBuffer(GLenum target, GLenum usage) : vboid(0), bufferType(target), bufferUsage(usage) { }
		/*!
		* \brief Default destructor. Please call DeleteRenderBuffer() to free up resources.
		*/
		~RenderBuffer() = default;

		/*!
		* \brief 
		*	Move constructor. Steals the resources from the other buffer.
		* \param[in,out] buffer
		*	- The buffer to steal and construct from.
		*/
		RenderBuffer(RenderBuffer&& buffer) noexcept;
		/*!
		* \brief
		*	Move assignment. Steals the resources from the other buffer.
		* \param[in,out] buffer
		*	- The buffer to steal resourrces from.
		*/
		RenderBuffer& operator=(RenderBuffer&& buffer) noexcept;

		// ===================== Deleted the Copy Constructor & Assignment =====================
		RenderBuffer(const RenderBuffer&) = delete;
		RenderBuffer& operator=(const RenderBuffer&) = delete;
		// =====================================================================================

		// ======= Getters & Setters =======
		/*!
		* \brief
		*	Getter for RenderBuffer object id.
		*
		* \return
		*	The id of the RenderBuffer object.
		*/
		GLuint Id() const { return vboid; }
		/*!
		* \brief
		*	Getter for bufferType of RenderBuffer object.
		*
		* \return
		*	The bufferType as GLenum.
		*/
		GLenum BufferType() const { return bufferType; }
		/*!
		* \brief
		*	Setter for bufferType of RenderBuffer object.
		*
		* \return
		*	A GLenum reference to the bufferType.
		*/
		GLenum& BufferType() { return bufferType; }
		/*!
		* \brief
		*	Getter for bufferUsage of RenderBuffer object.
		*
		* \return
		*	The bufferUsage as GLenum.
		*/
		GLenum BufferUsage() const { return bufferUsage; }
		/*!
		* \brief
		*	Setter for bufferUsage of RenderBuffer object.
		*
		* \return
		*	A GLenum reference to the bufferUsage.
		*/
		GLenum& BufferUsage() { return bufferUsage; }

		/*!
		* \brief
		*	Binds the current RenderBuffer object at the bufferTarget of the
		*	calling RenderBuffer object.
		*/
		void Bind() const;

		/*!
		* \brief 
		*	Unbinds the RenderBuffer from the specified buffer target.
		* 
		* \param[in] target
		*	- The buffer target to unbind RenderBuffer from.
		*/
		static void Unbind(GLenum target);

		/*!
		* \brief Creates the render buffer, calls glGenBuffers().
		*/
		void CreateRenderBuffer();
		/*!
		* \brief Deletes the render buffer, calls glDeleteBuffers().
		*/
		void DeleteRenderBuffer();

		/*!
		* \brief 
		*	Allocates memory and copy data into VBO using glBufferData. Will create RenderBuffer if
		*	not already created.
		*
		* \param[in] data
		*	- The pointer to the data to copy into the RenderBuffer, or NULL/nullptr if no data to copy
		* \param[in] size
		*	- The size of the RenderBuffer, in bytes, to create.
		*/
		void SetBuffer(const void* data, GLsizeiptr size);

		/*!
		* \brief 
		*	Updates existing data in RenderBuffer object. Will orphan
		*	the buffer.
		*
		* \param[in] data
		*	- The pointer to the data to update into the RenderBuffer.
		* \param[in] size
		*	- The size of the whole buffer, in bytes.
		* \param[in] count
		*	- The size of the data, in bytes, to update.
		* \param[in] offset
		*	- The offset of the RenderBuffer to which to update into.
		*/
		void ConfigureBuffer(const void* data, GLsizeiptr size, GLsizeiptr count, GLintptr offset);
	private:
		GLuint vboid;				// the id of the buffer
		GLenum bufferType;			// what type of buffer is this [default is GL_ARRAY_BUFFER
		GLenum bufferUsage;			// the expected usage pattern of the data store.
};