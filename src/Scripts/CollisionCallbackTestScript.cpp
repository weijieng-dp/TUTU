/*!
@file       CollisionCallbackTestScript.cpp
@author     Zhang Mingyang
@date       15/1/2026

Implements collision/trigger callback logging.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#include "CollisionCallbackTestScript.h"

void CollisionCallbackTestScript::OnStart(Registry&)
{
    LOGI("[%s] OnStart (entity %u)", label.c_str(), entity);
}

void CollisionCallbackTestScript::OnUpdate(Registry&, float dt, bool firstframe)
{
    (void)dt;
    (void)firstframe;
}

void CollisionCallbackTestScript::OnFixedUpdate(Registry&, float dt, bool firstframe)
{
    (void)dt;
    (void)firstframe;
}

void CollisionCallbackTestScript::OnCollisionEnter(const Collider& other)
{
    LOGI("[%s] OnCollisionEnter: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

void CollisionCallbackTestScript::OnCollisionStay(const Collider& other)
{
    LOGI("[%s] OnCollisionStay: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

void CollisionCallbackTestScript::OnCollisionExit(const Collider& other)
{
    LOGI("[%s] OnCollisionExit: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

void CollisionCallbackTestScript::OnTriggerEnter(const Collider& other)
{
    LOGI("[%s] OnTriggerEnter: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

void CollisionCallbackTestScript::OnTriggerStay(const Collider& other)
{
    LOGI("[%s] OnTriggerStay: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

void CollisionCallbackTestScript::OnTriggerExit(const Collider& other)
{
    LOGI("[%s] OnTriggerExit: self %u, other %u", label.c_str(), entity, other.GetEntity());
}

