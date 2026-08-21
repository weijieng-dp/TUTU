#pragma once

//---------------------------------------------------------
// @file Components.h
// 
// @brief contain all component for ECS
// 
//
// Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

//--------------------------------------------------------- 

#include <string>
#include "RuntimeReflect.h"
#include <type_traits>
#include <utility>
#include "MathLib.h"
#include "Texture.h"
#include <memory>
#include "Animation.h"
#include "StateInterface.h"
#include "CEO.h"
#include "ResourceManager.h"
#include "audio.h"
#include "Tileset.h"
#include "LayerManager.h"
#include "video.h"


struct NameComponent : rtr::Reflectable
{
    std::string name = "Entity";

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(NameComponent),
    field(name)
)
NOT_INSPECTABLE(NameComponent);

struct AnchorComponent : rtr::Reflectable
{
	std::string AnchoredTo = "Center";

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AnchorComponent),
    field(AnchoredTo)
)

struct TransformComponent : rtr::Reflectable
{
    Vec2 translate = Vec2{ 0.f, 0.f };
    float rotation = 0;
    Vec2 scale = Vec2(250.f, 250.f);
    Mat3 transform = Mat3();            // Model transform

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(TransformComponent),
    field(translate),
    field(rotation),
    field(scale)
)
NOT_INSPECTABLE(TransformComponent);

struct UITransformComponent : TransformComponent
{
    Vec2 min{ -0.5f,-0.5f }, max{ 0.5f,0.5f };  // normalized min max position relative to parent with range [-0.5, 0.5]
    Vec2 anchor{ 0.5f,0.5f };                 // normalized position relative to min max with range [0, 1]
    Vec2 relativePos = Vec2{ 0.f, 0.f };    // relative position to anchor in pixel units
    Vec2 relativePivot = Vec2{ 0.f, 0.f };  // normalized pivot position with range [-1, 1]
    Vec2 size = Vec2(250.f, 250.f);

    Mat3 xTransform = Mat3();               // transform to be inherited by children
    Mat3 viewportTransform = Mat3();        // transform in viewport coordinates
    Mat3 anchorTransform = Mat3();          // anchor transform, transform from local NDC to NDC relative to parent
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(UITransformComponent),
    field(min),
    field(max),
    field(anchor),
    field(relativePos),
    field(relativePivot),
    field(rotation),
    field(size)
)
NOT_INSPECTABLE(UITransformComponent);

//struct UITransformComponent : rtr::Reflectable
//{
//    Vec2 min{ -1.f,-1.f }, max{ 1.f,1.f };  // normalized min max position relative to parent with range [-1, 1]
//    Vec2 anchor{ 0.f,0.f };                 // normalized position relative to min max with range [-1, 1]
//    Vec2 relativePos = Vec2{ 0.f, 0.f };    // relative position to anchor in pixel units
//    Vec2 relativePivot = Vec2{ 0.f, 0.f };  // normalized pivot position with range [-1, 1]
//    float rotation = 0;
//    Vec2 size = Vec2(250.f, 250.f);
//
//    Mat3 viewportTransform = Mat3();        // transform in viewport coordinates
//    Mat3 anchorTransform = Mat3();          // anchor transform, transform from local NDC to NDC relative to parent
//    REFLECTABLE_PROPERTIES;
//};
//REFL_AUTO(
//    type(UITransformComponent),
//    field(min),
//    field(max),
//    field(anchor),
//    field(relativePos),
//    field(relativePivot),
//    field(rotation),
//    field(size)
//)

struct ActiveComponent : rtr::Reflectable
{
    bool isActiveSelf = true;     // user's own toggle
    bool isActiveInHierarchy = true; // final inherited active state

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ActiveComponent),
    field(isActiveSelf),
    field(isActiveInHierarchy)

)
NOT_INSPECTABLE(ActiveComponent);

struct SpriteRendererComponent : rtr::Reflectable
{
    TextureObj* texture{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };    
    TextureObj* emissiveTexture{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };
    Vec2 start{ 0.f, 0.f }; // start uv of tex coord
    Vec2 size{ 1.f, 1.f };  // size of portion of texture to be rendered tex coord
    Vec2 tile{ 1.f,1.f };
    Color color{1.f,1.f,1.f,1.f};

    bool visible{ true };
    bool isEmissive{ false };

