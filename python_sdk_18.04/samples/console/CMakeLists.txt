# help function to print environment variables
function(printvar name)
    message("${name}: ${${name}}")
endfunction()

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.3)

# Define the Project's name
project(IpxCameraSDK_Console_Samples)

# Create User-specified Option
set(ARCH "" CACHE STRING "Architecture of the project")
if ("${ARCH}" STREQUAL "")
    set(ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

# determine system name
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(SYS_NAME "Win")
    add_definitions(-D_WIN32)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(SYS_NAME "Linux")
    add_definitions(-DLINUX)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(SYS_NAME "Maci")
    add_definitions(-DAPPLE)
else()
    set(SYS_NAME "${CMAKE_SYSTEM_NAME}")
endif()

if("${ARCH}" STREQUAL "x86")
    set(SYS_ARCH "32_i86")
elseif("${ARCH}" STREQUAL "x86_64")
    set(SYS_ARCH "64_x64")
elseif("${ARCH}" STREQUAL "arm")
    set(SYS_ARCH "32_ARM")
elseif("${ARCH}" STREQUAL "aarch64")
    set(SYS_ARCH "64_ARM")
else()
    message(FATAL_ERROR "Unknown system architecture: ${ARCH}!")
endif()

# if you want to move samples away from main sdk,
# make sure to set correct path and names to
# ARCH_DIR and IPX_CAM_SDK_FIND variables

# set resultant architecture directory name
set(ARCH_DIR "${SYS_NAME}${SYS_ARCH}")

# set path where to search for IpxCameraSDK configuration file
set(IPX_CAM_SDK_FIND "${CMAKE_CURRENT_LIST_DIR}/../../lib/${ARCH_DIR}")

# set path to find IpxCameraSDK libraries by find_package
# variable IPX_LIB_DIR is internal, do not use it.
foreach(search_path "${IPX_CAM_SDK_FIND}" "${IPX_LIB_DIR}")
    if (NOT "${search_path}" STREQUAL "")
        list(FIND CMAKE_PREFIX_PATH "${search_path}" isFound)
        if (${isFound} EQUAL -1)
            list(APPEND CMAKE_PREFIX_PATH "${search_path}")
        endif()
    endif()
endforeach()

find_package(IpxCameraSDK REQUIRED)

# create variable for path to GenICam libraries
# IpxCameraSDK_GenLIB_DIR - defined in the configuration file of IpxCameraSDK
set(GenICam_LIBS "${IpxCameraSDK_GenLIB_DIR}")

# set standart std -std=c++11 or -std=gnu++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# print helpful info
printvar(ARCH_DIR)
printvar(IPX_CAM_SDK_FIND)
printvar(GenICam_LIBS)
printvar(IpxCameraSDK_DIR)
printvar(IpxCameraSDK_INCLUDE_DIR)
printvar(IpxCameraSDK_LIBRARY_DIR)
printvar(IpxCameraSDK_OUTPUT_DIR)

# Console sample
add_subdirectory(IpxStreamConsole)
add_subdirectory(IpxMultiStreamConsole)
add_subdirectory(IpxTriggerStreamWritingConsole)
