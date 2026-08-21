
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include <random>

class LightFlickerScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<LightFlickerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<LightFlickerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<LightFlickerScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    float flickerSpeed = 3.0f;
    float originalScale;
    float scaleOffset = 1.0f;
    
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(LightFlickerScript),
    field(flickerSpeed),
    field(scaleOffset)
)