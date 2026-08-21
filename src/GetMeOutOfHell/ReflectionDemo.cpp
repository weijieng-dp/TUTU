/*!
@file       Scene.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       01/10/2025
@brief		demo for reflection


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/


// test_reflection.cpp
#include "RuntimeReflect.h"
#include <iostream>
#include <cassert>
#include <string>
namespace {
    // Class/component to be reflected
    struct Stats : rtr::Reflectable {
        int strength{};
        int agility{};
        float stamina{};

        REFLECTABLE_PROPERTIES;
    };
    // Actor with nested Stats
    struct Actor : rtr::Reflectable {
        int id{};
        std::string name{};
        float health{};
        Stats stats{};

        REFLECTABLE_PROPERTIES;
    };
    // Top-level class with nested Actor
    struct Scene : rtr::Reflectable {
        std::string title{};
        Actor main_actor{};

        REFLECTABLE_PROPERTIES;
    };

    void print_stats(const Stats& stats) {
        std::cout << "Stats { strength: " << stats.strength
            << ", agility: " << stats.agility
            << ", stamina: " << stats.stamina << " } // print_stats\n";
    }

    void print_actor(const Actor& actor) {
        std::cout << "Actor { id: " << actor.id
            << ", name: " << actor.name
            << ", health: " << actor.health << ", stats: ";
        print_stats(actor.stats);
        std::cout << "} // print_actor\n";
    }

    void print_scene(const Scene& scene) {
        std::cout << "Scene { title: " << scene.title << ", main_actor: ";
        print_actor(scene.main_actor);
        std::cout << "} // print_scene\n";
    }

}

REFL_AUTO(
    type(Stats),
	field(strength),
	field(agility),
    field(stamina)
)

REFL_TYPE(Actor)
    REFL_FIELD(id)
    REFL_FIELD(name)
    REFL_FIELD(health)
    REFL_FIELD(stats)
REFL_END

REFL_AUTO(
    type(Scene),
	field(title),
    field(main_actor)
)

namespace rtr {
    int demo() {
        Stats stats;
        stats.strength = 10;
        stats.agility = 15;
        stats.stamina = 99.9f;

        Actor actor;
        actor.id = 42;
        actor.name = "Alice";
        actor.health = 99.5f;
        actor.stats = stats;

        Scene scene;
        scene.title = "Battle";
        scene.main_actor = actor;

        std::cout << "Initial objects: // main:1\n";
        print_scene(scene);

        // Test TypeInfo for Scene
        const rtr::TypeInfo& scene_ti = scene.GetTypeInfo();
        std::cout << "\nScene type name: " << scene_ti.Name() << " // main:2" << std::endl;
        auto scene_members = scene_ti.Members();
        std::cout << "Scene members: // main:3 ";
        for (const auto& m : scene_members) std::cout << m << " ";
        std::cout << std::endl;
        assert(scene_members.size() == 2);
        assert(scene_members[0] == "title");
        assert(scene_members[1] == "main_actor");

        // Test Instance for Scene
        rtr::Instance scene_inst = scene.GetInstance();
        std::string title_val = scene_inst.GetVal<std::string>("title");
        std::cout << "Scene title via reflection: " << title_val << " // main:4" << std::endl;
        assert(title_val == "Battle");

        // Access nested Actor via reflection (no pointer)
        Actor main_actor_val = scene_inst.GetVal<Actor>("main_actor");
        std::cout << "Main actor via reflection: // main:5\n";
        print_actor(main_actor_val);
        assert(main_actor_val.name == "Alice");
        assert(main_actor_val.health == 99.5f);

        // Access nested Stats via Actor reflection (no pointer)
        rtr::Instance actor_inst = main_actor_val.GetInstance();
        Stats stats_val = actor_inst.GetVal<Stats>("stats");
        std::cout << "Stats via reflection: // main:6\n";
        print_stats(stats_val);
        assert(stats_val.strength == 10);
        assert(stats_val.agility == 15);
        assert(stats_val.stamina == 99.9f);

        // Modify nested values via reflection
        std::cout << "\nModifying health and stats via reflection... // main:7\n";
        actor_inst.SetVal<float>("health", 88.8f);
        Stats new_stats;
        new_stats.strength = 20;
        new_stats.agility = 25;
        new_stats.stamina = 77.7f;
        actor_inst.SetVal<Stats>("stats", new_stats);
        Stats updated_stats = actor_inst.GetVal<Stats>("stats");
        print_actor(main_actor_val);

        assert(actor_inst.GetVal<float>("health") == 88.8f);
        assert(updated_stats.strength == 20);
        assert(updated_stats.agility == 25);
        assert(updated_stats.stamina == 77.7f);

        // Test error handling for missing field
        std::cout << "\nTesting error handling for missing field... // main:8\n";
        try {
            actor_inst.GetVal<int>("missing_field");
            assert(false && "Expected exception for missing field");
        }
        catch (const std::out_of_range&) {
            std::cout << "Caught expected exception for missing field // main:9" << std::endl;
        }

        // Test type mismatch
        std::cout << "\nTesting error handling for type mismatch... // main:10\n";
        try {
            actor_inst.GetVal<float>("id"); // id is int
            assert(false && "Expected exception for type mismatch");
        }
        catch (const std::bad_any_cast&) {
            std::cout << "Caught expected exception for type mismatch // main:11" << std::endl;
        }

        // Print final state
        std::cout << "\nFinal objects after reflection modifications: // main:12\n";
        print_scene(scene);

        std::cout << "\nAll verbose reflection tests (including nested classes) passed! // main:13" << std::endl;
        return 0;
    }
}
