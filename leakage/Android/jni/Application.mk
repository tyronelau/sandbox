#NDK_TOOLCHAIN_VERSION = clang3.4
STLPORT_FORCE_REBUILD := false
APP_STL := gnustl_static
APP_CPPFLAGS += -fno-exceptions
APP_CPPFLAGS += -fno-rtti -fPIC -fpic
APP_CPPFLAGS += -fvisibility=hidden
APP_CFLAGS += -fvisibility=hidden
HAVEVIDEO = 1
ENABLE_EAAC_ENC = 0
ENABLE_MP3_DEC = 1
ENABLE_SLES_PLAYER = 0
APP_ABI := armeabi-v7a
APP_PLATFORM := android-9

