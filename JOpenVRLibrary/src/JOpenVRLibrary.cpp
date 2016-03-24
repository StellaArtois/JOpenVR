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
static jclass       renderTextureInfo_Class                 = 0;
static jmethodID    renderTextureInfo_constructor_MethodID  = 0;
static jclass       matrix4f_Class                          = 0;
static jmethodID    matrix4f_constructor_MethodID           = 0;
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
    
    // Setup the compositor
    if ( !vr::VRCompositor() )
    {
        Reset();
		SetGenericErrorInfo(env, "Unable to initialise OpenVR compositor!");
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

JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getHmdParameters(JNIEnv *env, jobject) 
{
	if (!_initialised) 
        return 0;

    jstring productName = env->NewStringUTF( _hmdDesc.ProductName == NULL ? "" : _hmdDesc.ProductName );
    jstring manufacturer = env->NewStringUTF( _hmdDesc.Manufacturer == NULL ? "" : _hmdDesc.Manufacturer );
	jstring serialnumber = env->NewStringUTF( _hmdDesc.SerialNumber == NULL ? "" : _hmdDesc.SerialNumber );
    
    ClearException(env);

    jobject jHmdDesc = env->NewObject(hmdDesc_Class, hmdDesc_constructor_MethodID,
                                      (int)_hmdDesc.Type,
									  productName,
                                      manufacturer,
									  (int)_hmdDesc.VendorId,
									  (int)_hmdDesc.ProductId,
									  serialnumber,
									  (int)_hmdDesc.FirmwareMajor,
									  (int)_hmdDesc.FirmwareMinor,
									  _hmdDesc.CameraFrustumHFovInRadians,
									  _hmdDesc.CameraFrustumVFovInRadians,
									  _hmdDesc.CameraFrustumNearZInMeters,
									  _hmdDesc.CameraFrustumFarZInMeters,
                                      (int)_hmdDesc.AvailableHmdCaps,
									  (int)_hmdDesc.DefaultHmdCaps,
                                      (int)_hmdDesc.AvailableTrackingCaps,
                                      (int)_hmdDesc.DefaultTrackingCaps,
                                      _hmdDesc.DefaultEyeFov[0].UpTan,
                                      _hmdDesc.DefaultEyeFov[0].DownTan,
                                      _hmdDesc.DefaultEyeFov[0].LeftTan,
                                      _hmdDesc.DefaultEyeFov[0].RightTan,
                                      _hmdDesc.DefaultEyeFov[1].UpTan,
                                      _hmdDesc.DefaultEyeFov[1].DownTan,
                                      _hmdDesc.DefaultEyeFov[1].LeftTan,
                                      _hmdDesc.DefaultEyeFov[1].RightTan,
                                      _hmdDesc.MaxEyeFov[0].UpTan,
                                      _hmdDesc.MaxEyeFov[0].DownTan,
                                      _hmdDesc.MaxEyeFov[0].LeftTan,
                                      _hmdDesc.MaxEyeFov[0].RightTan,
                                      _hmdDesc.MaxEyeFov[1].UpTan,
                                      _hmdDesc.MaxEyeFov[1].DownTan,
                                      _hmdDesc.MaxEyeFov[1].LeftTan,
                                      _hmdDesc.MaxEyeFov[1].RightTan,
									  _hmdDesc.Resolution.w,
                                      _hmdDesc.Resolution.h,
									  _hmdDesc.DisplayRefreshRate
									  );

    env->DeleteLocalRef( productName );
    env->DeleteLocalRef( manufacturer );
    env->DeleteLocalRef( serialnumber );

    if (jHmdDesc == 0) PrintNewObjectException(env, "HmdParameters");

    return jHmdDesc;
} 

JNIEXPORT void JNICALL Java_com_valvesoftware_openvr_OpenVR__1resetTracking(JNIEnv *env, jobject) 
{
	if (!_initialised)
		return;

	_pHmdSession->ResetSeatedZeroPose();
}

JNIEXPORT void JNICALL Java_com_valvesoftware_openvr_OpenVR__1configureRenderer(
	JNIEnv *env,
    jobject,
    jfloat worldScale)
{
	if (!_initialised)
		return;

	//_worldScale = worldScale;
}


JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getRenderTextureSize(
	JNIEnv *env, 
	jobject, 
	jfloat leftFovUpTan,
	jfloat leftFovDownTan,
	jfloat leftFovLeftTan,
	jfloat leftFovRightTan,
	jfloat rightFovUpTan,
	jfloat rightFovDownTan,
	jfloat rightFovLeftTan,
	jfloat rightFovRightTan,
	jfloat RenderScaleFactor)
{
	if (!_initialised)
		return 0;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t scaledWidth = 0;
	uint32_t scaledHeight = 0;
	uint32_t totalWidth = 0;
	uint32_t totalHeight = 0;

	// TODO: Is this individual eye render target size?
	_pHmdSession->GetRecommendedRenderTargetSize(&width, &height);

	// A RenderScaleFactor of 1.0f signifies default (non-scaled) operation
	scaledWidth = (uint32_t)ceil((float)width * RenderScaleFactor);
	scaledHeight = (uint32_t)ceil((float)height * RenderScaleFactor);
	totalWidth = scaledWidth * 2;
	totalHeight = scaledHeight;

    ClearException(env);

    jobject jrenderTextureInfo = env->NewObject(renderTextureInfo_Class, renderTextureInfo_constructor_MethodID,
									scaledWidth,
									scaledHeight,
									scaledWidth,
									scaledHeight,
                                    totalWidth,
                                    totalHeight,
									_hmdDesc.Resolution.w,
									_hmdDesc.Resolution.h,
                                    (float)RenderScaleFactor
                                    );

    if (jrenderTextureInfo == 0) PrintNewObjectException(env, "RenderTextureInfo");

    return jrenderTextureInfo;
}
    
JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getTrackedPoses(
	JNIEnv *env, 
	jobject, 
	jlong FrameIndex
	)
{
    if (!_initialised)
        return 0;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	m_rTrackedDevicePose[0].mDeviceToAbsoluteTracking

	OpenVRUtil.convertMatrix4toQuat(hmdPose, rotStore);

	// Use mandated view offsets
	ovrVector3f ViewOffsets[2] = { _EyeRenderDesc[0].HmdToEyeViewOffset,
                                   _EyeRenderDesc[1].HmdToEyeViewOffset };

	// Get eye poses at our predicted display times
	double ftiming = ovr_GetPredictedDisplayTime(_pHmdSession, FrameIndex);
    _sensorSampleTime = ovr_GetTimeInSeconds();
	//ovr_GetInputState(_pHmdSession, ovrControllerType_All, &_inputState);
    _hmdState = ovr_GetTrackingState(_pHmdSession, ftiming, ovrTrue);
    ovr_CalcEyePoses(_hmdState.HeadPose.ThePose, ViewOffsets, _eyeRenderPose);
	double PredictedDisplayTime = ovr_GetPredictedDisplayTime(_pHmdSession, FrameIndex);

    ClearException(env);
	jobject jfullposestate = env->NewObject(fullPoseState_Class, fullPoseState_constructor_MethodID,
                                 FrameIndex,
								 _eyeRenderPose[0].Orientation.x,
								 _eyeRenderPose[0].Orientation.y,
								 _eyeRenderPose[0].Orientation.z,
								 _eyeRenderPose[0].Orientation.w,
								 _eyeRenderPose[0].Position.x,
								 _eyeRenderPose[0].Position.y,
								 _eyeRenderPose[0].Position.z,
								 _eyeRenderPose[1].Orientation.x,
								 _eyeRenderPose[1].Orientation.y,
								 _eyeRenderPose[1].Orientation.z,
								 _eyeRenderPose[1].Orientation.w,
								 _eyeRenderPose[1].Position.x,
								 _eyeRenderPose[1].Position.y,
								 _eyeRenderPose[1].Position.z,
								 _hmdState.HeadPose.ThePose.Orientation.x,   
								 _hmdState.HeadPose.ThePose.Orientation.y,  
								 _hmdState.HeadPose.ThePose.Orientation.z,   
								 _hmdState.HeadPose.ThePose.Orientation.w,   
								 _hmdState.HeadPose.ThePose.Position.x,      
								 _hmdState.HeadPose.ThePose.Position.y,      
								 _hmdState.HeadPose.ThePose.Position.z,      
								 _hmdState.HeadPose.AngularVelocity.x,    
								 _hmdState.HeadPose.AngularVelocity.y,    
								 _hmdState.HeadPose.AngularVelocity.z,    
								 _hmdState.HeadPose.LinearVelocity.x,     
								 _hmdState.HeadPose.LinearVelocity.y,     
								 _hmdState.HeadPose.LinearVelocity.z,     
								 _hmdState.HeadPose.AngularAcceleration.x,
								 _hmdState.HeadPose.AngularAcceleration.y,
								 _hmdState.HeadPose.AngularAcceleration.z,
								 _hmdState.HeadPose.LinearAcceleration.x, 
								 _hmdState.HeadPose.LinearAcceleration.y, 
								 _hmdState.HeadPose.LinearAcceleration.z, 
								 _hmdState.HeadPose.TimeInSeconds,        
								 _hmdState.RawSensorData.Temperature,
								 _hmdState.StatusFlags,
								 _hmdState.CameraPose.Orientation.x,   
								 _hmdState.CameraPose.Orientation.y,  
								 _hmdState.CameraPose.Orientation.z,   
								 _hmdState.CameraPose.Orientation.w,   
								 _hmdState.CameraPose.Position.x,      
								 _hmdState.CameraPose.Position.y,      
								 _hmdState.CameraPose.Position.z,
								 _hmdState.LeveledCameraPose.Orientation.x,   
								 _hmdState.LeveledCameraPose.Orientation.y,  
								 _hmdState.LeveledCameraPose.Orientation.z,   
								 _hmdState.LeveledCameraPose.Orientation.w,   
								 _hmdState.LeveledCameraPose.Position.x,      
								 _hmdState.LeveledCameraPose.Position.y,      
								 _hmdState.LeveledCameraPose.Position.z,
								 _hmdState.HandPoses[0].ThePose.Orientation.x,   
								 _hmdState.HandPoses[0].ThePose.Orientation.y,  
								 _hmdState.HandPoses[0].ThePose.Orientation.z,   
								 _hmdState.HandPoses[0].ThePose.Orientation.w,   
								 _hmdState.HandPoses[0].ThePose.Position.x,      
								 _hmdState.HandPoses[0].ThePose.Position.y,      
								 _hmdState.HandPoses[0].ThePose.Position.z,
								 _hmdState.HandStatusFlags[0],
								 _hmdState.HandPoses[1].ThePose.Orientation.x,   
								 _hmdState.HandPoses[1].ThePose.Orientation.y,  
								 _hmdState.HandPoses[1].ThePose.Orientation.z,   
								 _hmdState.HandPoses[1].ThePose.Orientation.w,   
								 _hmdState.HandPoses[1].ThePose.Position.x,      
								 _hmdState.HandPoses[1].ThePose.Position.y,      
								 _hmdState.HandPoses[1].ThePose.Position.z,
								 _hmdState.HandStatusFlags[1],
								 _hmdState.LastCameraFrameCounter,
								 PredictedDisplayTime);
	
	return jfullposestate;
}

JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1getMatrix4fProjection(
    JNIEnv *env, 
    jobject, 
    jfloat EyeFovPortUpTan,
    jfloat EyeFovPortDownTan,
    jfloat EyeFovPortLeftTan,
    jfloat EyeFovPortRightTan,
    jfloat nearClip,
    jfloat farClip)
{
    if (!_initialised)
        return 0;

    // TODO: We're just using one eye projection, NEEDS SUPPORT FOR BOTH
    vr::Hmd_Eye nEye = vr::Eye_Left;
    vr::HmdMatrix44_t mat = _pHmdSession->GetProjectionMatrix(nEye, nearClip, farClip, vr::API_OpenGL);

    ClearException(env);
    jobject jproj = env->NewObject(matrix4f_Class, matrix4f_constructor_MethodID,
                                   mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
                                   mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		                           mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		                           mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
                                   );
	if (jproj == 0) PrintNewObjectException(env, "Matrix4f");

    return jproj;
}

JNIEXPORT jobject JNICALL Java_com_valvesoftware_openvr_OpenVR__1submitFrame(
	JNIEnv *env,
	jobject)
{
    if (!_initialised)
        return 0;

    ovrViewScaleDesc viewScaleDesc;
    viewScaleDesc.HmdSpaceToWorldScaleInMeters = _worldScale;   
    viewScaleDesc.HmdToEyeViewOffset[0] = _EyeRenderDesc[0].HmdToEyeViewOffset;
    viewScaleDesc.HmdToEyeViewOffset[1] = _EyeRenderDesc[1].HmdToEyeViewOffset;
  
    ovrLayer_Union EyeLayer;
	ovrGLTexture* pDepthTexture[2];
	pDepthTexture[0] = (ovrGLTexture*)&_DepthTextureSet[0].Textures[0];
	pDepthTexture[1] = (ovrGLTexture*)&_DepthTextureSet[1].Textures[0];
	bool HasDepth = pDepthTexture[0]->OGL.TexId == -1 ? false : true;

    EyeLayer.Header.Type  = HasDepth == true ? ovrLayerType_EyeFovDepth : ovrLayerType_EyeFov;
    EyeLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
    
    for (int eye = 0; eye < 2; ++eye)
    {
        EyeLayer.EyeFov.ColorTexture[eye] = _pRenderTextureSet[eye];
        EyeLayer.EyeFov.Fov[eye]          = _hmdDesc.DefaultEyeFov[eye];
        EyeLayer.EyeFov.RenderPose[eye]   = _eyeRenderPose[eye];
        EyeLayer.EyeFov.SensorSampleTime  = _sensorSampleTime;
		EyeLayer.EyeFov.Viewport[eye]     = Recti(0,0,_RenderTextureSize[eye].w,_RenderTextureSize[eye].h);

		if (HasDepth)
		{
			EyeLayer.EyeFovDepth.DepthTexture[eye] = &_DepthTextureSet[eye];
            EyeLayer.EyeFovDepth.ProjectionDesc    = _PosTimewarpProjectionDesc;
		}
    }

    ovrLayerHeader* layers = &EyeLayer.Header;
    ovrResult ovr_result = ovr_SubmitFrame(_pHmdSession, 0, &viewScaleDesc, &layers, 1);
	if (OVR_FAILURE(ovr_result))
	{
		_SetErrorInfo(env, "Failed to submit frame!", ovr_result);
	}
	else
	{
		_SetErrorInfo(env, "Submitted frame successfully!", ovr_result);
	}
	
	return GetLastErrorInfo(env);
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
                         renderTextureInfo_Class,
                         "de/fruitfly/ovr/structs/RenderTextureInfo",
                         renderTextureInfo_constructor_MethodID,
                         "(IIIIIIIIF)V"))
    {
        Success = false;
    }


    if (!LookupJNIConstructorGlobal(env,
                         matrix4f_Class,
                         "de/fruitfly/ovr/structs/Matrix4f",
                         matrix4f_constructor_MethodID,
                         "(FFFFFFFFFFFFFFFF)V"))
    {
        Success = false;
    }

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

