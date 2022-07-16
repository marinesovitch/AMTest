LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := amtestbackend

LOCAL_C_INCLUDES := ../BackEnd/h ../

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := -fexceptions -ffast-math -O3 -funroll-loops
APP_OPTIM := release
#LOCAL_CFLAGS := -fexceptions -g -O0
#APP_OPTIM := debug

#VisualGDBAndroid: AutoUpdateSourcesInNextLine
LOCAL_SRC_FILES := \
    backendBridge.cpp \
    ../../BackEnd/detail/beBigCoordTypes.cpp \
    ../../BackEnd/detail/beBitmap.cpp \
    ../../BackEnd/detail/beConsts.cpp \
    ../../BackEnd/detail/beContentsGenerator.cpp \
    ../../BackEnd/detail/beController.cpp \
    ../../BackEnd/detail/beControllerImpl.cpp \
    ../../BackEnd/detail/beDiagnostics.cpp \
    ../../BackEnd/detail/beDocument.cpp \
    ../../BackEnd/detail/beDocumentImpl.cpp \
    ../../BackEnd/detail/beInstance.cpp \
    ../../BackEnd/detail/beInternalTypes.cpp \
    ../../BackEnd/detail/beIntervalTree.cpp \
    ../../BackEnd/detail/beMapReader.cpp \
    ../../BackEnd/detail/beMapStream.cpp \
    ../../BackEnd/detail/beRangeTree.cpp \
    ../../BackEnd/detail/beSegmentsManager.cpp \
    ../../BackEnd/detail/beTreeUtils.cpp \
    ../../BackEnd/detail/beTypes.cpp \
    ../../BackEnd/detail/beUtils.cpp \
    ../../BackEnd/detail/beViewportArea.cpp \
    ../../BackEnd/detail/ph.cpp


LOCAL_LDLIBS    := -ljnigraphics -llog

include $(BUILD_SHARED_LIBRARY)
