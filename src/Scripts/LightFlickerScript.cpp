#include "LightFlickerScript.h"
std::random_device rng{};
std::uniform_int_distribution<int> d(0, 1);

void LightFlickerScript::OnStart(Registry & r)
{
    originalScale = GetComponent<TransformComponent>(r)->scale.x;
    scaleOffset = 1.0f;
};


void LightFlickerScript::OnUpdate(Registry & registry, float dt, bool)
{
    TransformComponent* transform = GetComponent<TransformComponent>(registry);
    int heyhey = d(rng);
    
    float scaleDiff;
    if(heyhey)
        scaleDiff =  -flickerSpeed * dt;
    else
        scaleDiff =  flickerSpeed * dt;
    
    float finalScale = transform->scale.x + scaleDiff;
    if (scaleOffset < 0)
        scaleOffset = -scaleOffset;
    finalScale = std::clamp(finalScale, originalScale - scaleOffset, originalScale + scaleOffset);
    transform->scale = Vec2{finalScale, finalScale};
};

void LightFlickerScript::OnFixedUpdate(Registry&, float, bool)
{
};