    enum ShaderEmissiveState {
        EMISSIVE_DISABLED = 0,
        EMISSIVE_ENABLED = 1,
        EMISSIVE_TEXTURED = 2
    };
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(SpriteRendererComponent),
    field(texture),
    field(emissiveTexture),
    field(start),
    field(size),
    field(color),
    field(visible),
    field(isEmissive),
    field(tile)
)

enum class Shape {
    Box,
    Circle,
    Capsule
};

/*!
* \brief
*    The [CollisionComponent] struct defines collider shapes and properties
*    used for detecting and handling collisions between entities in the game world.
*    It supports box, circle, and capsule colliders, scaling, and trigger flags.
*
* \brief
*    Usage:
* \brief
*    - Attach a [CollisionComponent] to an entity to enable collision detection.
* \brief
*    - Set [shape] to [Shape::Box], [Shape::Circle], or [Shape::Capsule].
* \brief
*    - Adjust [offset], [baseHalfExtents], or [radius] to configure collider size.
* \brief
*    - Use [colliderScale] for per-collider scaling independent of visual scale.
* \brief
*    - Enable [isTrigger] to detect collisions without applying physics resolution.
* \brief
*    - At runtime, [worldMin] and [worldMax] are computed for collision checks.
*
* \return
*    [CollisionComponent] Stores collider configuration for collision detection and resolution.
*/
struct CollisionComponent : rtr::Reflectable
{
    Shape shape = Shape::Box;                   // Box, Circle, or Capsule
    Vec2 offset{ 0.f, 0.f };                    // Offset from entity center
	Vec2 baseHalfExtents{ 0.5f, 0.5f };         // Default unscaled size    *FOR CAPSULE, x = half-width (circle radius), y = half-height of rectangular section
    float radius = 0.5f;                        // For circle colliders
    bool isTrigger = false;                     // True = no physics resolution

    // Local scale to adjust collider independently of visual scale
    Vec2 colliderScale{ 1.f, 1.f };

    // Runtime computed values (for debugging & collision checks)
    Vec2 worldMin;
    Vec2 worldMax;

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(CollisionComponent),
    field(shape),
    field(offset),
    field(baseHalfExtents),
    field(radius),
    field(isTrigger),
    field(colliderScale)
)


struct AnimatorComponent : rtr::Reflectable
{
    float timeElapsed{0.f}; // time elapsed since first frame of animation
    int currFrame{ 0 }; // current frame index of animation
    float animSpeed = 1.f;
    Animation* currAnim{ &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("")};
    Animation* prevAnim{ nullptr };
    bool isPlaying{ true }; // whether animation is currently animating

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AnimatorComponent),
    field(isPlaying),
    field(timeElapsed),
    field(animSpeed),
    field(currAnim)
)

struct AudioComponent : rtr::Reflectable
{
    AudioObj* audio{ &CEO::Instance().GetManager<ResourceManager>()->GetAudio("") };
    FMOD::Channel* channel{ nullptr }; 
    std::string type{};
    float volume{1.f};
    float pitch{1.f};
    float fadeInTimer = 0.5f;
    float fadeOutTimer = 0.5f;
    float currentFadingTimer = 0.0f;
    int loopCount{}; // -1 means infinite loop, 0 means no looping, rest is just for how many loops

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AudioComponent),
    field(type),
    field(volume),
    field(pitch),
    field(loopCount),
    field(audio)
)


/*!
* \brief
*    The [PhysicsComponent] struct stores the physical properties of an entity,
*    including velocity, acceleration, mass, and flags for kinematics and gravity.
*    It is used by the [PhysicsSystem] to simulate motion and apply forces.
*
* \brief
*    Usage:
* \brief
*    - Attach a [PhysicsComponent] to an entity to enable physics simulation.
* \brief
*    - Set [velocity] and [acceleration] to control entity movement.
* \brief
*    - Use [mass] for calculations involving forces and collision responses.
* \brief
*    - Enable [isKinematic] to make the entity unaffected by physics forces but
*      still movable through manual updates.
* \brief
*    - Enable [useGravity] to automatically apply gravity each frame.
*
* \return
*    [PhysicsComponent] Holds data that drives physics simulation for an entity.
*/
struct PhysicsComponent : rtr::Reflectable
{
    Vec2 velocity{ 0.f, 0.f };      // Current movement velocity
    Vec2 acceleration{ 0.f, 0.f };  // Acceleration applied each frame
    Vec2 netForce{ 0.f, 0.f };      // Force accumulation (cleared every frame)
    float mass = 1.0f;              // Used for forces and collision response
    bool isKinematic = false;       // If true, ignores external physics forces
    bool useGravity = false;        // Enable/disable gravity for this entity

