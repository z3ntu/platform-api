LOCAL_PATH := $(call my-dir)

ANDROID_VERSION_MAJOR := $(word 1, $(subst ., , $(PLATFORM_VERSION)))
ANDROID_VERSION_MINOR := $(word 2, $(subst ., , $(PLATFORM_VERSION)))
ANDROID_VERSION_PATCH := $(word 3, $(subst ., , $(PLATFORM_VERSION)))

ifeq ($(ANDROID_VERSION_MINOR),)
    ANDROID_VERSION_MINOR := 0
endif
ifeq ($(ANDROID_VERSION_PATCH),)
    ANDROID_VERSION_PATCH := 0
endif

HAS_LIBINPUTSERVICE := $(shell test $(ANDROID_VERSION_MAJOR) -eq 4 -a $(ANDROID_VERSION_MINOR) -gt 2 && echo true)
IS_ANDROID_8 := $(shell test $(ANDROID_VERSION_MAJOR) -ge 8 && echo true)

include $(CLEAR_VARS)

LOCAL_CFLAGS += \
	-DANDROID_VERSION_MAJOR=$(ANDROID_VERSION_MAJOR) \
	-DANDROID_VERSION_MINOR=$(ANDROID_VERSION_MINOR) \
	-DANDROID_VERSION_PATCH=$(ANDROID_VERSION_PATCH)

UPAPI_PATH := $(LOCAL_PATH)/../../

ifeq ($(IS_ANDROID_8),true)
BOARD_HAS_GNSS_STATUS_CALLBACK := true
endif

ifneq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += -std=gnu++0x
endif

ifeq ($(BOARD_HAS_GNSS_STATUS_CALLBACK),true)
LOCAL_CFLAGS += -DBOARD_HAS_GNSS_STATUS_CALLBACK
endif

ifeq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += \
    -Wno-unused-parameter
endif

LOCAL_C_INCLUDES := \
	$(UPAPI_PATH)/include \
	$(UPAPI_PATH)/android/include

LOCAL_SRC_FILES := \
	ubuntu_application_sensors_for_hybris.cpp \
	../default/default_ubuntu_application_sensor.cpp

ifeq ($(IS_ANDROID_8),true)
ifeq ($(BOARD_HAS_LEGACY_GPS_HAL),true)
LOCAL_SRC_FILES += \
    ubuntu_application_gps_for_hybris.cpp
else
LOCAL_SRC_FILES += \
    ubuntu_application_gps_hidl_for_hybris.cpp
endif
else
LOCAL_SRC_FILES += \
    ubuntu_application_gps_for_hybris.cpp
endif

ifeq ($(IS_ANDROID_8),true)
LOCAL_SRC_FILES += \
    ubuntu_hardware_booster_hidl_for_hybris.cpp
else
LOCAL_SRC_FILES += \
    ubuntu_hardware_booster_for_hybris.cpp
endif

LOCAL_MODULE := libubuntu_application_api
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libandroidfw \
	libbinder \
	libcutils \
	libinput \
	liblog \
	libutils \
	libgui \
	libEGL \
	libGLESv2 \
	libhardware \
	libhardware_legacy \
	libdl

ifeq ($(IS_ANDROID_8),true)
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libhidltransport \
    libsensor \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.power@1.0 \
    android.hardware.power@1.1 \
    android.hardware.power@1.2 \
    android.hardware.power@1.3
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += -std=gnu++0x
endif

LOCAL_C_INCLUDES := \
	$(UPAPI_PATH)/include \
	$(UPAPI_PATH)/android/include

LOCAL_SRC_FILES:= \
	test_sensors_c_api.cpp \

LOCAL_MODULE:= direct_ubuntu_application_sensors_c_api_for_hybris_test
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libandroidfw \
	libutils \
	libEGL \
	libGLESv2 \
	libubuntu_application_api

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

ifneq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += -std=gnu++0x
endif

LOCAL_C_INCLUDES := \
	$(UPAPI_PATH)/include \
	$(UPAPI_PATH)/android/include

LOCAL_SRC_FILES:= \
	test_sensors.cpp \

LOCAL_MODULE:= direct_ubuntu_application_sensors_for_hybris_test
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libandroidfw \
	libutils \
	libEGL \
	libGLESv2 \
	libubuntu_application_api

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

ifneq ($(IS_ANDROID_8),true)
LOCAL_CFLAGS += -std=gnu++0x
endif

LOCAL_C_INCLUDES := \
	$(UPAPI_PATH)/include \
	$(UPAPI_PATH)/android/include

LOCAL_SRC_FILES:= \
	test_gps_api.cpp \

LOCAL_MODULE:= direct_ubuntu_application_gps_c_api_for_hybris_test
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libui \
	libutils \
	libEGL \
	libGLESv2 \
	libhardware \
	libhardware_legacy \
	libubuntu_application_api

#include $(BUILD_EXECUTABLE)
