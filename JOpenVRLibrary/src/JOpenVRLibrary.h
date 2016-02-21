#include <jni.h>
#include <string>
#include <memory>


/* Header */

#ifndef _Included_openvr_OpenVRLibrary
#define _Included_openvr_OpenVRLibrary
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     openvr
 * Method:    _initSubsystem
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_openvr__1initSubsystem
    (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