    REFLECTABLE_PROPERTIES;
};

// Reflection setup for runtime editor/inspection
REFL_AUTO(
    type(PhysicsComponent),
    field(velocity),
    field(acceleration),
    field(netForce),
    field(mass),
    field(isKinematic),
    field(useGravity)
)

struct EnemyComponent : rtr::Reflectable
{
    enum {
        EYEBALL, GIRL, ONI, GUY, LARVA
    };
    int type{EYEBALL};                  // what enemy. Int to help with serialization
    int hp{10};                         // should prob be in a seperate component
    int damage{};                       // how much damage the enemy does
    Vec2 targetPos{};                   // where the monster is pathfinding to
    float velocity{100.f};              // the overall speed (to normalize the physics component to)
    float attackRange{};                // like melee attack range, or ranged attack range both works
    float timeCount{-1.f}, maxTime{};   // for idle state
    unsigned flags{};                   // storing flags diff enemies might use      
    bool lockFacing{ false };           // Used if you want to lock the direction the enemy is facing.

    union {
        struct OniStruct {
            Registry::Entity walkID{};
            Registry::Entity attID{};
            Registry::Entity hit1ID{};
            Registry::Entity hit2ID{};
        } oniStruct;
        struct EyeStruct {
            Registry::Entity normID{};
            Registry::Entity hitID{};
            float distance{};
            Vec2 attVecNorm{};
        } eyeStruct;
        struct GirlStruct {
            float acceleration{};
            float ratio{};
            bool reachTarget{};
        } girlStruct;
        struct GuyStruct {
            void* projectileManager{};
            float attTimer{};
            bool idleUnmoving{};
            bool shotBullet{};
        } guyStruct;
        struct LarvaStruct {
            void* projectileManager{};
            float attTimer{};
            float attCooldown{};
        } larvaStruct;
    } enemyStruct{};

    REFLECTABLE_PROPERTIES;
};

REFL_AUTO(
    type(EnemyComponent),
    field(type),
    field(hp),
    field(damage),
    field(targetPos),
    field(velocity),
    field(attackRange),
    field(timeCount),
    field(maxTime),
    field(flags)
)

struct StateComponent : rtr::Reflectable
{
    std::vector<std::shared_ptr<IStateMachine>> stateMachines;
    REFLECTABLE_PROPERTIES;
};

REFL_AUTO(
    type(StateComponent)
)

/*!
* \brief
*    The [CameraComponent] struct stores the viewing and projection properties of a camera,
*    including viewport dimensions, view/projection matrices, and layer culling settings.
*    It is used by the rendering system to define camera transformations and determine
*    what content is visible and rendered to the screen.
*
* \brief
*    Usage:
* \brief
*    - Attach a [CameraComponent] to an entity to enable camera rendering.
* \brief
*    - Set [isMainCam] to true to designate this as the primary rendering camera.
* \brief
*    - Configure [viewportSize] to define the camera's viewing area dimensions.
* \brief
*    - Set [fixedAspectRatio] to true to maintain a fixed aspect ratio independent
*      of the main camera, or false to follow the main camera's aspect ratio.
* \brief
*    - Use [cullingMask] to determine which entity layers are rendered by this camera.
* \brief
*    - At runtime, [view], [proj], and [vp] matrices are computed for rendering transformations.
*
* \return
*    [CameraComponent] Holds camera configuration and transformation matrices for view and projection.
*/
struct CameraComponent : rtr::Reflectable
{
    bool isMainCam = false;
    bool fixedAspectRatio{ false };     // whether aspect ratio is fixed or follows main camera
    Vec2 viewportSize{1200.f, 540.f};
    Mat3 view = {};                     // view matrix of camera
    Mat3 proj = {};                     // projection matrix of camera
    Mat3 vp = {};                       // proj * view matrix of camrea

    uint64_t cullingMask{~0u};          // to check what layers to render

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(CameraComponent),
    field(isMainCam),
    field(fixedAspectRatio),
    field(viewportSize),
    field(view),
    field(cullingMask)
)

struct LightComponent : rtr::Reflectable
{
    enum LightType{POINT, GLOBAL};
    LightType type{ POINT };
    TextureObj* lightShapeTex{ &CEO::Instance().GetManager<ResourceManager>()->GetTexture("radialFade.png") };
    Color color{1,1,1,1};
    float intensity{ 1.f };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(LightComponent),
    field(type),
    field(lightShapeTex),
    field(color),
    field(intensity)
)

