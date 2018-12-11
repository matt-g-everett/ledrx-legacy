#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ledrx
EXCLUDE_COMPONENTS := asio aws_iot bt console cxx esp_adc_cal esp_http_server esp_https_ota espcoredump expat fatfs freemodbus idf_test jsmn json mdns mqtt openssl protobuf-c protocomm sdmmc spiffs ulp wear_levelling wifi_provisioning
EXTRA_COMPONENT_DIRS := $(PROJECT_PATH)/components

include $(IDF_PATH)/make/project.mk

.PHONY: ota

ota: all
	cp $(PROJECT_PATH)/version $(PROJECT_PATH)/ota/version
