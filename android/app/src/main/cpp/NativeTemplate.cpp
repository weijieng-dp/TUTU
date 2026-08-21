/**___________________________________________________________________________/
@file          NativeTemplate.cpp
@author        Ou Yukang (yukang.ou) 50%
@author        weijie.ng@digipen.edu (wei jie) 50%
@date          29/09/2025
@brief         Entry point for native code on android application and 
               Java-Native interface

Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include <jni.h>
#include <string>
#include <android/log.h>
// Ensure JNI macros are available
#ifndef JNIEXPORT
#define JNIEXPORT
#endif
#ifndef JNICALL
#define JNICALL
#endif
#include "GetMeOutOfHell/glapp.h"
#include <iostream>
#include "CoreLib/Registry.h"
#include "CoreLib/ScriptingAPI.h"
#include "CoreLib/Json.h"
#include "demo.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "CEO.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "SceneManager.h"
#include "FontManager.h"
#include "ScreenManager.h"
#include "RuntimeReflect.h"
#include "NativeScriptLoader.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "Camera.h"
#include "Pathfind.h"
#include "GraphicsSystem.h"
#include "LayerManager.h"
#include "DebugRender.h"
#include "UpdateStackManager.h"
#include "Application.h"
#include "HierarchyManager.h"
#include "Android/JNIHelper.h"

#ifndef LOG_TAG
#define LOG_TAG "NativeTemplate"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif


//static void draw(Registry& registry , ComponentRegistry& ComponentRegistry);
static void Loop(float dt);
static void FixedUpdate(float fixedDt);
static void initmanager();

static bool initialize = false;
static bool firstframe = true;
static const float fixedDeltaTime = 1.0f / 60.0f;
static float accumulator = 0.0f;
static int counter = 0;

// Global renderer instance
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_jnicpp_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {
    LOGI("stringFromJNI called");
    std::string hello = "OpenGL ES 3.0 Triangle Demo";
    return env->NewStringUTF(hello.c_str());
}
std::unordered_map<std::string, rtr::TypeInfo> typeMap;


extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_jnicpp_MainActivity_initGL(JNIEnv* env, jobject /* this */,
                                            jobject assetManager) {
    CEO::Instance().SetInstance(new CEO);
	CEO::Instance().AddManager<Application>();
    CEO::Instance().GetManager<Application>()->InitCoreManagers();
    CEO::Instance().AddManager<JNIHelper>();
    rtr::SetSharedMap(&typeMap);
    NativeComponentInitialise(*CEO::Instance().GetManager<Registry>(), *CEO::Instance().GetManager<ComponentRegistry>(), &typeMap, nullptr, CEO::Instance().GetPtr());

    AAssetManager* nativeAssetManager = nullptr;
    nativeAssetManager = AAssetManager_fromJava(env, assetManager);
    if (nativeAssetManager == nullptr) {
        LOGE("Failed to get native AssetManager");
        return JNI_FALSE;
    }
    LOGI("AssetManager obtained successfully");
    CEO::Instance().GetManager<FileManager>()->Initialize(nativeAssetManager);

    LOGI("initGL called");
    // start with a 16:9 aspect ratio
    if (!GLApp::init(*CEO::Instance().GetManager<Registry>(), *CEO::Instance().GetManager<ComponentRegistry>())) {
        //std::cout << "Unable to create OpenGL context" << std::endl;
        LOGI("ONO");
        return JNI_FALSE;
    }
    //CEO::Instance().GetManager<ResourceManager>()->LoadSceneEntities(*CEO::Instance().GetManager<Registry>(), *CEO::Instance().GetManager<ComponentRegistry>(), "GameStart_Scene");



    //DEMO::error_demo();

    return JNI_TRUE;
}
/*
Make sure callbacks are invoked when state changes in input devices occur.
Ensure time per frame and FPS are recorded.
Let application update state changes (such as animation).
*/
extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_renderFrame(JNIEnv* env, jobject /* this */)
{
    Input::UpdateTouchStart();
    // process events if any associated with input devices
    static auto currentTime = std::chrono::high_resolution_clock::now();

    auto newTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> frameDuration = newTime - currentTime;
    currentTime = newTime;

    float frameTime = frameDuration.count();
    accumulator += (frameTime <= 0.25f? frameTime : 0.25f);

    // main loop computes fps and other time related stuff once for all apps ...
    GLApp::update_time(1.0);

    // Handle UI Input
    UIManager::UpdateUIInput();

    // app update
    Loop(frameTime);

    Input::UpdateTouchEnd();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_screenSizeChanged(JNIEnv* env, jobject /* this */, int width, int height) {
    GLApp::ChangeFrameBufferSize(width, height);
//    if (!initialize) {
//        LOGI("INITING EDITOR IN screenSizeChanged");
//        Editor::Instance().InitEditor(static_cast<float>(GLApp::width), static_cast<float>(GLApp::height), compRegistry);
//        Editor::Instance().LoadProj("project");
//        Editor::Instance().InitDemo(registry, compRegistry);
//        initialize = true;
//    } else {
//        ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(GLApp::width), static_cast<float>(GLApp::height));
//    }
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_cleanupGL(JNIEnv* env, jobject /* this */)
{
    LOGI("cleanupGL called");

//    LOGI("Cleaning editor");
//    Editor::Instance().EditorCleanup();

    LOGI("Cleaning app");
    GLApp::cleanup();

    CEO::Instance().Free();

    initialize = false;
    firstframe = true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_nativeOnTouch(JNIEnv*, jobject, jint action, jint pointerID,jfloat x, jfloat y) {
    Input::UpdateTouch(pointerID,action,x,y);

}

extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_nativeOnTouchEnd(JNIEnv*, jobject) {
    Input::UpdateTouchEnd();
}

//extern "C" JNIEXPORT void JNICALL
//Java_com_example_jnicpp_MainActivity_initIMGUI(JNIEnv* env, jobject /* this */) {
//    if(!initialize) {
//        LOGI("INITING EDITOR");
//        if(Editor::Instance().InitEditor(static_cast<float>(GLApp::width), static_cast<float>(GLApp::height), *CEO::Instance().GetManager<ComponentRegistry>())) {
//            Editor::Instance().LoadProj("project");
//            Editor::Instance().InitDemo(*CEO::Instance().GetManager<Registry>(), *CEO::Instance().GetManager<ComponentRegistry>());
//            initialize = true;
//            LOGI("EDITOR INITIALISATION PASSED");
//        }
//        else {
//            LOGI("EDITOR INITIALISATION FAILED");
//            initialize = false;
//        }
//    }
//}


extern "C" JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_focusChanged(JNIEnv *env, jobject thiz, jboolean is_focused) {
    GLApp::ChangeFocus((is_focused));
}

/*!
* \brief
*	Update function that handles the update loop
*
* \param
*	[float] dt - delta time
*/
static void Loop(float dt) {

    CEO::Instance().GetManager<HierarchyManager>()->UpdateActiveHierarchy();
    CEO::Instance().GetManager<Pathfind>()->DrawCollisionMap(*(CEO::Instance().GetManager<Registry>()));
    Registry& registry = *CEO::Instance().GetManager<Registry>();
    // call on start for scripts if necessary
    CreateScriptInstance(registry, *CEO::Instance().GetManager<ComponentRegistry>());
    CEO::Instance().GetManager<ResourceManager>()->Update();
    // fixed updates
    if (accumulator >= fixedDeltaTime) {
        counter = 0;
        do {
            if(counter == 60)
            {
                accumulator = 0;
                break;
            }
            FixedUpdate(fixedDeltaTime);
            GLApp::FixedUpdate(registry, fixedDeltaTime);
            accumulator -= fixedDeltaTime;
            ++counter;
        } while (accumulator > fixedDeltaTime);
    }

    // scripts update
    UpdateScriptInstance(registry, *CEO::Instance().GetManager<ComponentRegistry>(), dt, firstframe);

    // engine update
    GLApp::Update(registry);
    GLApp::draw(registry);
    GLApp::PostRenderUpdate(registry);
}

/*!
* \brief
*	Update function that handles the update loop
*
* \param
*	[float] fixedDt - delta time
*/
static void FixedUpdate(float fixedDt) {

    // main loop computes fps and other time related stuff once for all apps ...


    FixedUpdateScriptInstance(*CEO::Instance().GetManager<Registry>(), *CEO::Instance().GetManager<ComponentRegistry>(), fixedDt, firstframe);

    Registry& r = *(CEO::Instance().GetManager<Registry>());

    auto v = r.GetEntitiesWithComponent<StateComponent>();
	UpdateStackManager* usm = CEO::Get<UpdateStackManager>();
    for (EntityRegistry::Entity e : v) {

        if (usm->ShouldNotUpdate(e)) continue;
        if (!usm->IsActive(e)) continue;

        auto& stateVec = r.GetComponent<StateComponent>(e)->stateMachines;
        std::for_each(stateVec.begin(), stateVec.end(), [&r,e,fixedDt](std::shared_ptr<IStateMachine>& sm) {
                          sm->Update(r, e, fixedDt);
                      }
        );
    }

    // Only step when not paused OR single-step is triggered
    if (PhysicsSystem::Instance().ShouldStep()) {
        PhysicsSystem::Instance().Update(*CEO::Instance().GetManager<Registry>(), fixedDt);
        CollisionSystem::Instance().Update(*CEO::Instance().GetManager<Registry>());
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_example_jnicpp_MainActivity_killAppJNI(JNIEnv *env, jobject thiz) {
	return Application::IsExitSignalled();
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
JNIHelper::SetJavaVM(vm);
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jnicpp_MainActivity_nativeInitClassLoader(
        JNIEnv* env,
        jobject /*thiz*/,
        jobject loader)
{
    if (!loader) {
        LOGE("loader is NULL");
        return;
    }

    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    if (!classLoaderClass) {
        LOGE("Failed to find ClassLoader class");
        return;
    }

    jmethodID loadClassMethod = env->GetMethodID(
            classLoaderClass,
            "loadClass",
            "(Ljava/lang/String;)Ljava/lang/Class;"
    );

    if (!loadClassMethod) {
        LOGE("Failed to get loadClass method");
        env->DeleteLocalRef(classLoaderClass);
        return;
    }

    JNIHelper::classloader = env->NewGlobalRef(loader);
    JNIHelper::loadclassMethod_methodID = loadClassMethod;

    env->DeleteLocalRef(classLoaderClass);

    LOGI("ClassLoader initialized successfully");
}