cmake_minimum_required(VERSION 2.8)

project(JOpenVRLibrary)

find_package(JNI REQUIRED)
 
include_directories(Include ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(Include ${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers)
include_directories(Include ${JNI_INCLUDE_DIRS})
link_directories(Include ${CMAKE_BINARY_DIR}/output)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(BITS "64")
    set(ARCH "64")
else()
    set(BITS "")
    set(ARCH "32")
endif()

if(WIN32)
    link_directories(Include ${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/win${ARCH})
elseif(APPLE)
    link_directories(Include ${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/osx32)
else()
    link_directories(Include ${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/linux${ARCH})
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ftree-vectorize")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftree-vectorize")
    endif()
	set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -msse2 ${CMAKE_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "-O2 -msse2 ${CMAKE_C_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-O1,--sort-common,--as-needed,-z,relro")
endif()

include(CheckCXXCompilerFlag)
macro(check_and_add_flag var flag)
    CHECK_CXX_COMPILER_FLAG(${flag} FLAG_${var})
    if(FLAG_${var})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
endmacro()

option(ENABLE_LTO "Build with Link-Time Optimization" FALSE)
    if(ENABLE_LTO)
    check_and_add_flag(LTO -flto)
    if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_AR gcc-ar)
    set(CMAKE_RANLIB gcc-ranlib)
    endif()
endif()

if(WIN32)
    set ( NATIVES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/natives/windows/${ARCH} )
    set ( SHARED_LIB_SRC_LOC ${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/win${ARCH}/openvr_api.dll )
elseif(APPLE)
    set ( NATIVES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/natives/osx/${ARCH} )
    set ( SHARED_LIB_SRC_LOC ${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/osx32/libopenvr_api.dylib )
else()
    set ( NATIVES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/natives/linux/${ARCH} )
    set ( SHARED_LIB_SRC_LOC ${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/linux${ARCH}/libopenvr_api.so )

endif()

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${NATIVES_DIR} )
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${NATIVES_DIR} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set( JOPENVR_DYNLIB_NAME JOpenVRLibrary${BITS} )
add_library(${JOPENVR_DYNLIB_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/JOpenVRLibrary.cpp)
target_link_libraries (${JOPENVR_DYNLIB_NAME} openvr_api)

file(COPY ${SHARED_LIB_SRC_LOC} DESTINATION ${NATIVES_DIR})