//
// With thanks to https://github.com/phr00t/jMonkeyVR/
//
void ConvertMat34ToPose(const vr::HmdMatrix34_t& mat34, Pose& pose)
{
	// Convert to standard 4x4 matrix
	vr::HmdMatrix44_t mat4;
	ConvertMat34to44(mat34, mat4);

	// Get orientation
	QuatfromRotationMatrix(mat4, pose.orient);
	// flip the pitch
	pose.orient.x = -pose.orient.x;
	pose.orient.z = -pose.orient.z;

	// Get position
	pose.pos.v[0] = -mat4.m[0][3];  // -x
	pose.pos.v[1] =  mat4.m[1][3];  //  y
	pose.pos.v[2] = -mat4.m[2][3];  // -z
}

void ConvertMat34to44(const vr::HmdMatrix34_t& mat34, vr::HmdMatrix44_t& mat4)
{
	mat4.m[0][0] = mat34.m[0][0]; mat4.m[1][0] = mat34.m[1][0]; mat4.m[2][0] = mat34.m[2][0]; mat4.m[3][0] = 0.0;
	mat4.m[0][1] = mat34.m[0][1]; mat4.m[1][1] = mat34.m[1][1]; mat4.m[2][1] = mat34.m[2][1]; mat4.m[3][1] = 0.0;
	mat4.m[0][2] = mat34.m[0][2]; mat4.m[1][2] = mat34.m[1][2]; mat4.m[2][2] = mat34.m[2][2]; mat4.m[3][2] = 0.0;
	mat4.m[0][3] = mat34.m[0][3]; mat4.m[1][3] = mat34.m[1][3]; mat4.m[2][3] = mat34.m[2][3]; mat4.m[3][4] = 1.0;
}

