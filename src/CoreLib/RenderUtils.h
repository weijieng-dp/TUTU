/*!
@file       RenderUtils.h
@author     Tan Jun Jie (t.junjie) 90%
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/09/2025
@brief		Handles primitives and font rendering.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include "Platform.h"
#include "RenderAttributes.h"
#include "MathLib.h"
#include "ResourceManager.h"
#include "font.h"
#include "CEO.h"

struct PrimitiveMesh {
	RenderAttributes	vao;
	RenderBuffer		vbo;

	void Free() { vao.DeleteVAO(); vbo.DeleteRenderBuffer(); }
};

struct InstanceData {
	Mat3 xform{};		// transform matrix (mdl to ndc)
	Vec2 sOrigin{};		// sprite origin [for sprite animation]
	Vec2 sSize{};		// sprite size [for sprite animation]
	GLuint id{};		// the entity_id
	Color color;
	Vec2 sTile{};
	int emissiveState{};

	InstanceData() = default;
	InstanceData(const Mat3& transform, const Vec2& origin, const Vec2& size, GLuint _id) :
		xform(transform), sOrigin(origin), sSize(size), id(_id) { }
};

struct BatchInfo {
	TextureObj* texture;
	TextureObj* emissiveTexture;
	GLuint offset;

	BatchInfo() : texture(nullptr), emissiveTexture(nullptr), offset(0) { }
	BatchInfo(TextureObj* tex, TextureObj* emissiveTex,GLuint _offset) : texture(tex), emissiveTexture{emissiveTex}, offset(_offset) {}
};

class RenderUtils {
	public:
		/*!
		* \brief Initialises all the VAOs and VBOs to be used by RenderUtils.
		*/
		void Init();	

		/*!
		* \brief Updates the entity instance GPU buffer with the data provided.
		* \param[in] xforms	- The vector that contains the instanceData to update. 
		*/
		void UpdateEntityInstanceBuffer(const std::vector<InstanceData>& xforms, size_t size);

		/*!
		* \brief 
		*	Updates the particle instance GPU buffer with the data provided. Will initialise vao & vbo
		*	if limit == 0.
		* \param[in] xforms	- The vector that contains the particle instanceData to update.
		* \param[in] limit	- The upper limit of the buffer.
		* \param[in] size	- The size of the particles to update (number of particles * sizeof(InstanceData)) 
		*/
		void UpdateParticleInstanceBuffer(const std::vector<InstanceData>& xforms, TextureObj* tex, size_t limit, size_t size);

		/*!
		* \brief Get the max depth value for rendering.
		* \return The max render depth value.
		*/
		GLuint GetMaxRenderDepth() const { return maxDepth; }

		size_t GetEntityInstanceLimit() const { return instanceLimit; }
		size_t GetParticleInstanceLimit() const { return particleInstanceLimit; }

		void SetEntityInstanceLimit(size_t newLimit) { instanceLimit = newLimit; }
		void SetParticleInstanceLiimit(size_t newLimit) { particleInstanceLimit = newLimit; }

		/*!
		* \brief Renders a quad wireframe, uses indexed rendering.
		* 
		*	Bind the Quad wireframe VAO beforehand with BindQuadWireframeVAO() and unbind after with
		*	RenderAttributes::Unbind().
		*/
		void RenderQuadWireframe();

		/*!
		* \brief Renders a quad. Draws it in at origin with size of half the viewport.
		*/
		void RenderQuad();

		/*!
		* \brief Renders a circle (with 66 slices).
		*/
		void RenderCircle();
		
		/*!
		* \brief Renders a circle wireframe. Reuses the circle vertices.
		*/
		void RenderCircleWireframe();

		/*!
		* \brief
		*	Renders a line. Reuses the first two quad vertices [-0.5f, 0.5f], [-0.5f, -0.5f].
		*/
		void RenderLine();

		/*!
		* \brief
		*	Renders a triangle. Reuses the first three quad vertices [-0.5f, 0.5f], [-0.5f, -0.5f],
		*	[0.5f, 0.5f].
		*/
		void RenderTriangle();

		/**
		* \brief Renders a quad wireframe, uses indexed rendering. Draws the quad wireframe with line loop.
		*
		*	Bind the Quad wireframe VAO beforehand with BindQuadWireframeVAO() and unbind after with
		*	RenderAttributes::Unbind().
		*/
		inline void RenderQuadWireframeRaw() { glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, NULL); }

		/*!
		* \brief Renders a quad. Draws it in at origin with size of half the viewport.
		*
		*	Bind the Quad VAO beforehand with BindQuadVAO() and unbind after with
		*	RenderAttributes::Unbind().
		*/
		inline void RenderQuadRaw() { glDrawArrays(GL_TRIANGLES, 0, 6); /* draws the quad with triangles */ }


		/*!
		* \brief Renders a circle using triangle fan (with 66 slices).
		*
		*	Bind the Circle VAO beforehand with BindCircleVAO() and unbind after with
		*	RenderAttributes::Unbind().
		*/
		inline void RenderCircleRaw() { glDrawArrays(GL_TRIANGLE_FAN, 0, 66); }


		/*!
		* \brief Renders a circle wireframe using GL_LINE_LOOP. Reuses the circle vertices.
		*
		*	Bind the Circle VAO beforehand with BindCircleVAO() and unbind after with
		*	RenderAttributes::Unbind().
		*/
		inline void RenderCircleWireframeRaw() { glDrawArrays(GL_LINE_LOOP, 1, 65); }

		/*!
		* \brief
		*	Renders a line. Reuses the first two quad vertices [-0.5f, 0.5f], [-0.5f, -0.5f].
		*
		*	Bind the Quad VAO beforehand with BindQuadVAO() and unbind after with RenderAttributes::Unbind().
		*/
		inline void RenderLineRaw() { glDrawArrays(GL_LINES, 0, 2); }

		/*!
		* \brief
		*	Renders a triangle. Reuses the first three quad vertices [-0.5f, 0.5f], [-0.5f, -0.5f], [0.5f, 0.5f].
		*
		*	Bind the Quad VAO before using with BindQuadVAO() and unbind VAO after with RenderAttributes::Unbind()
		*/
		inline void RenderTriangleRaw() { glDrawArrays(GL_TRIANGLES, 0, 3); }

		/*!
		* \brief Bind font shader and quad vao befre this function call.
		* \param[in] text		- The text to render, of type const reference to string.
		* \param[in] view		- view transform
		* \param[in] pos		- position of text
		* \param[in] fontSize	- size of text in pixels
		* \param[in] shader		- The font shader
		* \param[in] font		- font to render text in
		*/
		void RenderTextRaw(std::string const& text, Mat3 view, Vec2 pos, float fontSize, GLSLShader& shader,
			FontObj const& font = CEO::Instance().GetManager<ResourceManager>()->GetFont("pixel.ttf"));
		/*!
		* \brief Renders text.
		* \param[in] text		- The text to render, of type const reference to string.
		* \param[in] view		- view transform
		* \param[in] pos		- position of text
		* \param[in] fontSize	- size of text in pixels
		* \param[in] color		- color of text
		* \param[in] font		- font to render text in
		*/
		void RenderText(std::string const& text, Mat3 view, Vec2 pos, float fontSize,
			Color color = { 1.f,1.f,1.f }, FontObj const& font = CEO::Instance().GetManager<ResourceManager>()->GetFont("pixel.ttf"));
		
		/*!
		* \brief
		*	Instanced Rendering call (non-indexed).
		* 
		*	Bind the appropriate VAO and/or VBO before and after calling.
		*
		* \param[in] primCount	- The number of instances to draw.
		* \param[in] primType	- The type of primitive to draw with, defaulted to GL_TRIANGLES
		* \param[in] offset		- The starting index in the buffer, defaulted to 0.
		* \param[in] count		- The number of indices to render depending of primType, defaulted to 6.
		*/
		inline void RenderInstanced(GLsizei primCount, GLenum primType = GL_TRIANGLES, GLint offset = 0, 
			GLsizei count = 6) { glDrawArraysInstanced(primType, offset, count, primCount); }

		/*!
		* \brief
		*	Instanced Rendering call (indexed).
		*
		*	Bind the appropriate VAO and/or VBO before and after calling.
		*
		* \param[in] primCount		- The number of instances to draw.
		* \param[in] primType		- The type of primitive to draw with, defaulted to GL_TRIANGLES
		* \param[in] count			- The number of indices to render depending of primType, defaulted to 6.
		* \param[in] type			- The type of value of the indices, defaulted to GL_UNSIGNED_SHORT. Accepted value: [GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT].
		* \param[in] indices		- If an index buffer is bound, specifies the byte offset (casted to pointer type). Else, specifies a pointer to where the indices is located.
		*/
		inline void RenderInstancedIndexed(GLsizei primCount, GLenum primType = GL_TRIANGLES, GLsizei count = 6,
			GLenum type = GL_UNSIGNED_SHORT, void* indices = NULL) { 
			glDrawElementsInstanced(primType, count, type, indices, primCount);
		}

		/*!
		* \brief Frees up all the opengl objects from RenderUtils.
		*/
		void Free();

		/*!
		* \brief Getter for the VAO used for entity instancing by RenderUtils.
		* \return An RenderAttributes reference to the entity instance VAO.
		*/
		inline RenderAttributes& GetEntityInstanceVAO() { return entityInstance.vao; }
		/*!
		* \brief Getter for the VBO used for entity instancing by RenderUtils.
		* \return An RenderBuffer reference to the entity instance VBO.
		*/
		inline RenderBuffer& GetEntitynstanceVBO() { return entityInstance.vbo; }

		/*!
		* \brief Getter for the VAO used for particle instancing by RenderUtils.
		* \return An RenderAttributes reference to the particle instance VAO.
		*/
		inline RenderAttributes& GetParticlesInstanceVAO(TextureObj* tex) { return particleInstance[tex].vao; }

		/*!
		* \brief Getter for the VBO used for particle instancing by RenderUtils.
		* \return An RenderBuffer reference to the particle instance VBO.
		*/
		inline RenderBuffer& GetParticlesInstanceVBO(TextureObj* tex) { return particleInstance[tex].vbo; }

		/*!
		* \brief Getter for the VBO used for quad rendering by RenderUtils.
		* \return An RenderBuffer reference to the quad VBO.
		*/
		inline RenderBuffer& QuadVBO() { return quad.vbo; }

		inline RenderBuffer& CircleVBO() { return circle.vbo; }
		/*!
		* \brief Binds the VAO used for circle rendering by RenderUtils.
		*/
		inline void BindCircleVAO() { circle.vao.Bind(); }
		//static inline void BindCircleVBO() { circle.vbo.Bind(); }

		/*!
		* \brief Binds the VAO used for quad rendering by RenderUtils.
		*/
		inline void BindQuadVAO() { quad.vao.Bind(); }
		//static inline void BindQuadVBO() { quad.vbo.Bind(); }

		/*!
		* \brief Binds the VAO used for entity instance rendering by RenderUtils.
		*/
		inline void BindEntityInstanceVAO() { entityInstance.vao.Bind(); }
		//static inline void BindInstanceVBO() { instance.vbo.Bind(); }

		/*!
		* \brief Binds the VAO used for particle instance rendering by RenderUtils.
		*/
		inline void BindParticleInstanceVAO(TextureObj* tex) { particleInstance[tex].vao.Bind(); }

		/*!
		* \brief Binds the VAO used for quad wireframe rendering by RenderUtils.
		*/
		inline void BindQuadWireframeVAO() { quadWireframeVao.Bind(); }
		/*!
		* \brief Binds the EBO used hold the indices for quad wireframe rendering by RenderUtils.
		*/
		inline void BindQuadWireframeEBO() { quadWireframeEbo.Bind(); }
	private:	// private functions
		/*!
		* \brief  Initialises a default quad mesh that centers at origin with size of half the viewport.
		*/
		void InitQuad();
		/*!
		* \brief Initialises a default circle mesh with 64 slices.
		*/
		void InitCircle();
		/*!
		* \brief Initialises a quad wireframe mesh, reuses the vbo of quad but with it's own vao and ebo.
		*/
		void InitQuadWireframe();
		/*!
		* \brief Initialises the VAO and VBO to use for instance rendering.
		*/
		void InitInstancing(PrimitiveMesh& instanceMesh, size_t instanceLimit);
	private:
		// ===== Instancing =====
		std::unordered_map<TextureObj*, PrimitiveMesh> particleInstance{};	// a map of particle instance buffers, grouped by texture
		PrimitiveMesh entityInstance{};			// vao and vbo for entity instanced rendering

		PrimitiveMesh circle{};					// the vao and vbo for a circle primitive
		PrimitiveMesh quad{};					// the vao and vbo for a quad primitive

		size_t instanceLimit{ 2500 };			// the size of the entity instance buffer, initial limit of 2500
		size_t particleInstanceLimit{ 2500 };	// use to set the initial size / limit of particle instance buffer

		// ==== For Drawing A Quad Wireframe ====
		RenderAttributes quadWireframeVao{};	// the vao used for a quad wireframe
		RenderBuffer quadWireframeEbo{ GL_ELEMENT_ARRAY_BUFFER };			// the ebo for a quad wireframe

		GLuint maxDepth{ 1000 };				// the max rendering layer

		bool initialised{ false };				// a flag for whether RenderUtils has been initialised	
};