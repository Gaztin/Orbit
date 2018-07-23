LOCAL_PATH := $(call my-dir)
# Recursive wildcards, as per: https://stackoverflow.com/a/18258352/9110986
rwildcard   = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

include $(CLEAR_VARS)

LOCAL_MODULE     := orbit_core
LOCAL_SRC_FILES  := $(call rwildcard, $(LOCAL_PATH)/../../../../src/orbit/core/, *.cpp)
LOCAL_CPPFLAGS   := -std=c++14
LOCAL_LDLIBS     := -llog -landroid
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../src/

include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE           := orb01
LOCAL_SRC_FILES        := $(call rwildcard, $(LOCAL_PATH)/../../../../src/samples/01/, *.cpp)
LOCAL_CPPFLAGS         := -std=c++14
LOCAL_C_INCLUDES       := $(LOCAL_PATH)/../../../../src/
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_SHARED_LIBRARIES := orbit_core

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
