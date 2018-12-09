#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
BUILD_VERSION_FILE=$(realpath ${DIR}/../buildversion)
echo $(($(cat ${BUILD_VERSION_FILE}) + 1)) > ${BUILD_VERSION_FILE}
