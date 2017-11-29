LOCAL_PATH := $(call my-dir)
JNI_PATH :=$(LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_CFLAGS    += -g -std=c++0x
LOCAL_CFLAGS    += -ggdb -fno-omit-frame-pointer
LOCAL_CFLAGS    += -O0

LOCAL_CXXFLAGS    += -g -std=c++0x
LOCAL_CXXFLAGS    += -ggdb -fno-omit-frame-pointer
LOCAL_CXXFLAGS    += -O0

LOCAL_CPPFLAGS    += -g -std=c++0x
LOCAL_CPPFLAGS    += -ggdb -fno-omit-frame-pointer
LOCAL_CPPFLAGS    += -O0

LOCAL_C_INCLUDES := ${LOCAL_PATH}/../
LOCAL_SRC_FILES := \
	../../backtrace.cpp \
	../../CityHash.cpp \
	../../dllmain.cpp \
	../../callstack_android.cpp \
	../../guard_thread.cpp \
	../../allocator.cpp

LOCAL_LDLIBS := -ldl

LOCAL_MODULE := debug
include $(BUILD_SHARED_LIBRARY)

