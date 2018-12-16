#!/bin/bash

if [ -z "$1" ]; then
    echo "Please pass an IP address or domain name as the first parameter."
    exit 1
fi

OTA_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
PUBLISH_DIR="${OTA_DIR}/../publish"
TLS_DIR="${OTA_DIR}/../tls"

mkdir -p ${PUBLISH_DIR}
mkdir -p ${TLS_DIR}

if [ ! -e ${PUBLISH_DIR}/ledrx.bin ]; then
    ln -s ${OTA_DIR}/../build/ledrx.bin ${PUBLISH_DIR}/ledrx.bin
fi

if [ ! -e ${TLS_DIR}/ca_key.pem ]; then
    openssl genrsa -out ${TLS_DIR}/ca_key.pem 2048
fi

if [ ! -e ${TLS_DIR}/ca_cert.pem ]; then
    openssl req -new -x509 -key ${TLS_DIR}/ca_key.pem -out ${TLS_DIR}/ca_cert.pem -days 36500 -subj "/C=GB/ST=Hampshire/L=Southampton/O=Matt Everett Software/CN=$1"
fi
