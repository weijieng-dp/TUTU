/*!
@file       HierarchyManager.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       25/09/2025
@brief
            Declares the HierarchyManager class, which provides functionality
            for managing parent-child relationships between entities and
            traversing entity hierarchies within the ECS.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*______________________________________________________________________*/

#pragma once
#include "Registry.h"
#include "components.h"
#include <queue>
#include <functional>
#include "Components.h"

class HierarchyManager
{
public:
	HierarchyManager() {};

    /*!
     * \brief
     *   Retrieves a list of all entities in the hierarchy.
     *
     * \return
     *   - Vector of Entity IDs representing the hierarchy.
     */
    std::vector<EntityRegistry::Entity> GetHierarchyList();

    /*!
     * \brief
     *   Updates the currently active hierarchy, typically after changes
     *   such as reparenting or adding/removing entities.
     */
    void UpdateActiveHierarchy();

    /*!
     * \brief
     *   Changes the parent of a child entity to a new parent.
     *
     * \param[in] child
     *   - Entity ID of the child to reparent.
     *
     * \param[in] newParent
     *   - Entity ID of the new parent entity.
     */
    void Reparent(EntityRegistry::Entity child, EntityRegistry::Entity newParent);

    /*!
     * \brief
     *   Reparents a child entity to the root of the hierarchy.
     *
     * \param[in] child
     *   - Entity ID of the child to move to the root.
     *
     * \return
     *   - true if the operation succeeded, false otherwise.
     */
    bool ReparentToRoot(EntityRegistry::Entity child);

    /*!
    * \brief
    *   Traverses the hierarchy starting from a root entity and applies
    *   a user-defined function to each entity.
    *
    * \tparam OutParam
    *   - Additional output parameters to pass to the process function.
    *
    * \param[in] registry
    *   - ECS registry containing the hierarchy components.
    *
    * \param[in] root
    *   - Entity ID to start traversal from.
    *
    * \param[in] process
    *   - Function to execute for each entity, signature:
    *     void(EntityRegistry::Entity, OutParam&...)
    *
    * \param[out] outParam
    *   - References to additional output parameters for the process function.
    *
    * \note
    *   Traversal is breadth-first. Children are processed in the order
    *   they appear under their parent.
    */
    template <typename... OutParam>
    static void Traverse(Registry& registry, EntityRegistry::Entity root,
        std::function<void(EntityRegistry::Entity, OutParam&...)> process,
        OutParam&... outParam) {
        std::queue<EntityRegistry::Entity> toProcess;
        toProcess.push(root);

        while (!toProcess.empty()) {
            EntityRegistry::Entity curr{ toProcess.front() };
            toProcess.pop();

            process(curr, outParam...);

            if (HierarchyComponnent* comp = registry.GetComponent<HierarchyComponnent>(curr); comp && comp->firstChild) {
                curr = comp->firstChild;
                comp = registry.GetComponent<HierarchyComponnent>(curr);
                do {
                    toProcess.push(curr);
                    curr = comp->nextSibling;
                    comp = registry.GetComponent<HierarchyComponnent>(curr);
                } while (curr);
            }
        }
    }
    // no output param version of the above function
	static void Traverse(Registry& registry, EntityRegistry::Entity root, std::function<void(EntityRegistry::Entity)> process);

private:
	std::vector < EntityRegistry::Entity> HierarchyList;

};