LOCAL_PATH := $(call my-dir)
# Recursive wildcards, as per: https://stackoverflow.com/a/18258352/9110986
rwildcard   = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

include $(CLEAR_VARS)

LOCAL_MODULE     := orbit_core
LOCAL_SRC_FILES  := $(wildcard $(LOCAL_PATH)/../../../../src/orbit/core/*.cpp) \
                    $(wildcard $(LOCAL_PATH)/../../../../src/orbit/core/internal/*_android.cpp)
LOCAL_CPPFLAGS   := -std=c++1z -fexceptions -Wall -DORB_BUILD
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../src/ \
                    $(NDK_ROOT)/sources/android/native_app_glue/

include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE           := orbit_graphics
LOCAL_SRC_FILES        := $(wildcard $(LOCAL_PATH)/../../../../src/orbit/graphics/*.cpp) \
                          $(wildcard $(LOCAL_PATH)/../../../../src/orbit/graphics/internal/*_egl.cpp)
LOCAL_CPPFLAGS         := -std=c++1z -fexceptions -Wall -DORB_BUILD
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../../../../src/ \
                          $(NDK_ROOT)/sources/android/native_app_glue/
LOCAL_STATIC_LIBRARIES := orbit_core

include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE           := orb01
LOCAL_SRC_FILES        := $(call rwildcard, $(LOCAL_PATH)/../../../../src/samples/01/, *.cpp)
LOCAL_CPPFLAGS         := -std=c++1z -fexceptions -Wall -DORB_BUILD
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../../../../src/
LOCAL_LDLIBS           := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue orbit_core orbit_graphics

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
