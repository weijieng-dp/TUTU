/*!
@file       HierarchyManager.cpp
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

#include "pch.h"
#include "HierarchyManager.h"
#include "Components.h"

void HierarchyManager::UpdateActiveHierarchy()
{
	std::vector <EntityRegistry::Entity> TempHierarchyList;
	Registry* reg = CEO::Instance().GetManager<Registry>();
	for (auto root : reg->GetEntitiesWithComponent<HierarchyComponnent>())
	{
		auto* h = reg->GetComponent<HierarchyComponnent>(root);

		if (h->parent != 0) continue;

		EntityRegistry::Entity cur = root;

		while (cur != 0)
		{
			 h = reg->GetComponent<HierarchyComponnent>(cur);
			auto* a = reg->GetComponent<ActiveComponent>(cur);


			if (h->parent == 0)
			{
				a->isActiveInHierarchy = a->isActiveSelf;
				TempHierarchyList.push_back(cur);
			}
			else
			{
				auto* parentAct = reg->GetComponent<ActiveComponent>(h->parent);
				a->isActiveInHierarchy = a->isActiveSelf && parentAct->isActiveInHierarchy;
			}

			// Traverse through children/sibling (your iterative logic)
			if (h->firstChild != 0)
			{
				cur = h->firstChild;
				TempHierarchyList.push_back(cur);
			}
			else
			{
				while (cur != 0)
				{
					auto* hc = reg->GetComponent<HierarchyComponnent>(cur);
					if (hc->nextSibling != 0)
					{
						cur = hc->nextSibling;
						TempHierarchyList.push_back(cur);
						break;
					}
					cur = hc->parent;
				}
			}
		}
	}
	HierarchyList = std::move(TempHierarchyList);
}

std::vector <EntityRegistry::Entity> HierarchyManager::GetHierarchyList()
{
	return HierarchyList;
}

void HierarchyManager::Reparent(EntityRegistry::Entity child, EntityRegistry::Entity newParent)
{
	Registry* registry = CEO::Instance().GetManager<Registry>();
	auto* hChild = registry->GetComponent<HierarchyComponnent>(child);

	TransformComponent* newPTransform = registry->GetComponent<TransformComponent>(newParent);
	TransformComponent* newCTransform = registry->GetComponent<TransformComponent>(child);

	if (newPTransform && newCTransform)
	{
		Mat3 LocalMat = newPTransform->transform.Inversed() * newCTransform->transform;

		newCTransform->translate = Vec2(LocalMat.m[6], LocalMat.m[7]);

		newCTransform->scale = Vec2(sqrtf(LocalMat.m[0] * LocalMat.m[0] + LocalMat.m[1] * LocalMat.m[1]),
			sqrtf(LocalMat.m[3] * LocalMat.m[3] + LocalMat.m[4] * LocalMat.m[4]));
		newCTransform->rotation = atan2f(LocalMat.m[3], LocalMat.m[0]);
	}



	EntityRegistry::Entity oldParent = hChild->parent;

	// 1. Remove child from old parent's child list
	if (oldParent != 0)
	{
		auto* hOld = registry->GetComponent<HierarchyComponnent>(oldParent);

		// If child is firstChild
		if (hOld->firstChild == child)
		{
			hOld->firstChild = hChild->nextSibling;
		}
		else
		{
			// find previous sibling
			EntityRegistry::Entity sib = hOld->firstChild;
			while (sib != 0)
			{
				auto* hs = registry->GetComponent<HierarchyComponnent>(sib);
				if (hs->nextSibling == child)
				{
					hs->nextSibling = hChild->nextSibling;
					break;
				}
				sib = hs->nextSibling;
			}
		}
	}

	// 2. Insert child at the front of newParent's children
	auto* hNew = registry->GetComponent<HierarchyComponnent>(newParent);
	hChild->nextSibling = hNew->firstChild;
	hNew->firstChild = child;
	hChild->parent = newParent;

	if (PrefabComponent* prefabComp = registry->GetComponent<PrefabComponent>(child)) {
		EntityRegistry::Entity curr = hChild->parent;
		while (curr != 0) {
			if (registry->HasComponent<PrefabComponent>(curr)) {
				prefabComp->root = false;
				break;
			}
			curr = registry->GetComponent<HierarchyComponnent>(curr)->parent;
		}
	}
}

bool HierarchyManager::ReparentToRoot(EntityRegistry::Entity child)
{
	Registry* reg = CEO::Instance().GetManager<Registry>();
	auto* hChild = reg->GetComponent<HierarchyComponnent>(child);
	if (!hChild) return false;

	EntityRegistry::Entity  oldParent = hChild->parent;

	TransformComponent* newCTransform = reg->GetComponent<TransformComponent>(child);

	if (newCTransform)
	{
		Mat3 WorldMat = newCTransform->transform;

		newCTransform->translate = Vec2(WorldMat.m[6], WorldMat.m[7]);
		newCTransform->scale = Vec2(sqrtf(WorldMat.m[0] * WorldMat.m[0] + WorldMat.m[1] * WorldMat.m[1]),
			sqrtf(WorldMat.m[3] * WorldMat.m[3] + WorldMat.m[4] * WorldMat.m[4]));
		newCTransform->rotation = atan2f(WorldMat.m[3], WorldMat.m[0]);
	}
	// 1. Remove from old parent's child list
	if (oldParent != 0)
	{
		auto* hOld = reg->GetComponent<HierarchyComponnent>(oldParent);

		// Case: removing the firstChild
		if (hOld->firstChild == child)
		{
			hOld->firstChild = hChild->nextSibling;
		}
		else
		{
			// Find previous sibling
			EntityRegistry::Entity sib = hOld->firstChild;
			while (sib != 0)
			{
				auto* hS = reg->GetComponent<HierarchyComponnent>(sib);
				if (hS->nextSibling == child)
				{
					hS->nextSibling = hChild->nextSibling;
					break;
				}
				sib = hS->nextSibling;
			}
		}
	}

	// 2. Mark this entity as a root
	hChild->parent = 0;
	hChild->nextSibling = 0;  // no siblings initially
	if (PrefabComponent* prefabComp = reg->GetComponent<PrefabComponent>(child)) {
		prefabComp->root = true;
	}
	return true;
}

//no out param overload
void HierarchyManager::Traverse(Registry& registry, EntityRegistry::Entity root, std::function<void(EntityRegistry::Entity)> process) {
	std::queue<EntityRegistry::Entity> toProcess;
	toProcess.push(root);

	while (!toProcess.empty()) {
		//Get current Node
		EntityRegistry::Entity curr{ toProcess.front() };
		toProcess.pop();

		//process current Node
		process(curr);

		/*
		if there are children in current Node, add them to toProcess queue

		This adds all children to be processed, in the next iteration, the first child will be processed first

		1st Iteration:
		toProcess = [child1, child2, child3]
		2nd Iteration:
		toProcess = [child2, child3, grandChild1.1, grandChild1.2]
		3rd Iteration:
		toProcess = [child3, grandChild1.1, grandChild1.2, grandChild2.1, grandChild2.2]
		*/
		if (HierarchyComponnent* comp = registry.GetComponent<HierarchyComponnent>(curr); comp && comp->firstChild) {
			curr = comp->firstChild; // start from first child 
			comp = registry.GetComponent<HierarchyComponnent>(curr);
			do
			{
				// Adds child entity to toProcess stack
				toProcess.push(curr);
				curr = comp->nextSibling; // move to next sibling
				comp = registry.GetComponent<HierarchyComponnent>(curr);
			} while (curr);// check if there is a next sibling
		}
	}
}