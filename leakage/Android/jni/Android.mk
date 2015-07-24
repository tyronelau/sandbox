LOCAL_PATH := $(call my-dir)
JNI_PATH :=$(LOCAL_PATH)

include $(CLEAR_VARS)
LOCAL_CFLAGS    += -g -std=c++0x
LOCAL_CFLAGS    += -ggdb
LOCAL_CFLAGS    += -O3

LOCAL_CXXFLAGS    += -g -std=c++0x
LOCAL_CXXFLAGS    += -ggdb
LOCAL_CXXFLAGS    += -O3

LOCAL_CPPFLAGS    += -g -std=c++0x
LOCAL_CPPFLAGS    += -ggdb
LOCAL_CPPFLAGS    += -O3

LOCAL_C_INCLUDES := ${LOCAL_PATH}/../
LOCAL_SRC_FILES := \
	../../backtrace.cpp \
	../../CityHash.cpp \
	../../dllmain.cpp \
	../../callstack_android.cpp \
	../../guard_thread.cpp

LOCAL_LDLIBS := -ldl # -lpthread

LOCAL_MODULE := debug
include $(BUILD_SHARED_LIBRARY)

