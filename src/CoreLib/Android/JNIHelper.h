#include "../Platform.h"
/* !
@file    	  JNIHelper.h
@author  	  weijie.ng@digipen.edu
@date    	  03/04/2024

The [JNIHelper] class provides helper functions for interacting with Java
code from native C++ on Android. It manages access to the Java VM, obtains
thread-specific JNI environments, loads Java classes, and resolves method
information needed for JNI method calls.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*__________________________________________________________________________*/
#ifdef PLATFORM_ANDROID

#include <JNI.h>


typedef struct JniMethodInfo_
{
    JNIEnv* env;
    jclass      classID;
    jmethodID   methodID;
} JniMethodInfo;

class JNIHelper
{
public:
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
     static void SetJavaVM(JavaVM* javaVM);

     /*!
* \brief
*   Retrieves the stored Java VM pointer used by the JNI helper system.
*
* \return
*   [JavaVM*] Pointer to the active Java VM instance.
*/
    static JavaVM* GetJavaVM();

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
    static JNIEnv* GetEnv();

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
    static bool getMethodInfo_DefaultClassLoader(JniMethodInfo &methodinfo,
                                                     const char *className,
                                                     const char *methodName,
                                                     const char *paramCode);
    
    
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
    static bool GetStaticMethodInfo(JniMethodInfo& methodinfo,
        const char* className,
        const char* methodName,
        const char* paramCode);

    static jmethodID loadclassMethod_methodID;
    static jobject classloader;

private:
    static JavaVM* javaVM;
};



#endif