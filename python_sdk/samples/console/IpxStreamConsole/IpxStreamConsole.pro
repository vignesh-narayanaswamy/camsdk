TARGET = IpxStreamConsole
CONFIG += debug_and_release
CONFIG += c++11 console
CONFIG -= app_bundle
QT -= gui

# if user did not specified arch set it as host
isEmpty(QMAKE_TARGET.arch) {
    QMAKE_TARGET.arch = $$QMAKE_HOST.arch
}

# suffix for GenICam libraries
sfx = _gcc421_v3_0

# define architecture name
contains(QMAKE_TARGET.arch, x86){
    SYS_ARCH = 32_i86
} else {
    contains(QMAKE_TARGET.arch, x86_64) {
        SYS_ARCH = 64_x64
    } else {
        contains(QMAKE_TARGET.arch, arm) {
            SYS_ARCH = 32_ARM
            sfx = _gcc46_v3_0
        } else{
            contains(QMAKE_TARGET.arch, aarch64) {
                SYS_ARCH = 64_ARM
                sfx = _gcc48_v3_0
            } else {
                error(Unknown system architecture: $$QMAKE_TARGET.arch!)
            }
        }
    }
}

# construct correct configuration name
win32 {
    SYS_NAME = Win
}
unix:!macx {
    SYS_NAME = Linux
}
macx {
    SYS_NAME = Maci
    sfx = _clang61_v3_0
}

# define main root directory
IPX_ROOT_DIR = $$PWD/../../..

# define directory for configuration
IPX_BIN_DIR = bin
IPX_LIB_DIR = lib
#CONFIG(debug, debug|release) {
#    IPX_BIN_DIR = bin_dbg
#    IPX_LIB_DIR = lib_dbg
#}

# if you want to move samples away from main sdk,
# make sure to set correct path and names to
# IPX_CAM_SDK_BUILD_MODE and IPX_CAM_SDK_LIB variables
IPX_CAM_SDK_BUILD_MODE = $$SYS_NAME$$SYS_ARCH
IPX_CAM_SDK_LIB = $${IPX_ROOT_DIR}/$${IPX_LIB_DIR}/$${IPX_CAM_SDK_BUILD_MODE}

SOURCES +=     IpxStreamSample.cpp
INCLUDEPATH += $${IPX_ROOT_DIR}/inc

unix {
    QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-write-strings -Wno-unknown-pragmas \
        -Wno-unused-function -Wno-reorder -Wno-unused-result -Wno-deprecated-declarations
    QMAKE_CXXFLAGS += -std=c++11

    macx {
        DEFINES += APPLE
        QMAKE_LFLAGS += "-Wl,-rpath,@executable_path"
        QMAKE_LFLAGS += "-Wl,-rpath,$${IPX_CAM_SDK_LIB}"
    } else {
        DEFINES += LINUX
        QMAKE_LFLAGS += "-Wl,-rpath,\\\$$ORIGIN:$${IPX_CAM_SDK_LIB}"
    }

    LIBS += -L"$${IPX_CAM_SDK_LIB}" -lIpxCameraApi
    LIBS += -L"$${IPX_CAM_SDK_LIB}/genicam/bin/$${IPX_CAM_SDK_BUILD_MODE}" -lGCBase$$sfx  -lGenApi$$sfx  -llog4cpp$$sfx \
        -lLog$$sfx  -lMathParser$$sfx -lNodeMapData$$sfx -lXmlParser$$sfx
}
win32 {
    # so not to show warning C4100: 'value': unreferenced formal parameter
    QMAKE_CXXFLAGS_WARN_ON -= -w34100
    DEFINES += UNICODE _WIN32 WIN64
    LIBS += -L"$${IPX_CAM_SDK_LIB}" -lIpxCameraApi
    DESTDIR = $${IPX_ROOT_DIR}/$${IPX_BIN_DIR}/$${IPX_CAM_SDK_BUILD_MODE}
    warning("So not to copy SDK binaries, DESTDIR is set to $$DESTDIR")
    warning("Before building $$TARGET please make sure it works for you")
}
