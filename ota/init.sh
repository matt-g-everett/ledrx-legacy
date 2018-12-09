ln -s ../build/ledrx.bin ledrx.bin
openssl genrsa -out ca_key.pem 2048
openssl req -new -x509 -key ca_key.pem -out ca_cert.pem -days 36500 -subj "/C=GB/ST=Hampshire/L=Southampton/O=Matt Everett Software/CN=192.168.1.148"
