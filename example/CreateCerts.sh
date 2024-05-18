#!/bin/bash

# Create a self-signed certificate for the localhost
# Structure:
#   keys/
#       ca/
#           ca_key.pem
#           ca_cert.pem
#       server/
#           server_key.pem
#           server_cert.pem
#       client/
#           client_key.pem
#           client_cert.pem

# Get directory of this file
currentDir=$(dirname $(readlink -f $0))

# Get localization
C=$(curl -s http://ipinfo.io/json | grep -Po '"country":\s*"\K[^"]*')
ST=$(curl -s http://ipinfo.io/json | grep -Po '"region":\s*"\K[^"]*')
L=$(curl -s http://ipinfo.io/json | grep -Po '"city":\s*"\K[^"]*')

# Remove all old cert files
rm -r ${currentDir}/keys/

# Create folder structure
mkdir ${currentDir}/keys/
mkdir ${currentDir}/keys/ca/
mkdir ${currentDir}/keys/server/
mkdir ${currentDir}/keys/client/

# Create self-signed CA certificate and key
openssl ecparam -name secp521r1 -genkey -noout -out ${currentDir}/keys/ca/ca_key.pem
openssl req -new -x509 -days 365 -sha512 -utf8 \
    -key ${currentDir}/keys/ca/ca_key.pem \
    -out ${currentDir}/keys/ca/ca_cert.pem \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Example/OU=Example.Authority/CN=localhost"

# Create server private key and certificate signing request and sign it with the CA
openssl ecparam -name secp521r1 -genkey -noout -out ${currentDir}/keys/server/server_key.pem
openssl req -new -sha512 -utf8 \
    -key ${currentDir}/keys/server/server_key.pem \
    -out ${currentDir}/keys/server/server_req.csr \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Example/OU=Example.Server/CN=localhost"
openssl x509 -req -days 365 \
    -in ${currentDir}/keys/server/server_req.csr \
    -CA ${currentDir}/keys/ca/ca_cert.pem \
    -CAkey ${currentDir}/keys/ca/ca_key.pem \
    -CAcreateserial \
    -out ${currentDir}/keys/server/server_cert.pem

# Create client private key and certificate signing request and sign it with the CA
openssl ecparam -name secp521r1 -genkey -noout -out ${currentDir}/keys/client/client_key.pem
openssl req -new -sha512 -utf8 \
    -key ${currentDir}/keys/client/client_key.pem \
    -out ${currentDir}/keys/client/client_req.csr \
    -subj "/C=${C}/ST=${ST}/L=${L}/O=Example/OU=Example.Client/CN=localhost"
openssl x509 -req -days 365 \
    -in ${currentDir}/keys/client/client_req.csr \
    -CA ${currentDir}/keys/ca/ca_cert.pem \
    -CAkey ${currentDir}/keys/ca/ca_key.pem \
    -CAcreateserial \
    -out ${currentDir}/keys/client/client_cert.pem
