#include "Haptics.h"
/* !
@file    	  Haptics.cpp
@author  	  weijie.ng@digipen.edu
@date    	  03/04/2024

The [Haptics] system provides vibration support for Android devices by
bridging native C++ code with Java through JNI. This function calls the
Java-side static `vibrate` method in `MainActivity`, allowing gameplay
systems to trigger haptic feedback with a chosen duration and amplitude.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*__________________________________________________________________________*/

#ifdef PLATFORM_ANDROID

/*!
* \brief
*   Triggers vibration on an Android device by calling the corresponding
*   static Java `vibrate` method through JNI. This allows native C++ code
*   to request haptic feedback with a specified duration and strength.
*
* \param
*   milliseconds - The duration of the vibration in milliseconds.
* \param
*   amplitude - The vibration strength to use, passed as a HapticStrength
*   enum value and converted into an integer for the Java method call.
*
* \return
*   None.
*/
void Haptics::VibrateFromCpp(int milliseconds, HapticStrength amplitude) {
    {

        JniMethodInfo t;

        if (JNIHelper::GetStaticMethodInfo(
                t,
                "com/example/jnicpp/MainActivity", 
                "vibrate",
                "(II)V"))
        {
            t.env->CallStaticVoidMethod(t.classID, t.methodID, milliseconds,static_cast<int>(amplitude));

            if (t.env->ExceptionCheck()) {
                t.env->ExceptionDescribe();
                t.env->ExceptionClear();
            }

            t.env->DeleteLocalRef(t.classID);
        }
        else
        {
            LOGE("Failed to call Vibrate");
        }
    }
}
#endif
