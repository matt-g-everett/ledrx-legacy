#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
PROTO_DIR="${SCRIPT_DIR}/../../modules/ledapi/protobuf"
PROTO_PATH="${PROTO_DIR}/coapapi.proto"

protoc-c ${PROTO_PATH} --proto_path ${PROTO_DIR} --c_out ${SCRIPT_DIR}

mv ${SCRIPT_DIR}/coapapi.pb-c.h ${SCRIPT_DIR}/include/
