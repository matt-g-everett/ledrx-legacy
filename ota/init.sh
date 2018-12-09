#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

ln -s ../build/ledrx.bin ${DIR}/ledrx.bin
ln -s ../version ${DIR}/version
openssl genrsa -out ca_key.pem 2048
openssl req -new -x509 -key ca_key.pem -out ca_cert.pem -days 36500 -subj "/C=GB/ST=Hampshire/L=Southampton/O=Matt Everett Software/CN=$1"
