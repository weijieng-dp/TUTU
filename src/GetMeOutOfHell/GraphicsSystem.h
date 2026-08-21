/*!
@file       GraphicsSystem.h
@author     Tan Jun Jie (t.junjie)	70%
@author		Ou Yukang (yukang.ou)	30%
@date       25/09/2025
@brief		Handles all the drawing / rendering for the game. Draws all
			entities and UI entities, minimap, and applies post processing
			(if applicable).

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include <Registry.h>

#include "RenderUtils.h"
#include "glslshader.h"

class GraphicsSystem {
	public:
		GraphicsSystem() = default;
		~GraphicsSystem() = default;
		/*!
		* \brief Initialize GraphicsSystem and DebugRender.
		*/
		void Init();

		/*!
		* \brief 
		*	Draws background and entities. Uses sprite shader for rendering.
		*	Will render debug if DebugRender is set to true.
		*
		* \param[in, out] registry - The entity registry to get the entities for drawing.
		*/
		void Draw(Registry& registry);

		/*!
		* \brief
		*	Used for updating information needed for instanced drawing. Handles
		*	updating DebugRender as well.
		*
		* \param[in, out] registry - The entity registry to get the entities for drawing.
		*/
		void Update(Registry& registry);
		
		/*!
		* \brief Draws the all active entities with instanced rendering.
		* \param[in, out] shader - The shader to use for rendering.
		*/
		void DrawEntitiesInstanced(GLSLShader& shader);

		/*!
		* \brief Draws the all active world entities.
		* \param[in, out] shader - The shader to use for rendering.
		*/
		void DrawEntities(GLSLShader& shader);

		/*!
		* \brief 
		*	Draws the all active UI entities using non-instance rendering. 
		*	Will also call DrawMiniMap();
		*
		* \param[in, out] registry	- The entity registry to get the entities for drawing.
		* \param[in, out] shader	- The shader to use for rendering.
		*/
		void DrawUIEntities(Registry& registry, GLSLShader& shader);
		/*!
		* \brief Draws the background.
		* \param[in, out] shader - The shader to use for rendering.
		*/
		void DrawBackground(GLSLShader& shader) const;

		/*!
		* \brief Updates minimap framebuffer.
		* \param[in, out] registry - The entity registry.
		*/
		void UpdateMinimap(Registry& registry);

		/*!
		* \brief Draws the minimap onto the game.
		* \param[in, out] registry	- The entity registry.
		* \param[in, out] shader	- The shader to use for drawing.
		*/
		void DrawMinimap(Registry& registry, GLSLShader& shader);

		/*!
		* \brief Frees up resources used. (Helps to free RenderUtils and DebugRender).
		*/
		void Free();

		/*!
		* \brief Returns the name of the texture to use for drawing background.
		* \return - A reference to a std::string.
		*/
		std::string& Background() { return background; }

		/*!
		* \brief Sets the colour to clear background with.
		* \param[in] r - The red component of RGBA, in normalised range [0.f - 1.f].
		* \param[in] g - The green component of RGBA, in normalised range [0.f - 1.f].
		* \param[in] b - The blue component of RGBA, in normalised range [0.f - 1.f].
		* \param[in] a - The alpha component of RGB, in normalised range [0.f - 1.f], defaulted to 1.f.
		*/
		inline void SetBackgroundColour(float r, float g, float b, float a = 1.f) {
			clear_colour[0] = r; clear_colour[1] = g; clear_colour[2] = b; clear_colour[3] = a;
		}

		/*!
		* \brief
		*	Set whether to draw background with texture or colour.
		*
		* \param[in] b
		*	- Pass true if to use texture for background drawing, false
		*	to just use colour.
		*/
		void UseTexAsBackground(bool flag) noexcept { useTexAsBackground = flag; }
 	private:
		/*!
		* \brief Updates the instance buffer with entities' transform.
		* \param[in, out] registry - The entity registry to get the entities' transform.
		*/
		void UpdateInstancedEntitiesTransform(Registry& registry);
		/*!
		* \brief
		*	Resize the instance buffer should the data exceed buffer size.
		*
		* \param[in] size
		*	- The incoming size, sets new buffer size as 2 * size if current
		*	buffer size is <= size.
		*/
		void ResizeInstanceBuffer(size_t size = 0);

		/*!
		* \brief Apply post process bloom onto color buffer.
		* \param[in] postProcess - The entities with post process components to get properties from.
		*/
		void Bloom(const std::vector<EntityRegistry::Entity>& postProcess);

		/*!
		* \brief Apply post process vignette onto color buffer.
		* \param[in] postProcess - The entities with post process components to get properties from.
		*/
		void Vignette(const std::vector<EntityRegistry::Entity>& postProcess);

		/*!
		* \brief Draw darkness and lights
		* \param[in, out] registry - The entity registry.
		*/
		void LightingPass(Registry& registry);

		/*!
		* \brief Used for view culling. Check whether is in camera's view using AABB since 2D.
		* \param[in] transform		- The entity's transform to check.
		* \param[in] camPos			- The camera's (world) position.
		* \param[in] camHalfExtent	- The camera's viewport half extent.
		*/
		bool IsInView(const Mat3& transform, const Vec2& camPos, const Vec2& camHalfExtent);
		/*!
		* \brief Used for view culling. Check whether is in camera's view using AABB since 2D.
		* \param[in] pos			- The entity's (world) position to check.
		* \param[in] scale			- The entity's scale.
		* \param[in] camPos			- The camera's (world) position.
		* \param[in] camHalfExtent	- The camera's viewport half extent.
		*/
		bool IsInView(const Vec2& pos, const Vec2& scale, const Vec2& camPos, const Vec2& camHalfExtent);
	public:
		Color playerColour{ 1.f, 1.f, 1.f, 1.f };	// Colour to render player in minimap
		Color enemyColour{ 1.f, 0.f, 0.f, 1.f };	// colour to render enemies in minimap

		Vec2 backgroundOffset{0.f, 0.f};			// sprite offset for background texture
		Vec2 backgroundSize{1.f, 1.f};				// sprite size for background texture
		bool instancingFlag{ true };				// the flag to toggle between instanced and non-instance rendering
		bool postProcessFlag{ true };				// the flag to toggle post processing
	private:
		std::string background{};					// the name of the texture to draw as background
		std::vector<BatchInfo> batchOffSets;		// holds batch info (texture, count) for instanced rendering
		std::vector<InstanceData> xforms;			// a containter to hold entities' xforms for instanced rendering

		float normalisedDepthDivisor{};
		float clear_colour[4]{ 0.0f, 0.0f, 0.0f, 0.0f };		// the colour to clear the buffer with, rgba normalize components in range 0.0f to 1.0f
		bool useTexAsBackground{ true };						// whether to draw background with texture or colour, true for texture
};