/*!
@file       CollisionCallbackTestScript.h
@author     Zhang Mingyang
@date       15/1/2026

Script to log collision/trigger callback lifecycles.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#pragma once

#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

/*!
* \brief
*   Logs collision and trigger callback lifecycles to validate enter/stay/exit
*   behavior for native scripts.
*/
class CollisionCallbackTestScript : public ScriptInstance
{
public:
    std::string label{ "CollisionCallbackTest" };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    void OnCollisionEnter(const Collider& other);
    void OnCollisionStay(const Collider& other);
    void OnCollisionExit(const Collider& other);
    void OnTriggerEnter(const Collider& other);
    void OnTriggerStay(const Collider& other);
    void OnTriggerExit(const Collider& other);
    void BindFrom() {}
    void BindTo() {}


    REFLECTABLE_PROPERTIES;
};

REFL_AUTO(
    type(CollisionCallbackTestScript),
    field(label)
)