//
// With thanks to https://github.com/jMonkeyEngine/jmonkeyengine and
//                https://github.com/phr00t/jMonkeyVR/
//
void QuatfromRotationMatrix(const vr::HmdMatrix44_t& mat4, vr::HmdQuaternion_t& orientation)
{
	float m00 = mat4.m[0][0];
	float m01 = mat4.m[0][1]; 
	float m02 = mat4.m[0][2];
    float m10 = mat4.m[1][0];
	float m11 = mat4.m[1][1];
	float m12 = mat4.m[1][2]; 
	float m20 = mat4.m[2][0]; 
	float m21 = mat4.m[2][1]; 
	float m22 = mat4.m[2][2];

    // first normalize the forward (F), up (U) and side (S) vectors of the rotation matrix
    // so that the scale does not affect the rotation
    float lengthSquared = m00 * m00 + m10 * m10 + m20 * m20;
    if (lengthSquared != 1 && lengthSquared != 0)
	{
        lengthSquared = 1.0f / sqrt(lengthSquared);
        m00 *= lengthSquared;
        m10 *= lengthSquared;
        m20 *= lengthSquared;
    }
    lengthSquared = m01 * m01 + m11 * m11 + m21 * m21;
    if (lengthSquared != 1 && lengthSquared != 0) {
        lengthSquared = 1.0f / sqrt(lengthSquared);
        m01 *= lengthSquared;
        m11 *= lengthSquared;
        m21 *= lengthSquared;
    }
    lengthSquared = m02 * m02 + m12 * m12 + m22 * m22;
    if (lengthSquared != 1 && lengthSquared != 0) {
        lengthSquared = 1.0f / sqrt(lengthSquared);
        m02 *= lengthSquared;
        m12 *= lengthSquared;
        m22 *= lengthSquared;
    }

    // Use the Graphics Gems code, from 
    // ftp://ftp.cis.upenn.edu/pub/graphics/shoemake/quatut.ps.Z
    // *NOT* the "Matrix and Quaternions FAQ", which has errors!

    // the trace is the sum of the diagonal elements; see
    // http://mathworld.wolfram.com/MatrixTrace.html
    float t = m00 + m11 + m22;

    // we protect the division by s by ensuring that s>=1
    if (t >= 0) // |w| >= .5
	{ 
        float s = sqrt(t + 1); // |s|>=1 ...
        orientation.w = 0.5f * s;
        s = 0.5f / s;                 // so this division isn't bad
        orientation.x = (m21 - m12) * s;
        orientation.y = (m02 - m20) * s;
        orientation.z = (m10 - m01) * s;
    } 
	else if ((m00 > m11) && (m00 > m22))
	{
        float s = sqrt(1.0f + m00 - m11 - m22); // |s|>=1
        orientation.x = s * 0.5f; // |x| >= .5
        s = 0.5f / s;
        orientation.y = (m10 + m01) * s;
        orientation.z = (m02 + m20) * s;
        orientation.w = (m21 - m12) * s;
    } 
	else if (m11 > m22)
	{
        float s = sqrt(1.0f + m11 - m00 - m22); // |s|>=1
        orientation.y = s * 0.5f; // |y| >= .5
        s = 0.5f / s;
        orientation.x = (m10 + m01) * s;
        orientation.z = (m21 + m12) * s;
        orientation.w = (m02 - m20) * s;
    } 
	else
	{
        float s = sqrt(1.0f + m22 - m00 - m11); // |s|>=1
        orientation.z = s * 0.5f; // |z| >= .5
        s = 0.5f / s;
        orientation.x = (m02 + m20) * s;
        orientation.y = (m21 + m12) * s;
        orientation.w = (m10 - m01) * s;
    }
}

