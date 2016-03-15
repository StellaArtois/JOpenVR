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

struct Pose
{
	Pose()
	{
		orient.x = 0;
		orient.y = 0;
		orient.z = 0;
		orient.w = 1;
		pos.v[0] = 0;
		pos.v[1] = 0;
		pos.v[2] = 0;
	}

	vr::HmdQuaternion_t orient;
	vr::HmdVector3_t pos;
};

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
void ConvertMat34ToPose(const vr::HmdMatrix34_t& mat34, Pose& pose);
void QuatfromRotationMatrix(const vr::HmdMatrix44_t& mat4, vr::HmdQuaternion_t& orientation);
void ConvertMat34to44(const vr::HmdMatrix34_t& mat34, vr::HmdMatrix44_t& mat4);
void QuatSlerp(vr::HmdQuaternion_t q1, vr::HmdQuaternion_t q2, float t, vr::HmdQuaternion_t& quatresult);

#ifdef __cplusplus
}
#endif
#endif
