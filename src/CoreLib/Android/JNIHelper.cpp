#include "JNIHelper.h"
/* !
@file    	  JNIHelper.cpp
@author  	  weijie.ng@digipen.edu
@date    	  03/04/2024

The [JNIHelper] class provides helper functions for interacting with Java
code from native C++ on Android. It manages access to the Java VM, obtains
thread-specific JNI environments, loads Java classes, and resolves method
information needed for JNI method calls.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*__________________________________________________________________________*/
#ifdef PLATFORM_ANDROID

#include <pthread.h>

JavaVM* JNIHelper::javaVM = nullptr;
jmethodID JNIHelper::loadclassMethod_methodID = nullptr;
jobject JNIHelper::classloader = nullptr;

static pthread_key_t g_key;


/*!
* \brief
*   Detaches the current native thread from the Java VM when the thread exits.
*   This is used as a pthread thread-local storage destructor so that any
*   thread attached through JNI is properly cleaned up.
*
* \param
*   a - Unused thread-local storage value passed by pthread.
*
* \return
*   None.
*/
void _detachCurrentThread(void* a) {
    JNIHelper::GetJavaVM()->DetachCurrentThread();
}


/*!
* \brief
*   Loads and returns a Java class reference using the stored custom class
*   loader. This is useful on Android where FindClass may fail on native
*   threads that are not created by Java.
*
* \param
*   className - The fully qualified Java class name to load.
*
* \return
*   [jclass] A local reference to the requested Java class if found;
*   otherwise nullptr.
*/
jclass _getClassID(const char *className) {
    if (nullptr == className) {
        return nullptr;
    }

    JNIEnv* env = JNIHelper::GetEnv();

    jstring _jstrClassName = env->NewStringUTF(className);

    jclass _clazz = (jclass) env->CallObjectMethod(JNIHelper::classloader,
                                                   JNIHelper::loadclassMethod_methodID,
                                                   _jstrClassName);

    if (nullptr == _clazz) {
        LOGE("Classloader failed to find class of %s", className);
        env->ExceptionClear();
    }

    env->DeleteLocalRef(_jstrClassName);

    return _clazz;
}

/*!
* \brief
*   Stores the Java VM pointer for later JNI access and initializes the
*   thread-local storage key used to automatically detach native threads
*   from the JVM when they terminate.
*
* \param
*   m_javaVM - Pointer to the Java VM provided by the Android runtime.
*
* \return
*   None.
*/
 void JNIHelper::SetJavaVM(JavaVM* m_javaVM)
{
    pthread_t thisthread = pthread_self();
    javaVM = m_javaVM;
    LOGI("JniHelper::setJavaVM(%p), pthread_self() = %ld", javaVM, thisthread);
    pthread_key_create(&g_key, _detachCurrentThread);
}

 /*!
* \brief
*   Retrieves the stored Java VM pointer used by the JNI helper system.
*
* \return
*   [JavaVM*] Pointer to the active Java VM instance.
*/
JavaVM* JNIHelper::GetJavaVM(){
    pthread_t thisthread = pthread_self();
    LOGD("JniHelper::getJavaVM(), pthread_self() = %ld", thisthread);
    return javaVM;
}

/*!
* \brief
*   Retrieves the JNI environment for the current thread. If the thread is
*   not yet attached to the Java VM, it is attached automatically and marked
*   for detachment through thread-local storage cleanup.
*
* \return
*   [JNIEnv*] Pointer to the JNI environment for the current thread, or
*   nullptr if the thread could not be attached.
*/
JNIEnv* JNIHelper::GetEnv(){
    JNIEnv* env = nullptr;

    if (javaVM->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        // Not attached to attach thread
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return nullptr;
        }

        // Store in TLS so destructor can detach later
        pthread_setspecific(g_key, env);
    }
    return env;
}

/*!
* \brief
*   Retrieves method information for a non-static Java method using the
*   default JNI class loader. It locates the class, finds the method ID,
*   and fills the provided JniMethodInfo structure with the resolved data.
*
* \param
*   methodinfo - Reference to the structure that will store the resolved
*   JNI environment, class reference, and method ID.
* \param
*   className - The fully qualified Java class name containing the method.
* \param
*   methodName - The name of the Java method to retrieve.
* \param
*   paramCode - The JNI method signature describing the parameter and
*   return types of the method.
*
* \return
*   [bool] True if the class and method were found successfully; otherwise
*   false.
*/
 bool JNIHelper::getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                     const char *className,
                                                     const char *methodName,
                                                     const char *paramCode) {
        if ((nullptr == className) ||
            (nullptr == methodName) ||
            (nullptr == paramCode)) {
            return false;
        }

        JNIEnv *env = JNIHelper::GetEnv();
        if (!env) {
            return false;
        }

        jclass classID = env->FindClass(className);
        if (! classID) {
            LOGE("Failed to find class %s", className);
            env->ExceptionClear();
            return false;
        }

        jmethodID methodID = env->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGE("Failed to find method id of %s", methodName);
            env->ExceptionClear();
            return false;
        }

        methodinfo.classID = classID;
        methodinfo.env = env;
        methodinfo.methodID = methodID;

        return true;
    }

 /*!
* \brief
*   Retrieves method information for a static Java method using the stored
*   custom Android class loader. It resolves the class, finds the static
*   method ID, and fills the provided JniMethodInfo structure.
*
* \param
*   methodinfo - Reference to the structure that will store the resolved
*   JNI environment, class reference, and method ID.
* \param
*   className - The fully qualified Java class name containing the static
*   method.
* \param
*   methodName - The name of the static Java method to retrieve.
* \param
*   paramCode - The JNI method signature describing the parameter and
*   return types of the static method.
*
* \return
*   [bool] True if the class and static method were found successfully;
*   otherwise false.
*/
bool JNIHelper::GetStaticMethodInfo(JniMethodInfo& methodinfo,
                                const char* className,
                                const char* methodName,
                                const char* paramCode)
{
    if ((nullptr == className) ||
        (nullptr == methodName) ||
        (nullptr == paramCode)) {
        return false;
    }

    JNIEnv *env = JNIHelper::GetEnv();
    if (!env) {
        LOGE("Failed to get JNIEnv");
        return false;
    }



    jclass classID = _getClassID(className);
    if (! classID) {
        LOGE("Failed to find class %s", className);
        env->ExceptionClear();
        return false;
    }

    jmethodID methodID = env->GetStaticMethodID(classID, methodName, paramCode);
    if (! methodID) {
        LOGE("Failed to find static method id of %s", methodName);
        env->ExceptionClear();
        return false;
    }

    methodinfo.classID = classID;
    methodinfo.env = env;
    methodinfo.methodID = methodID;
    return true;
}


#endif