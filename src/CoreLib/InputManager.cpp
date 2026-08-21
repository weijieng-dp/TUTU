/**___________________________________________________________________________/
@file          InputManager.cpp
@author        Ng Wei Jie (weijie.ng) 100%
@date          01/10/2025

This file implements the platform-abstracted [Input] system for handling user
input across Windows and non-Windows platforms (e.g., mobile/touch).

It provides:
- Singleton instance management for [Input].
- On mobile platforms:
  - A `Controller` class for joystick-style touch input.
  - Functions to draw and update the virtual joystick UI.
  - Utility to retrieve the normalized joystick direction.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "InputManager.h"
#include "CEO.h"
Input* Input::Instance = nullptr;


#ifndef PLATFORM_WINDOWS


void Input::Controller::InitJoystick(std::string const& texture, std::string const& backgroundtexture)
{
     joystickTexture = &CEO::Get<ResourceManager>()->GetTexture(texture);
     joystickBackgroundTexture = &CEO::Get<ResourceManager>()->GetTexture(backgroundtexture);
}

void Input::Controller::DrawJoystick(GLSLShader& shader, Vec2 const& GivenScreenSize)
{
    if (index == 999) return;

    static bool IsInitialized = false;
    shader.Use();
    //LOGI("given screen size %f, %f", screenSize.x, screenSize.y);
    //screenSize = GivenScreenSize;
    if (!IsInitialized)
    {
        originalPos = Vec2(-GivenScreenSize.x * 2 / 6, -GivenScreenSize.y / 5);
        IsInitialized = true;
    }
    Mat3 transform = Mat3::Identity();
    Mat3 scale = Mat3::Scale(radius*2.0f, radius * 2.0f);
    Mat3  translate = Mat3::Translation(originalPos.x, originalPos.y);

    Mat3 xform{ Mat3::Scale(2.f / GivenScreenSize.x,2.f / GivenScreenSize.y) };
    Color color(1,1,1,1);
    shader.SetUniform("uColour", color);

    shader.SetUniform("mdl_to_ndc", xform * translate * scale);   // pass in the transformation matrix
    shader.SetUniform("uUseTex", 1);                        // tell shaders to use Texture for rendering


    joystickBackgroundTexture->BindTexture();
    CEO::Instance().Get<RenderUtils>()->RenderQuad();
    joystickBackgroundTexture->UnbindTexture();


    transform = Mat3::Identity();
    scale = Mat3::Scale(radius * 2.0f, radius * 2.0f);
    translate = Mat3::Translation(currentPos.x, currentPos.y);

    xform = Mat3::Scale(2.f / GivenScreenSize.x , 2.f / GivenScreenSize.y );
    shader.SetUniform("uColour", color); 

    shader.SetUniform("mdl_to_ndc", xform * translate * scale);   // pass in the transformation matrix
    shader.SetUniform("uUseTex", 1);                        // tell shaders to use Texture for rendering

    joystickTexture->BindTexture();
    CEO::Instance().Get<RenderUtils>()->RenderQuad();
    joystickTexture->UnbindTexture();

    shader.UnUse();
}

void Input::Controller::UpdateJoystick()
{
    if(ID == 1) {
        for (auto &[id, Action]: Input::GetAllPointer()) {
            if (Input::IsPointerHeld(id)) {
                Vec2 Pos = Vec2(Input::GetX(id), Input::GetY(id));
                if (Pos.x <= -(screenSize.x /2) / 5  && Pos.x >= -screenSize.x * 2 && Pos.y <= screenSize.y / 4 &&
                    Pos.y >= -screenSize.y / 2) {
                    if (index > id)
                        index = id;
                }
            }
        }

        if (Input::IsPointerPressed(index)) {
            Vec2 InputPos = Vec2(Input::GetX(index), Input::GetY(index));
            if (InputPos.x < -(screenSize.x /2) / 5 && InputPos.x > -screenSize.x * 2 && InputPos.y < screenSize.y / 4 &&
                InputPos.y > -screenSize.y / 2) {
                originalPos = InputPos;
            }

        }

        if (Input::IsPointerHeld(index)) {

            Vec2 InputPos = Vec2(Input::GetX(index), Input::GetY(index));


            if ((InputPos - originalPos).LengthSquared() <= radius * radius) {
                currentPos = Vec2(Input::GetX(index), Input::GetY(index));
            } else {
                float displacement = (InputPos - originalPos).Length();
                float ratio = radius / displacement;
                float constraintX = originalPos.x + (InputPos.x - originalPos.x) * ratio;
                float constraintY = originalPos.y + (InputPos.y - originalPos.y) * ratio;
                currentPos = Vec2(constraintX, constraintY);

            }

            if ((InputPos - originalPos).LengthSquared() < 0.03f * (radius * radius)) {
                currentPos = originalPos;
                index = 998;
            }
        } else {
            currentPos = originalPos;
            index = 999;
        }
    }
    else if(ID ==2)
    {
        for (auto &[id, Action]: Input::GetAllPointer()) {
            if (Input::IsPointerHeld(id)) {
                Vec2 Pos = Vec2(Input::GetX(id), Input::GetY(id));
                if (Pos.x >= (screenSize.x /2) / 5 && Pos.x <= screenSize.x / 2 && Pos.y <= screenSize.y / 4 &&
                    Pos.y >= -screenSize.y / 2) {
                    if (index > id)
                        index = id;
                }
            }
        }

        if (Input::IsPointerPressed(index)) {
            Vec2 InputPos = Vec2(Input::GetX(index), Input::GetY(index));
            if (InputPos.x >= (screenSize.x /2) / 5 && InputPos.x <= screenSize.x / 2 && InputPos.y < screenSize.y / 4 &&
                InputPos.y > -screenSize.y / 2) {
                originalPos = InputPos;
            }

        }

        if (Input::IsPointerHeld(index)) {

            Vec2 InputPos = Vec2(Input::GetX(index), Input::GetY(index));


            if ((InputPos - originalPos).LengthSquared() <= radius * radius) {
                currentPos = Vec2(Input::GetX(index), Input::GetY(index));
            } else {
                float displacement = (InputPos - originalPos).Length();
                float ratio = radius / displacement;
                float constraintX = originalPos.x + (InputPos.x - originalPos.x) * ratio;
                float constraintY = originalPos.y + (InputPos.y - originalPos.y) * ratio;
                currentPos = Vec2(constraintX, constraintY);

            }

            if ((InputPos - originalPos).LengthSquared() < 0.03f * (radius * radius)) {
                currentPos = originalPos;
                index = 998;
            }
        } else {
            currentPos = originalPos;
            index = 999;
        }
    }


}

Vec2 Input::Controller::GetJoystickValue() {
    return (currentPos - originalPos).Normalised();
}

void Input::Controller::ScreenSize(Vec2 newSize)
{
    screenSize = newSize;
}

#endif
#ifdef PLATFORM_WINDOWS
Vec2 Input::ApplyStickDeadzone(float x, float y) const
{
    float deadzone = 0.15f;

    float magnitude = sqrt(x * x + y * y);

    if (magnitude < deadzone)
        return Vec2(0.0f, 0.0f);

    // Normalize direction
    float nx = x / magnitude;
    float ny = y / magnitude;

    // Optional: scale smoothly (nice feel)
    float scaled = (magnitude - deadzone) / (1.0f - deadzone);

    return Vec2(nx * scaled, ny * scaled);
}
#endif