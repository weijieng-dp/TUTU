/*!
@file       RenderAttributes.h
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for setting attributes for VAO


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include "Platform.h"
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#endif
#include "RenderBuffer.h"

class RenderAttributes {
	public:
		enum ATTRIBUTE_INDEX : GLuint {	// used for vertex attribute index
			ATTRIB_POSITION = 0,
			ATTRIB_TEXTURE = 1,
			ATTRIB_TRANSFORM1 = 2,
			ATTRIB_TRANSFORM2 = 3,
			ATTRIB_TRANSFORM3 = 4,
			ATTRIB_SPRITE_ORIGIN = 5,
			ATTRIB_SPRITE_SIZE = 6,
			ATTRIB_ID = 7,
			ATTRIB_COLOR = 8,
			ATTRIB_TILE = 9,
			ATTRIB_EMISSION = 10,
		};

		/*!
		* \brief
		*    Default constructor for a opengl vao. DOESN'T glGenVertexArray, please
		*    use CreateVAO() for that.
		*/
		RenderAttributes() : vaoid(0) { }
		/*!
		* \brief Default destructor. Please call DeleteVAO() to free up resources
		*/
		~RenderAttributes() = default;

		/*!
		* \brief
		*    Move constructor for a opengl vao. Construct a RenderAttributes
		*	 object by stealing the vao from the other RenderAttributes object.
		*
		* \param other -
		*    A rvalue reference to RenderAttributes object to move construct
		*	 from.
		*/
		RenderAttributes(RenderAttributes&& other) noexcept;
		/*!
		* \brief
		*	Move assignment operator for a opengl vao. Steals the vao from
		*	the other RenderAttributes object.
		*
		* \param other
		*	- A rvalue reference to RenderAttributes object to move construct
		*	from.
		* 
		* \return 
		*	A reference to the RenderAttributes object after move
		*	assigning.
		*/
		RenderAttributes& operator=(RenderAttributes&& other) noexcept;


		// ===================== Deleted the Copy Constructor & Assignment =====================
		RenderAttributes(const RenderAttributes&) = delete;
		RenderAttributes& operator=(const RenderAttributes&) = delete;
		// =====================================================================================

		/*!
		* \brief 
		*	Returns the vaoid.
		*
		* \return 
		*	The vaoid of the RenderAttributes object.
		*/
		GLuint Id() const { return vaoid; }

		/*!
		* \brief Binds the vaoid of the calling RenderAttributes object as active.
		*/
		void Bind() const { glBindVertexArray(vaoid); }

		/*!
		* \brief Unbinds all vaoid. Calls glBindVertexArray(0)
		*/
		static void Unbind() { glBindVertexArray(0); }

		/*!
		* \brief Creates a VAO in opengl.
		*/
		void CreateVAO();

		/*!
		* \brief Deletes the VAO object.
		*/
		void DeleteVAO();

		/*!
		* \brief
		*	Sets attribute in VAO and binds to specified VBO, will create a VAO if 
		*	not already created.
		*	
		*	Example: Interleaved vertex {x, y, u, v} |
		*		stride = 4 * sizeof(GLfloat) |
		*		position relativeOffset = 0 |
		*		texture relativeOffset = 2 * sizeof(GLfloat)
		*
		* \param[in] vbo
		*	- The vbo that this vao binds to.
		* \param[in] attribIndex
		*	- The index of the attribute to associate with a vertex buffer binding.
		* \param[in] size
		*	- The number of values per vertex that are stored in the array.
		* \param[in] stride
		*	- The distance between consecutive vertices within the buffer.
		* \param[in] bufferOffset
		*	- The offset of the first element of the buffer.
		* \param[in] relativeOffset
		*	- The offset of this element within each vertex.
		* \param[in] type
		*	- The type of data as a GLenum, defaulted to GL_FLOAT
		* \param[in] normalised
		*	- Specifies whether fixed-point data values should be normalised. 
		*	Defaulted to GL_FALSE (not normalised)
		*/
		void SetAttribute(const RenderBuffer& buffer, ATTRIBUTE_INDEX attribIndex,
			GLint size, GLsizei stride, GLsizeiptr bufferOffset, GLuint relativeOffset, GLenum type = GL_FLOAT,
			GLboolean normalised = GL_FALSE);

		/*!
		* \brief
		*	Sets attribute in VAO. Please bind the VAO and VBO before calling this function
		*	and unbind after.
		*
		*	Example: Interleaved vertex {x, y, u, v} |
		*		stride = 4 * sizeof(GLfloat) |
		*		position relativeOffset = 0 |
		*		texture relativeOffset = 2 * sizeof(GLfloat)
		*
		* \param[in] attribIndex
		*	- The index of the attribute to associate with a vertex buffer binding.
		* \param[in] size
		*	- The number of values per vertex that are stored in the array.
		* \param[in] stride
		*	- The distance between consecutive vertices within the buffer.
		* \param[in] bufferOffset
		*	- The offset of the first element of the buffer.
		* \param[in] relativeOffset
		*	- The offset of this element within each vertex.
		* \param[in] type
		*	- The type of data as a GLenum, defaulted to GL_FLOAT
		* \param[in] normalised
		*	- Specifies whether fixed-point data values should be normalised. 
		*	Defaulted to GL_FALSE (not normalised)
		*/
		inline void SetAttribute(ATTRIBUTE_INDEX attribIndex, GLint size, GLsizei stride,
			GLsizeiptr bufferOffset, GLuint relativeOffset, GLenum type = GL_FLOAT, GLboolean normalised = GL_FALSE) {
			switch (type) {
				// intentional flow down
				case GL_BYTE: case GL_UNSIGNED_BYTE: case GL_SHORT: 
				case GL_UNSIGNED_SHORT: case GL_INT: case GL_UNSIGNED_INT:
					// for integral values
					glVertexAttribIPointer(attribIndex, size, type, stride, (void*)(bufferOffset + relativeOffset));
					break;
				case GL_HALF_FLOAT: case GL_FLOAT: case GL_FIXED: 
				case GL_INT_2_10_10_10_REV: case GL_UNSIGNED_INT_2_10_10_10_REV:
					// for floating-point values
					glVertexAttribPointer(attribIndex, size, type, normalised, stride, (void*)(bufferOffset + relativeOffset));
					break;
				default:
					LOGE("Invalid data type passed into Set Attribute.");
					break;
			}
		}

	private:
		GLuint vaoid;	// the id of the vertex array object
};