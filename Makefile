#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ledrx
EXCLUDE_COMPONENTS := asio aws_iot bt console cxx esp_adc_cal esp_https_ota espcoredump expat fatfs freemodbus idf_test jsmn json mdns mqtt protocomm sdmmc spiffs ulp wear_levelling wifi_provisioning
EXTRA_COMPONENT_DIRS := $(PROJECT_PATH)/components

include $(IDF_PATH)/make/project.mk

.PHONY: ota increment coapapi

increment:
	# Increment the prerelease number automatically
	python ota/inc-version.py

ota: increment all
	# Copy the version into the publish location
	cp $(PROJECT_PATH)/version.txt $(PROJECT_PATH)/publish/version

coapapi:
	$(PROJECT_PATH)/components/coapapi/generate.sh