struct PostProcessComponent : rtr::Reflectable
{
    float bloomThreshold{ 0.9f };
    float bloomExposure{ 1.0f };
    bool  isUIVignette{ false };
    float vignetteIntensity{ 0.f };
    float vignetteFalloff{ 0.f };
    //float gamma{ 2.2f };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(PostProcessComponent),
    field(bloomThreshold),
    field(bloomExposure),
    field(isUIVignette),
    field(vignetteIntensity),
    field(vignetteFalloff)
)

struct ParticleEmitterComponent : rtr::Reflectable {
    // texture to apply on particle.ErrorTex means don't use texture
    TextureObj* texture{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };

    Color startColor{ 1.f, 1.f, 1.f, 1.f };     // colour to start particle on
    Color endColor{ 0.5f, 0.5f, 0.5f, 1.f };    // colour to end particle on

    Vec2 emitterPosition{ 0.f, 0.f };   // the position of the particle emitter (relative to transform component)
    Vec2 halfExtents{ 0.f, 0.f };       // the square of the of where the particles can spawn (origin being emitter position)

    Vec2 startScale{ 1.f, 1.f };        // starting scale for particle
    Vec2 endScale{ 0.f, 0.f };          // ending scale for particle 
    Vec2 rotationRange{ 0.f, 0.f };     // the min and max range of rotation

    // ===== Particle Lifetime =====
    Vec2 particleLifetimeRange{ 5.f, 10.f }; // the range to randomise the particle life time [min, max]

    // ==== Emission ====
    Vec2 emissionRateRange{ 0.f, 10.f };    // particle emission rate range
    uint32_t maxParticles{ 100 };           // max of particles that can exist at a time
    uint32_t aliveParticles{ 0 };           // number of particles current alive

    // =========
    Shape particleSpawnAreaShape{ Shape::Box }; // what is the shape of the spawn area
    float radius{ 0.f };                // if emitter shape is a circle, use radius instead of half extents

    // ==== Motion ====
    float direction{ 0.f };             // the direction to spawn particle in, measured in degrees. Starts from right (0 == right, 90 == top)
    float spread{ 360.f };              // the spread of the particle spawn, in degrees, starting from direction. >= 360 means radial spread
    float minSpeed{ 0.f };              // minimum speed value of particle
    float maxSpeed{ 1.f };              // maximum speed value of particle
    Vec2 acceleration{ 0,0 };

    // ==== Emitter Duration =======
    float duration{ 10.f };              // duration of emitter (ignored if looping == true)

    // ==== For internal calculations ====
    float timeElapsed{ 0.f };       // for internal tracking of emission rate
    float spawnAccumulator{ 0.f };  // 

    // ==== Flags ====
    bool enabled{ true };           // toggle for turn on/off the emitter
    bool isEmmissive{ false };      // whether particles have emission
    bool looping{ false };          // whether to loop
    bool useLocalPosition{ true };  // whether emitterPosition is in Local Position or Global Position

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ParticleEmitterComponent),
    field(texture),
    field(startColor),
    field(endColor),
    field(emitterPosition),
    field(halfExtents),
    field(startScale),
    field(endScale),
    field(rotationRange),
    field(particleLifetimeRange),
    field(emissionRateRange),
    field(maxParticles),
    field(particleSpawnAreaShape),
    field(radius),
    field(direction),
    field(spread),
    field(minSpeed),
    field(maxSpeed),
    field(acceleration),
    field(duration),
    field(timeElapsed),
    field(spawnAccumulator),
    field(enabled),
    field(isEmmissive),
    field(looping),
    field(useLocalPosition)
)

struct ScriptInstance : rtr::Reflectable
{
 EntityRegistry::Entity entity = 0;
    template <typename T>
    T* GetComponent(Registry& registry)
    {
        return registry.GetComponent<T>(entity);
    }

    template <typename T>
    T& TryGetComponent(Registry& registry)
    {
        return registry.TryGetComponent<T>(entity);

    }

};
REFL_AUTO(
    type(ScriptInstance)
)

/*!
* \brief
*	Wrapper that provides GetComponent<T>() and GetEntity() so scripts can access 
*   the other collider’s components without directly using raw IDs.
*/
struct Collider
{
    Registry* registry = nullptr;
    EntityRegistry::Entity entity = 0;
    EntityRegistry::Entity ownerEntity = 0;

    template <typename T>
    T* GetComponent() const
    {
        return registry ? registry->GetComponent<T>(entity) : nullptr;
    }