//
// With thanks to https://github.com/jMonkeyEngine/jmonkeyengine
//
void QuatSlerp(vr::HmdQuaternion_t q1, vr::HmdQuaternion_t q2, float t, vr::HmdQuaternion_t& quatresult) 
{
    // Create a local quaternion to store the interpolated quaternion
    if (q1.x == q2.x && q1.y == q2.y && q1.z == q2.z && q1.w == q2.w) 
	{
		quatresult.x = q1.x;
		quatresult.y = q1.y;
		quatresult.z = q1.z;
		quatresult.w = q1.w;
        return;
    }

    float result = (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z)
            + (q1.w * q2.w);

    if (result < 0) 
	{
        // Negate the second quaternion and the result of the dot product
        q2.x = -q2.x;
        q2.y = -q2.y;
        q2.z = -q2.z;
        q2.w = -q2.w;
        result = -result;
    }

    // Set the first and second scale for the interpolation
    float scale0 = 1 - t;
    float scale1 = t;

    // Check if the angle between the 2 quaternions was big enough to
    // warrant such calculations
    if ((1 - result) > 0.1f) 
	{
		// Get the angle between the 2 quaternions,
        // and then store the sin() of that angle
        float theta = acos(result);
        float invSinTheta = 1 / sin(theta);

        // Calculate the scale for q1 and q2, according to the angle and
        // it's sine value
        scale0 = sin((1 - t) * theta) * invSinTheta;
        scale1 = sin((t * theta)) * invSinTheta;
    }

    // Calculate the x, y, z and w values for the quaternion by using a
    // special form of linear interpolation for quaternions.
    quatresult.x = (scale0 * q1.x) + (scale1 * q2.x);
    quatresult.y = (scale0 * q1.y) + (scale1 * q2.y);
    quatresult.z = (scale0 * q1.z) + (scale1 * q2.z);
    quatresult.w = (scale0 * q1.w) + (scale1 * q2.w);
}

// 
// Thanks to: https://github.com/ValveSoftware/openvr/blob/master/samples/hellovr_opengl/hellovr_opengl_main.cpp
//
//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}
