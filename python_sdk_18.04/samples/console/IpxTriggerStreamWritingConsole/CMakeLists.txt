project(IpxTriggerStreamWritingConsole)

# find threading library on the system, prefer pthread
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# To build an executable linker has to find all dependencies, to specify search path
# -rpath, -rpath-link flags can be used
set(RPATH_LINK_FLAG "-Wl,-rpath-link=${GenICam_LIBS}")
if (APPLE)
    set(RPATH_LINK_FLAG)
endif()

add_executable(${PROJECT_NAME} IpxTriggerStreamWritingConsole.cpp)
target_link_libraries(${PROJECT_NAME}
    ${RPATH_LINK_FLAG}
    Threads::Threads
    IpxCameraApi
    IpxImageSerializer
    IpxImageApi
    IpxDemosaicing
    )
