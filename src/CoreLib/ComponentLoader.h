/**___________________________________________________________________________/
@file          CollisionSystem.h
@author        weijie.ng@digipen.edu (wei jie) 70%
@author        kaedenjiawei.tan@digipen.edu ( kaeden) 30 %
@date          25/09/2025

This file is where we register all of the component into componentRegistry
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "Savestates.h"
#include "Components.h"

#define RegisterComponentMacro(type)\
 registerComp<type>(registry, componentRegistry, std::string(refl::reflect<type>().name)); \
 rtr::TypeInfo::Init<type>();


/*!
* \brief
*    Registers a component type with the ECS system, linking it to creation and
*    removal functions in the [ComponentRegistry].
*
* \tparam T
*    The component type to register.
*
* \param
*    [Registry&] registry - Reference to the ECS registry managing entities and components.
*
* \param
*    [ComponentRegistry&] componentRegistry - Registry that holds mappings from
*    component names to creation and removal functions.
*
* \param
*    [std::string] name - The string identifier for the component type.
*
* \details
*    Associates the given component type with two lambdas:
*    - A creation lambda that adds an empty instance of the component to an entity.
*    - A removal lambda that removes the component from an entity.
*    This enables dynamic component creation and removal through reflection or editor systems.
*/
template<typename T>
void registerComp(Registry& registry, ComponentRegistry& componentRegistry, std::string name)
{
	componentRegistry.RegisterComponent<T>(
		name,
		[&registry](EntityRegistry::Entity entity) {
			registry.AddComponent<T>(entity, {});
		}, 
		[&registry](EntityRegistry::Entity entity) {
			registry.RemoveComponent<T>(entity);
		}
	);
#ifdef PLATFORM_WINDOWS
	//CEO::Instance().GetManager<Savestate>()->AddComponentCast<ComponentStorage<T>>();
#endif
};

/*!
* \brief
*    Initializes and registers all default components used by the engine.
*
* \param
*    [Registry&] registry - Reference to the ECS registry managing entities and components.
*
* \param
*    [ComponentRegistry&] componentRegistry - Registry used to register all component types.
*
* \details
*    Calls the [RegisterComponentMacro] for each engine-defined component type,
*    making them available for creation, removal, and reflection-driven systems.
*    This ensures that the ECS can dynamically work with core components such as
*    transform, rendering, physics, and animation.
*/
void ComponentInitialise(Registry& registry, ComponentRegistry& componentRegistry)
{
	// Register components
	RegisterComponentMacro(NameComponent);
	RegisterComponentMacro(AnchorComponent);
	RegisterComponentMacro(UITransformComponent);
	RegisterComponentMacro(TransformComponent);
	RegisterComponentMacro(ActiveComponent);
	RegisterComponentMacro(SpriteRendererComponent);
	RegisterComponentMacro(CollisionComponent);
	RegisterComponentMacro(AnimatorComponent);
	RegisterComponentMacro(PhysicsComponent);
	RegisterComponentMacro(CameraComponent);
	RegisterComponentMacro(AudioComponent);
	RegisterComponentMacro(ButtonComponent);
	//CEO::Instance().GetManager<Savestate>()->AddComponentCast<ComponentStorage<NativeScriptingComponent>>();
	RegisterComponentMacro(EnemyComponent);
	RegisterComponentMacro(StateComponent);
	RegisterComponentMacro(NativeScriptingComponent);
	RegisterComponentMacro(TextRendererComponent);
	RegisterComponentMacro(VideoComponent);
	RegisterComponentMacro(LayerComponent);
	RegisterComponentMacro(OniAttackComponent);
	RegisterComponentMacro(UpdateStackComponent);
	RegisterComponentMacro(HierarchyComponnent);
	RegisterComponentMacro(PrefabDummyMetatag);
	RegisterComponentMacro(PrefabComponent);
	RegisterComponentMacro(ParticleEmitterComponent);
	RegisterComponentMacro(TilemapComponent);
	RegisterComponentMacro(TileCameraBoundsComponent);
	RegisterComponentMacro(LightComponent);
	RegisterComponentMacro(PostProcessComponent);
}