    EntityRegistry::Entity GetEntity() const
    {
        return entity;
    }

    EntityRegistry::Entity GetOwnerEntity() const
    {
        return ownerEntity;
    }
};


/*!
* \brief
*	Component that stores and manages native script instances bound to an entity.
*	Handles creation, deletion, and lifecycle callbacks for each script through
*	function pointers and reflection metadata.
*/
struct NativeScriptingComponent : rtr::Reflectable {

    /*!
* \brief
*	Encapsulates all runtime information and function bindings for a single
*	native script instance, including creation, update, and fixed update
*	function pointers.
*/

    enum ScriptState
    {
        Normal,
        Faulted
    };
    struct ScriptEntry {
        ScriptInstance* Instance = nullptr;

        using FnCreate = void(*)(ScriptEntry&);
        using FnDelete = void(*)(ScriptEntry&);
        using FnStart = void(*)(ScriptInstance*, Registry&);
        using FnUpdate = void(*)(ScriptInstance*, Registry&, float, bool);
        using FnBindFR = void(*)(ScriptInstance*);
        using FnBindTO = void(*)(ScriptInstance*);
        using FnFixedUpdate = void(*)(ScriptInstance*, Registry&, float, bool);
        using FnCollisionCallback = void(*)(ScriptInstance*, const Collider&);

        FnCreate      OnCreateScript = nullptr;
        FnDelete      OnDeleteScript = nullptr;
        FnStart       OnStartFunction = nullptr;
        FnUpdate      OnUpdateFunction = nullptr;
        FnBindFR      BindFrom = nullptr;
        FnBindTO      BindTo = nullptr;
        FnFixedUpdate OnFixedUpdateFunction = nullptr;
        FnCollisionCallback OnCollisionEnterFunction = nullptr;
        FnCollisionCallback OnCollisionStayFunction = nullptr;
        FnCollisionCallback OnCollisionExitFunction = nullptr;
        FnCollisionCallback OnTriggerEnterFunction = nullptr;
        FnCollisionCallback OnTriggerStayFunction = nullptr;
        FnCollisionCallback OnTriggerExitFunction = nullptr;

        bool isStarted = false;
        ScriptState state = ScriptState::Normal;
    };


    std::map<std::string, ScriptEntry> scripts;
    std::unordered_map<std::string, bool> firstframescripts;


    /*!
    * \brief
    *	Generic creation implementation for a script type T. Allocates a new
    *	instance of the script.
    * \param
    *	e - reference to the ScriptEntry where the instance is stored
    */
    template<typename T>
    static void CreateImpl(ScriptEntry& e) {
        e.Instance = new T();
        
    }

    /*!
* \brief
*	Generic deletion implementation for a script type T. Frees memory and
*	resets instance data.
* \param
*	e - reference to the ScriptEntry whose instance will be deleted
*/
    template<typename T>
    static void DeleteImpl(ScriptEntry& e) {
        if (e.Instance) {
            delete static_cast<T*>(e.Instance);
            e.Instance = nullptr;
            e.isStarted = false;
        }
    }


    /*!
* \brief
*	Calls the OnStart function of a script type T.
* \param
*	i - pointer to the active script instance
* \param
*	r - reference to the ECS registry
*/
    template<typename T>
    static void StartImpl(ScriptInstance* i, Registry& r) {
        static_cast<T*>(i)->OnStart(r);
    }

    /*!
* \brief
*	Calls the OnUpdate function of a script type T each frame.
* \param
*	i - pointer to the active script instance
* \param
*	r - reference to the ECS registry
* \param
*	dt - delta time between frames
* \param
*	firstframe - true if this is the first update after initialization
*/
    template<typename T>
    static void UpdateImpl(ScriptInstance* i, Registry& r, float dt, bool firstframe) {
        static_cast<T*>(i)->OnUpdate(r, dt,  firstframe);
    }


    /*!
    * \brief
    *	Calls the OnFixedUpdate function of a script type T at fixed timestep
    *	intervals.
    * \param
    *	i - pointer to the active script instance
    * \param
    *	r - reference to the ECS registry
    * \param
    *	dt - fixed timestep delta
    * \param
    *	firstframe - true if this is the first fixed update after initialization
    */
    template<typename T>
    static void FixedUpdateImpl(ScriptInstance* i, Registry& r, float dt,bool firstframe) {
        static_cast<T*>(i)->OnFixedUpdate(r, dt, firstframe);
    }

