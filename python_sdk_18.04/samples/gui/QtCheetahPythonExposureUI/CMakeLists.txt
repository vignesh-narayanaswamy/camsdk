project(QtCheetahPythonExposureUI)

set(HDRS QtCheetahPythonExposureUI.h
    ../common/inc/GrabbingThread.h
    ../common/inc/ipxparammaphelper.h
    )

set(SRCS QtCheetahPythonExposureUI.cpp
    main.cpp
    ../common/src/GrabbingThread.cpp
    )

set(FORMS imperx.ui)

set(RESOURCES ../common/res/qrc/imperx.qrc)
		
set(RESOURCE_IMAGES ../common/res/images/camconfig_basic.png
    ../common/res/images/camconfig_expert.png
    ../common/res/images/center_window.png
    ../common/res/images/connection_USB.png
    ../common/res/images/disconnection_USB.png
    ../common/res/images/full_screen.png
    ../common/res/images/GenICamTree_32.png
    ../common/res/images/play.png
    ../common/res/images/stop.png
    ../common/res/images/zoom_100.png
    ../common/res/images/zoom_fit_window.png
    ../common/res/images/zoom-.png
    ../common/res/images/zoom.png
    ../common/res/images/zoom+.png
    ../common/res/images/zoom_spread_window.png
    ../common/res/icons/imperx.ico
)
		
add_definitions(-DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB)

if (NOT UNIX)
    add_definitions(-D_USRDLL -DUNICODE -D_UNICODE)
endif()

if (NOT CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DQT_NO_DEBUG -DQT_NO_DEBUG_OUTPUT)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(../common/inc)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(OS_FLAGS WIN32)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(OS_FLAGS MACOSX_BUNDLE)
endif()

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

add_executable(${PROJECT_NAME} ${OS_FLAGS} ${HDRS} ${SRCS} ${RESOURCE_IMAGES} ${RESOURCES} ${FORMS})
target_link_libraries(${PROJECT_NAME}
    ${RPATH_LINK_FLAG}
    Threads::Threads
    IpxCameraApi
    IpxCameraGuiApi
    IpxDisplayQt
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    )
