#include "JOpenVRLibrary.h"

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

vr::IVRSystem*       _pHmdSession = 0;
vr::IVRRenderModels* _pRenderModels = 0;

bool                _initialised        = false;
bool                _performedFirstInit = false;

const std::string   UNKNOWN_RUNTIME_VER = "<Unknown>";
const std::string   NO_ERROR            = "<No error>";
std::string         _sRuntimeVersion    = UNKNOWN_RUNTIME_VER;

struct ErrorInfo 
{
	ErrorInfo()
	{
		sError = NO_ERROR;
		eError = vr::VRInitError_None;
		Success = true;
		UnqualifiedSuccess = true;
	}

	std::string sError;
	vr::EVRInitError eError;
	bool        Success;
	bool        UnqualifiedSuccess;
};

ErrorInfo           _lastError;

// JNI class / method caching
static jclass       errorInfo_Class                         = 0;
static jmethodID    errorInfo_constructor_MethodID          = 0;


/* 
Initialises 
   - the OpenVR client -> RT connection
   - the HMD device session
   - gets the HMD render parameters
*/
JNIEXPORT jboolean JNICALL Java_com_valvesoftware_openvr_OpenVR__1initSubsystem(JNIEnv *env, jobject jobj)
{
	vr::EVRInitError eError = vr::VRInitError_None;

	// Do any lib first init
	if (!LibFirstInit(env))
	{
		SetGenericErrorInfo(env, "Failed libFirstInit()");
		return false;
	}

	// Ensure we have a clean state
	Reset();
	
	// Create the HMD session
    _pHmdSession = vr::VR_Init( &eError, vr::VRApplication_Scene );
    if ( eError != vr::VRInitError_None )
    {
		SetErrorInfo(env, "Unable to initialise OpenVR!", eError);
        return false;
    }

	_initialised = true;

	// Get the HMD configuration parameters
	_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	if( !_pRenderModels )
	{
		Reset();
		SetErrorInfo(env, "Unable to initialise OpenVR Rendermodels!", eError);
		return false;
	}
	
 	SetErrorInfo(env, "Initialised OpenVR successfully!", eError);
	return true;
}

JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getLastError(JNIEnv *env, jobject jobj) 
{
	return GetLastErrorInfo(env);
}

JNIEXPORT void JNICALL Java_com_valvesoftware_openvr_OpenVR__1destroySubsystem(JNIEnv *env, jobject jobj) 
{
	printf("Destroying OpenVR device interface.\n");	
	Reset();
}

 
    
/**** HELPERS ****/

void ClearException(JNIEnv *env)
{
    env->ExceptionClear();
}

void PrintNewObjectException(JNIEnv *env, std::string objectName)
{
    printf("Failed to create object '%s'", objectName.c_str());
    env->ExceptionDescribe();
    env->ExceptionClear();
}

void Reset()
{
	if (_initialised)
	{
	    // Shutdown OpenVR
		vr::VR_Shutdown();
	}

	if (_pRenderModels)
		_pRenderModels = 0;

	if (_pHmdSession)
		_pHmdSession = 0;


	_initialised = false;
}

bool CacheJNIGlobals(JNIEnv *env)
{
	bool Success = true;


	if (!LookupJNIConstructorGlobal(env,
                         errorInfo_Class,
                         "com/valvesoftware/openvr/structs/ErrorInfo",
                         errorInfo_constructor_MethodID,
                         "(Ljava/lang/String;IZZ)V"))
    {
        Success = false;
    }

 

    return Success;
}

bool LookupJNIConstructorGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jmethodID& method,
                     std::string Signature)
{
	std::string methodName = "<init>";
	return LookupJNIMethodGlobal(env, clazz, className, method, Signature, methodName);
}

bool LookupJNIMethodGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jmethodID& method,
                     std::string Signature,
					 std::string MethodName)
{
    if (clazz == NULL)
	{
		jclass localClass = env->FindClass(className.c_str());
        if (localClass == 0)
        {
            printf("Failed to find class '%s'", className.c_str());
            return false;
        }

		clazz = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
	}

	if (method == NULL)
	{
		method = env->GetMethodID(clazz, MethodName.c_str(), Signature.c_str());
        if (method == 0)
        {
            printf("Failed to find method '%s' for class '%s' with signature: %s", 
                MethodName.c_str(), className.c_str(), Signature.c_str());
            return false;
        }
	}

    return true;
}

bool LookupJNIFieldGlobal(JNIEnv *env,
                     jclass& clazz,
                     std::string className,
                     jfieldID& field,
					 std::string Signature,
					 std::string FieldName)
{
    if (clazz == NULL)
	{
		jclass localClass = env->FindClass(className.c_str());
        if (localClass == 0)
        {
            printf("Failed to find class '%s'", className.c_str());
            return false;
        }

		clazz = (jclass)env->NewGlobalRef(localClass);
		env->DeleteLocalRef(localClass);
	}

	if (field == NULL)
	{
		field = env->GetFieldID(clazz, FieldName.c_str(), Signature.c_str());
        if (field == 0)
        {
            printf("Failed to find field '%s' for class '%s' with signature: %s", 
                FieldName.c_str(), className.c_str(), Signature.c_str());
            return false;
        }
	}

    return true;
}

bool LibFirstInit(JNIEnv *env)
{
	bool Success = true;
	if (!_performedFirstInit)
	{
		Success = CacheJNIGlobals(env);
		if (Success) 
		{
			_performedFirstInit = true;
		}
	}
	return Success;
}

void SetErrorInfo(JNIEnv *env, const char* error, vr::EVRInitError eError)
{	
	_lastError.sError             = vr::VR_GetVRInitErrorAsEnglishDescription( eError );
	_lastError.eError             = eError;
	_lastError.Success            = (eError == vr::VRInitError_None ? true : false);
	_lastError.UnqualifiedSuccess = (eError == vr::VRInitError_None ? true : false);
}

void SetGenericErrorInfo(JNIEnv *env, const char* error)
{	
    SetErrorInfo(env, error, vr::VRInitError_Unknown);
}

jobject GetLastErrorInfo(JNIEnv *env)
{
	if (!_performedFirstInit)
	{
		return 0;
	}

	ClearException(env);

	jstring jerrorStr = env->NewStringUTF( _lastError.sError.c_str() );
	jobject errorInfo = env->NewObject(errorInfo_Class, errorInfo_constructor_MethodID,
                                      jerrorStr,
									  (int)_lastError.eError,
									  _lastError.Success,
									  _lastError.UnqualifiedSuccess);
	env->DeleteLocalRef( jerrorStr );
	if (errorInfo == 0) PrintNewObjectException(env, "ErrorInfo");
	return errorInfo;
}