    template <typename T, typename = void>
    struct HasOnCollisionEnter : std::false_type {};
    template <typename T>
    struct HasOnCollisionEnter<T, std::void_t<decltype(std::declval<T&>().OnCollisionEnter(
        std::declval<Collider>()))>> : std::true_type {};

    template <typename T, typename = void>
    struct HasOnCollisionStay : std::false_type {};
    template <typename T>
    struct HasOnCollisionStay<T, std::void_t<decltype(std::declval<T&>().OnCollisionStay(
        std::declval<Collider>()))>> : std::true_type {};

    template <typename T, typename = void>
    struct HasOnCollisionExit : std::false_type {};
    template <typename T>
    struct HasOnCollisionExit<T, std::void_t<decltype(std::declval<T&>().OnCollisionExit(
        std::declval<Collider>()))>> : std::true_type {};

    template <typename T, typename = void>
    struct HasOnTriggerEnter : std::false_type {};
    template <typename T>
    struct HasOnTriggerEnter<T, std::void_t<decltype(std::declval<T&>().OnTriggerEnter(
        std::declval<Collider>()))>> : std::true_type {};

    template <typename T, typename = void>
    struct HasOnTriggerStay : std::false_type {};
    template <typename T>
    struct HasOnTriggerStay<T, std::void_t<decltype(std::declval<T&>().OnTriggerStay(
        std::declval<Collider>()))>> : std::true_type {};

    template <typename T, typename = void>
    struct HasOnTriggerExit : std::false_type {};
    template <typename T>
    struct HasOnTriggerExit<T, std::void_t<decltype(std::declval<T&>().OnTriggerExit(
        std::declval<Collider>()))>> : std::true_type {};

    template<typename T>
    static void CollisionEnterImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnCollisionEnter(other);
        static_cast<T*>(i)->BindTo();
    }

    template<typename T>
    static void CollisionStayImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnCollisionStay(other);
        static_cast<T*>(i)->BindTo();
    }

    template<typename T>
    static void CollisionExitImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnCollisionExit(other);
        static_cast<T*>(i)->BindTo();
    }

    template<typename T>
    static void TriggerEnterImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnTriggerEnter(other);
        static_cast<T*>(i)->BindTo();
    }

    template<typename T>
    static void TriggerStayImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnTriggerStay(other);
        static_cast<T*>(i)->BindTo();
    }

    template<typename T>
    static void TriggerExitImpl(ScriptInstance* i, const Collider& other) {
        static_cast<T*>(i)->BindFrom();
        static_cast<T*>(i)->OnTriggerExit(other);
        static_cast<T*>(i)->BindTo();
    }


    template<typename T>
    static void BindFromImpl(ScriptInstance* i) {
        static_cast<T*>(i)->BindFrom();
    }

    template<typename T>
    static void BindToImpl(ScriptInstance* i) {
        static_cast<T*>(i)->BindTo();
    }
    /*!
* \brief
*	Binds all function pointers for a given script type T to its ScriptEntry,
*	enabling runtime reflection and lifecycle management.
* \param
*	name - name of the script type to bind
*/
    template<typename T>
    void Bind(std::string const& name) {


        auto& entry = scripts[name];
        entry.isStarted = false;
        entry.OnCreateScript = &CreateImpl<T>;
        entry.OnDeleteScript = &DeleteImpl<T>;
        entry.OnStartFunction = &StartImpl<T>;
        entry.OnUpdateFunction = &UpdateImpl<T>;
        entry.OnFixedUpdateFunction = &FixedUpdateImpl<T>;
        entry.BindFrom = &BindFromImpl<T>;
        entry.BindTo = &BindToImpl<T>;
        if constexpr (HasOnCollisionEnter<T>::value) {
            entry.OnCollisionEnterFunction = &CollisionEnterImpl<T>;
        }
        if constexpr (HasOnCollisionStay<T>::value) {
            entry.OnCollisionStayFunction = &CollisionStayImpl<T>;
        }
        if constexpr (HasOnCollisionExit<T>::value) {
            entry.OnCollisionExitFunction = &CollisionExitImpl<T>;
        }
        if constexpr (HasOnTriggerEnter<T>::value) {
            entry.OnTriggerEnterFunction = &TriggerEnterImpl<T>;
        }
        if constexpr (HasOnTriggerStay<T>::value) {
            entry.OnTriggerStayFunction = &TriggerStayImpl<T>;
        }
        if constexpr (HasOnTriggerExit<T>::value) {
            entry.OnTriggerExitFunction = &TriggerExitImpl<T>;
        }
        entry.state = NativeScriptingComponent::ScriptState::Normal;

        firstframescripts[name] = true;
    }


    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(NativeScriptingComponent)
)
DO_NOT_SERIALIZE(NativeScriptingComponent);
NOT_INSPECTABLE(NativeScriptingComponent);

