#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ledrx
EXCLUDE_COMPONENTS := asio aws_iot bt console cxx esp_adc_cal esp_http_client esp_http_server esp_https_ota esp-tls espcoredump expat fatfs freemodbus idf_test jsmn json mdns mqtt nghttp openssl protobuf-c protocomm sdmmc spiffs tcp_transport ulp wear_levelling wifi_provisioning
EXTRA_COMPONENT_DIRS := $(PROJECT_PATH)/components

include $(IDF_PATH)/make/project.mk

all: bump_build_version

bump_build_version:
	$(PROJECT_PATH)/tools/bump-build-version.sh
