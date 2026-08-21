/*!
@file       DebugRender.h
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Handles all the drawing / rendering for debug (e.g.
			collision hitbox, velocity direction arrow, UI wireframes).


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "RenderUtils.h"
#include "glslshader.h"
#include "RenderAttributes.h"
#include "Registry.h"
#include <array>

class DebugRender {
	public:
		enum DebugShapes {	// the type of debugs renders
			QUAD_WIREFRAME = 0,		// quad wireframes
			CIRCLE_WIREFRAME,		// circle wireframes
			ARROW,					// arrow
			END_OF_DEBUG_SHAPES
		};

		enum DebugTypes {
			// ======= Uses QUAD_WIREFRAME =======
			ENTITY_COLLISION_QUAD_WIREFRAME = 0,
			UI_QUAD_WIREFRAME,
			PARTICLE_SPAWNBOX_WIREFRAME,

			// ======= Uses CIRCLE_WIREFRAME =======
			ENTITY_COLLISION_CIRCLE_WIREFRAME,
			PARTICLE_SPAWNCIRCLE_WIREFRAME,

			// ======= Uses ARROW =======
			VELOCITY_ARROW,

			END_OF_DEBUG_TYPES
		};
	public:
		DebugRender() = default;
		~DebugRender() = default;
		/*!
		* \brief Initialise the vao and vbo for instancing.
		*/
		void Init();

		/*!
		* \brief Updates the instancing buffer with the transforms for debug rendering.
		* \param[in] registry	- A reference to the registry that holds the entities and components.
		*/
		void UpdateInstancedTransform(Registry& registry);

		/*!
		* \brief Draws debug information (e.g. hitbox wireframe).
		* \param[in] shader		- The shader to use for rendering.
		*/
		void DrawDebug(GLSLShader& shader);

		/*!
		* \brief Draws collision wireframes.
		* \param[in] shader		- The shader to use for rendering, to pass uniforms to.
		* \param[in] viewProj	- The projection matrix.
		*/
		void DrawCollision(GLSLShader& shader, const Mat3& viewproj);

		/*!
		* \brief Draws UI wireframes.
		* \param[in] shader		- The shader to use for rendering, to pass uniforms to.
		* \param[in] viewProj	- The projection matrix.
		*/
		void DrawUI(GLSLShader& shader, const Mat3& proj);

		/*!
		* \brief Draw velocity direction arrows.
		* \param[in] shader		- The shader to use for rendering, to pass uniforms to.
		* \param[in] viewProj	- The projection matrix.
		*/
		void DrawVelocity(GLSLShader& shader, const Mat3& viewProj);

		/*!
		* \brief Draws the particle spawn box for particle emitters.
		* \param[in] shader		- The shader to use for rendering, to pass uniforms to.
		* \param[in] viewProj	- The projection matrix.
		*/
		void DrawParticleSpawnbox(GLSLShader& shader, const Mat3& viewProj);

		/*!
		* \brief Draw velocity direction arrows.
		* \param[in] shader		- The shader to use for rendering.
		*/
		void DrawCamera();

		/*!
		* \brief Draws debug with instancing.
		* \param[in] shader		- The instancing shader to use for rendering.
		*/
		void DrawDebugInstanced(GLSLShader& shader);
		/*!
		* \brief Draws the collision wireframes using instancing.
		*/
		void DrawCollisionInstanced(GLSLShader& shader);
		/*!
		* \brief Draws the collision wireframes using instancing.
		*/
		void DrawUIInstanced();
		/*!
		* \brief Draws the velocity arrows using instancing.
		*/
		void DrawVelocityInstanced();

		/*!
		* \brief Draws the particle spawn box for particle emitters.
		*/
		void DrawParticleSpawnboxInstanced(GLSLShader& shader);

		/*!
		* \brief Free up the resources used for instancing.
		*/
		void Free();

		/*!
		* \brief Set the colour to use for debug drawing.. Please use normalised RGB values [0, 1.0].
		* \param[in] clr - A const reference to a vec3 that holds the rgb values to draw all collision wireframes with.
		*/
		inline void SetDebugColour(DebugTypes type, const glm::vec3& clr) { SetDebugColour(type, clr[0], clr[1], clr[2]); }
		/*!
		* \brief Set the colour to use for debug drawing.. Please use normalised RGBA values [0, 1.0].
		* \param[in] clr - A const reference to a vec4 that holds the rgba values to draw all collision wireframes with.
		*/
		inline void SetDebugColour(DebugTypes type, const Color& clr) {
			if (type == END_OF_DEBUG_TYPES) debugColors.fill(clr);
			else debugColors[type] = clr;
		}
		/*!
		* \brief Set the colour to use for debug drawing. Please use normalised RGBA values [0, 1.0].
		* \param[in] r - Normalised value to set for the red component.
		* \param[in] g - Normalised value to set for the green component.
		* \param[in] b - Normalised value to set for the blue component.
		* \param[in] a - Normalised value to set for the alpha component, defaulted to 1.f.
		*/
		inline void SetDebugColour(DebugTypes type, float r, float g, float b, float a = 1.f) {
			if (type == END_OF_DEBUG_TYPES) debugColors.fill({ r, g, b, a });
			else debugColors[type] = { r, g, b, a };
		}
		/*!
		* \brief Getter for the line width for collision wireframes.
		* \return The current line width used to draw collision wireframes.
		*/
		GLfloat GetLineWidth() const { return lineWidth; }

		/*!
		* \brief Set the line width for the wireframes.
		* \param[in] width	- The line width to use for drawing the wireframes.
		*/
		void SetLineWidth(GLfloat width) { lineWidth = width < 1.f ? 1.f : width; }

		/*!
		* \brief Returns the colour used for debug drawing.
		* \return A const float pointer to the first element (red) of the rgba.
		*/
		Color& DebugColour(DebugTypes type) { return debugColors[type]; }
	private:
		/*!
		* \brief Helper function for initializing the instance debug buffers
		* \param[in] transform		- The container for the instance transforms.
		* \param[in] instanceMesh	- The vao & vbo for the instance buffer.
		* \param[in] type			- The debug type to init.
		*/
		void InitInstance(std::vector<Mat3>& transform, PrimitiveMesh& instanceMesh, DebugShapes type);
		/*!
		* \brief Helper function to update the instance buffer on GPU side.
		*/
		void UpdateInstanceBuffer();
		/*!
		* \brief Helper function for resizing the instance buffer if exceed instance buffer limit.
		* \param[in] buffer		- Which debug instance buffer to resize 
		* \param[in] size		- if size >= current buffer size, buffer size is resized to size * 2.
		*/
		void ResizeInstanceBuffer(DebugShapes buffer, size_t size = 0);

		/*!
		* \brief 
		*	Helper function for offsetting the buffer (to replicate base instance rendering as OpenGL ES
		*	does not have those function). 
		
		*	It will bind and unbind vbo in the function so just remember to bind VAO before calling and after.
		* 
		* \param[in, out] instance	- Which primitive instance to offset.
		* \param[in] offset			- The offset, in bytes.
		*/
		void OffsetInstanceBuffer(PrimitiveMesh& instance, GLuint offset);
	private:
		std::vector<Mat3> quadWireframeTransforms;		// contains all the xforms for quad wireframes
		std::vector<Mat3> circleWireframeTransforms;	// contains all the xforms for circle wireframes
		std::vector<Mat3> arrowTransforms;				// contains all the xforms for arrows

		PrimitiveMesh quadWireframeInstance;			// vao and vbo for instance rendering quad wireframes
		PrimitiveMesh circleWireframeInstance;			// vao and vbo for instance rendering circle wireframes
		PrimitiveMesh arrowInstance;					// vao and vbo for instance rendering arrows (for velocity debug)

		// the count for each instance
		std::array<int, DebugTypes::END_OF_DEBUG_TYPES> instanceCount{};
		// the size of instance buffer for each debug type, set to 2500 initially
		std::array<int, DebugShapes::END_OF_DEBUG_SHAPES> instanceLimits{};

		std::array<Color, DebugTypes::END_OF_DEBUG_TYPES> debugColors;

		//Color debugClr{ };		// the colour to use for drawing collision wireframes
		GLfloat lineWidth{ 2.f };					// the line width to use for rendering wireframes
	public:
		Vec2 velocityArrowSize{ 10.f, 15.f };		// the size to draw the velocity direction arrow
		bool debugDrawEnabled{ false };				// whether to draw debug
		bool collisionDebugEnabled{ true };			// whether to draw collision wireframes
		bool velocityDebugEnabled{ true };			// whether to draw velocity arrows
		bool uiDebugEnabled { true };				// whether to draw UI wireframe
		bool particleDebugEnabled{ true };			// whether to draw particle spawn area wireframe
};