struct ButtonComponent : rtr::Reflectable
{
    enum ButtonState {
        IDLE,
        HOVERED,
        PRESSED,
        RELEASED
    };
    ButtonState state = IDLE;
    std::function<void()> onPress;
    std::function<void()> onClick;
    std::function<void()> onHeld;
    std::function<void()> onHoverEnter;
    std::function<void()> onHoverExit;
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ButtonComponent),
    field(state)
)

struct TextRendererComponent : rtr::Reflectable
{
    enum Alignment {
        LEFT,
        CENTER,
        RIGHT
    };
    std::string text;
    Alignment alignment = CENTER; // text alignment, use type enum Alignment
    Color color{1.f,1.f,1.f,1.f};
    int fontSize = 16;
    float linePadding = 0.f;
    FontObj* font{ &CEO::Instance().GetManager<ResourceManager>()->GetFont("Yjfontv2-Regular.otf") };
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(TextRendererComponent),
    field(text),
    field(alignment),
    field(color),
    field(fontSize),
    field(linePadding),
    field(font)
)
/*,
    field(font)*/ // handle this in editor later

/*!
* \brief
*	Stores information regarding layers.
*   Usage:
* 
*   - Set [renderPriority] for the rendering order. (Higher number will be drawn over by entities with lower number).
*   - Set [layer] for the layer the entity is residing in. Used for adjusting collision interactions between entities
*   and whether entity will be rendered by main camera.
*/
struct LayerComponent : rtr::Reflectable
{
    unsigned renderPriority{ 1 };    // the depth value, lower will be further back, higher will be rendered on top.
    unsigned layer{ 0 };             // the layer the entity is/are in

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(LayerComponent),
    field(renderPriority),
    field(layer)
)

// scuffed aaah shit
struct OniAttackComponent : rtr::Reflectable
{
    int AttackType{};
    bool linked{ false };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(OniAttackComponent),
    field(AttackType)
)

struct UpdateStackComponent : rtr::Reflectable
{
    int stack{0};
    UpdateStackComponent(int i = 0) : stack{ i } {}
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(UpdateStackComponent),
    field(stack)
)
NOT_INSPECTABLE(UpdateStackComponent);

struct ProjectileComponent : rtr::Reflectable
{
    float velocity{ 250.f };
    int damage{1};
    float lifespan{ 15.f };                // how long projectile should stay alive
    float timeAlive{};

    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(ProjectileComponent),
    field(velocity),
    field(damage),
    field(lifespan),
    field(timeAlive)
)

// temporarily put here for testing
#include "TilesetManager.h"
struct TilemapComponent : rtr::Reflectable
{
    enum TilemapProperties
    {
        CHUNK_SIZE = 16
    };
    
    using TilemapChunk = std::array<Tile, CHUNK_SIZE* CHUNK_SIZE>;
    using ChunkMap = std::map<ChunkCoordinates, TilemapChunk>;
    ChunkMap chunks;
    //int tileSize = 16;
    std::shared_ptr<Tileset> tileset{};

    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(TilemapComponent),
    field(chunks),
    field(tileset)

)


/*!
* \brief
*    The [TileCameraBoundsComponent] struct defines camera boundary constraints within a tilemap,
*    allowing restricted camera movement to specific regions and supporting blocked areas.
*    It is used by the camera system to prevent camera overflow and manage camera positioning
*    relative to tilemap boundaries.
*
* \brief
*    Usage:
* \brief
*    - Attach a [TileCameraBoundsComponent] to define the camera's movement boundaries.
* \brief
*    - Set [min] and [max] to define the allowed camera bounds in world coordinates.
* \brief
*    - Enable [hasBlockedSquare] to activate a blocked region within the camera bounds.
* \brief
*    - Configure [blockedMin] and [blockedMax] to define the restricted area that the camera
*      cannot enter when blocking is enabled.
* \brief
*    - At runtime, the camera system uses these bounds to constrain camera movement and positioning.
*
* \return
*    [TileCameraBoundsComponent] Holds boundary constraints and blocking configuration for camera movement.
*/
struct TileCameraBoundsComponent : rtr::Reflectable
{
    Vec2 min{};
    Vec2 max{};

