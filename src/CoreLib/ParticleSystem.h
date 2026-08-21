/*!
@file       ParticleSystem.h
@author     Tan Jun Jie (t.junjie) 100%
@date       07/01/2026 (DD/MM/YYYY)
@brief		Handles the creation, updating, and rendering of particles.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include "MathLib.h"
#include "RenderUtils.h"
#include "glslshader.h"
#include "Components.h"
#include <vector>
#include <random>

struct Particle {
	Particle() : transform(), color{ 1.f, 1.f, 1.f, 1.f }, colorDelta{ 0.f, 0.f, 0.f, 0.0f },
		position(0.f, 0.f), scale(1.f, 1.f), scaleDelta(0.f, 0.f), velocity(0.f, 0.f),
		emitterId(0), rotation(0.f), lifetime(1.f), timeElapsed(0.f), isEmissive(false) { }

	Particle(const Color& startCol, const Color& endCol, const Vec2& pos,
		const Vec2& _startScale, const Vec2& _endScale, const Vec2& vel, uint32_t id, float rot, float lifespan, bool isEmissive);

	/*!
	* \brief
	*	Updates the particle. Increments timeElapsed by dt, interpolates
	*	scale & color with LERP.
	* 
	* \param[in] dt
	*	- The delta time to update the particle with.
	*/
	void Update(float dt);
	
	Mat3 transform;							// calculated transform for this particle

	Color color{ 1.f, 1.f, 1.f, 1.f };		// The current colour of the particle
	Color colorDelta{ 0.f, 0.f, 0.f, 0.f };	// The colour delta for changing the colour of the particle

	Vec2 position{ 0.f, 0.f };				// The position of the particle (relative to the entity/game obj the emitter is attached to)

	Vec2 scale{ 1.f, 1.f };					// The current scale of the particle
	Vec2 scaleDelta{ 0.f, 0.f };			// The scale delta for changing the colour of the particle
	
	Vec2 velocity{ 0.f, 0.f };				// The velocity of the particle

	uint32_t emitterId{ 0 };				// which emitter this particle belongs to, associate by entity Id

	float rotation{ 0.f };					// the rotation of the particle
	float lifetime{ 1.f };					// how long the particle will live for
	float timeElapsed{ 0.f };				// how long the particle has been alive for

	bool  isEmissive{ false };
};

class ParticleSystem {
public:
	static ParticleSystem& Instance();

	static constexpr float PARTICLE_SCALE_MULTIPLIER{ 25.f };	// base scale multiplier for particle

	/*!
	* \brief Initializes the buffers for ParticleSystem.
	*/
	void Init();

	/*!
	* \brief
	*	Updates the ParticleSystem. Checks through all particle emitters,
	*	creates new particles (if needed), updates existing particles, 
	*	and removes particles that exceeds their lifetime.
	* 
	*	After updating, uploads data to GPU.
	*
	* \param[in, out] registry
	*	- The entity registry to get the ParticleEmitterComponent.
	* 
	* \param[in] dt
	*	- The delta time to update the ParticleSystem with.
	*/
	void Update(Registry& registry, float dt);

	/*!
	* \brief
	*	Render the particles using instancing.
	*
	* \param[in, out] shdr
	*	- The shader to use for rendering the particles.
	*/
	void Draw(GLSLShader& shdr) const;

	/*!
	* \brief
	*	Helper function to enable a particle emitter.
	*
	* \param[in] emitterId
	*	- The entity ID for the particle emitter component.
	*/
	void EnableEmitter(Registry::Entity emitterId);

	/*!
	* \brief
	*	Helper function to disable a particle emitter. Resets
	*	internal timer & accumulator. 
	*
	*	WILL NOT REMOVE ALREADY CREATED PARTICLES FROM THIS EMITTER.
	*	Disables the emitter to stop spawning.
	*
	* \param[in] emitterId
	*	- The entity ID for the particle emitter component.
	*/
	void DisableEmitter(Registry::Entity emitterId);

	/*!
	* \brief
	*	Helper function to clear/remove all particles.
	*/
	void ClearAllParticles();

	/*!
	* \brief
	*	Helper function to clear/remove particles that has this texture (remove a batch).
	*/
	void ClearParticles(TextureObj* tex);
private:
	/*!
	* \brief
	*	Handles updating the particle information on CPU side.
	*
	* \param[in, out] registry
	*	- The entity registry to get the ParticleEmitterComponent.
	* \param[in] dt
	*	- The delta time to update the ParticleSystem with.
	*/
	void UpdateParticles(Registry& registry, float dt);

	/*!
	* \brief
	*	Handles updating the particle on GPU side (for instancing).
	* \param[in, out] registry - The entity registry.
	*/
	void UpdateInstance(Registry& registry);

	/*!
	* \brief
	*	Resize the particle instance buffer to be able to contain
	*	given size
	* 
	* \param[in] tex
	*	- The texture this particle instance buffer batch. ERROR TEX is
	*	used for no texture.
	*
	* \param[in] size
	*	- The size of particles that the instance buffer needs to contain
	*/
	void ResizeParticleBuffer(TextureObj* tex, size_t size);

	/*!
	* \brief
	*	Helper function to calculate a random floating-point number between
	*	min and max (inclusive).
	* \param[in] min - Min value to randomise between (inclusive).
	* \param[in] max - Max value to randomise between (inclusive).
	* \return A randomised floating-point number between min and max.
	*/
	float RandomFloat(float min, float max);

	/*!
	* \brief
	*	Helper function to calculate a random Vec2 between
	*	min and max (inclusive).
	* \param[in] min - Min value to randomise between (inclusive).
	* \param[in] max - Max value to randomise between (inclusive).
	* \return A randomised Vec2 between min and max.
	*/
	Vec2 RandomVec2(const Vec2& min, const Vec2& max);
private:
	// CPU-side container of alive particles. Key: [texture], Value: [CPU container for particles, number of particles alive]
	std::unordered_map <TextureObj*, std::pair<std::vector<Particle>, size_t>> particles;
	// GPU-side container of particles (for instancing).  Key: [texture], Value: [Buffer containing instance data for particles to upload to GPU]
	std::unordered_map<TextureObj*, std::vector<InstanceData>> particleInstance;
	std::unordered_map<TextureObj*, size_t> particleLimit;		// the size of the GPU-side buffer (particleInstance), mapped to texture

	std::mt19937 rng{ std::random_device{}() };

	bool initialized{ false };						// whether the ParticleSystem is initialized
};