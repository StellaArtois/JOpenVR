#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "JOpenVRLibrary.h"
#include "openvr.h"

vr::IVRSystem *m_pHMD = 0;

/* 
Initialises 
   - the LibOVR client -> RT connection
   - the HMD device session
   - gets the HMD parameters
*/
JNIEXPORT jboolean JNICALL Java_openvr__1initSubsystem(JNIEnv *env, jobject jobj)
{
    bool success = true;
    
    // Loading the SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );
    
    if ( eError != vr::VRInitError_None )
    {
        success = false;
    }
    
    if( m_pHMD )
    {
        vr::VR_Shutdown();
        m_pHMD = NULL;
    }
    
    return success;
}