    bool hasBlockedSquare{ false };
    Vec2 blockedMin{};
    Vec2 blockedMax{};

    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(TileCameraBoundsComponent),
    field(min),
    field(max),
    field(hasBlockedSquare),
    field(blockedMin),
    field(blockedMax)
)



struct HierarchyComponnent : rtr::Reflectable
{
    EntityRegistry::Entity parent{};
    EntityRegistry::Entity firstChild{};
    EntityRegistry::Entity nextSibling{};

    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(HierarchyComponnent),
    field(parent),
    field(firstChild),
    field(nextSibling)
)
NOT_INSPECTABLE(HierarchyComponnent);

struct PrefabDummyMetatag : rtr::Reflectable
{
    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(PrefabDummyMetatag)
)
DO_NOT_SERIALIZE(PrefabDummyMetatag);
NOT_INSPECTABLE(PrefabDummyMetatag);

struct PrefabComponent : rtr::Reflectable
{
    std::string name{};

    bool root{};
	std::unordered_map<std::string, rtr::TypeInfo::MemberFlag> overriddenComponents{};
    REFLECTABLE_PROPERTIES
};
REFL_AUTO(
    type(PrefabComponent)
)
NOT_INSPECTABLE(PrefabComponent);
DO_NOT_SERIALIZE(PrefabComponent);


/*!
* \brief
*    The [VideoComponent] struct manages video playback and cutscene orchestration within the game,
*    including video resource binding, playback settings, and scene transition logic.
*    It is used by the video system to render videos, synchronize audio, handle user input,
*    and manage transitions between scenes based on video completion or user skip actions.
*
* \brief
*    Usage:
* \brief
*    - Attach a [VideoComponent] to an entity to enable video playback.
* \brief
*    - Set [videoPath] to the path of the video file to be loaded and played.
* \brief
*    - Enable [autoplay] to start video playback immediately upon scene load.
* \brief
*    - Enable [loop] to repeat the video indefinitely, or set [nextSceneOnEnd] for scene transitions.
* \brief
*    - Configure [playbackSpeed] to control video playback speed (1.0 = normal speed).
* \brief
*    - Set [allowSkip] to allow user skip functionality, and configure [skipKey] for the input key.
* \brief
*    - Use [nextSceneOnEnd] to specify the scene to load when video ends naturally.
* \brief
*    - Use [nextSceneOnSkip] to specify the scene to load when user skips the video.
* \brief
*    - Configure [audioPath], [audioType], [audioVolume], and [audioPitch] to synchronize external audio.
* \brief
*    - At runtime, [endedThisFrame], [isEnded], and [hasAutoStarted] track video state.
* \brief
*    - Runtime audio state is managed through [audioChannel] and [isAudioStarted].
*
* \return
*    [VideoComponent] Holds video playback configuration, cutscene settings, and runtime state for orchestrated video experiences.
*/
struct VideoComponent : rtr::Reflectable
{
    // Runtime handle to manager-owned video resource.
    VideoObj* video{ nullptr };

    // Authoring/runtime playback settings.
    std::string videoPath{};
    bool autoplay{ false };
    bool loop{ false };
    float playbackSpeed{ 1.f };

    // Runtime-only guard for one-time autoplay kickoff.
    bool hasAutoStarted{ false };

    // Runtime playback events/state for scripts and gameplay systems.
    bool endedThisFrame{ false };
    bool isEnded{ false };

    // Cutscene policy settings (component-driven orchestration).
    bool allowSkip{ true };
    int skipKey{ 32 }; // default SPACE key
    std::string nextSceneOnEnd{};
    std::string nextSceneOnSkip{};

    // External audio sync path 
    std::string audioPath{};
    std::string audioType{ "BGM" };
    bool audioLoop{ false };
    float audioVolume{ 1.f };
    float audioPitch{ 1.f };

    // Runtime audio state.
    FMOD::Channel* audioChannel{ nullptr };
    bool isAudioStarted{ false };

    // Runtime bookkeeping for robustness across scene reloads/resource resets.
    std::string boundVideoPath{};

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(VideoComponent),
    field(videoPath),
    field(autoplay),
    field(loop),
    field(playbackSpeed),
    field(allowSkip),
    field(skipKey),
    field(nextSceneOnEnd),
    field(nextSceneOnSkip),
    field(audioPath),
    field(audioType),
    field(audioLoop),
    field(audioVolume),
    field(audioPitch)
)
