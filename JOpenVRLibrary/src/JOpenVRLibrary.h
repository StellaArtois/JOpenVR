#include <jni.h>
#include <string>
#include <memory>

#if defined(__linux__)
#define COMPILER_GCC
#endif

#include "openvr.h"

/* Header */

#ifndef _Included_com_valvesoftware_openvr_OpenVRLibrary
#define _Included_com_valvesoftware_openvr_OpenVRLibrary
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_valvesoftware_openvr_OpenVR
 * Method:    _initSubsystem
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_valvesoftware_openvr_OpenVR__1initSubsystem
    (JNIEnv *, jobject);
    
/*
 * Class:     com_valvesoftware_openvr_OpenVR
 * Method:    _getLastError
 * Signature: ()Lcom/valvesoftware/openvr/structs/ErrorInfo;
 */
JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getLastError
	(JNIEnv *env, jobject jobj);

/*
 * Class:     com_valvesoftware_openvr_OpenVR
 * Method:    _destroySubsystem
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_valvesoftware_openvr_OpenVR__1destroySubsystem
  (JNIEnv *, jobject);



/* Helpers */
bool LibFirstInit(JNIEnv *env);
void Reset();  
bool CacheJNIGlobals(JNIEnv *env);
bool LookupJNIConstructorGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jmethodID& method,
                     std::string Signature);
bool LookupJNIMethodGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jmethodID& method,
                     std::string Signature,
					 std::string MethodName);
bool LookupJNIFieldGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jfieldID& field,
					 std::string Signature,
					 std::string FieldName);
void SetGenericErrorInfo(JNIEnv *env, const char* error);
void SetErrorInfo(JNIEnv *env, const char* error, vr::EVRInitError eError);
jobject GetLastErrorInfo(JNIEnv *env);                     
    
#ifdef __cplusplus
}
#endif
#endif
