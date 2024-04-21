#!/bin/bash

# TODO: Use ecdsa over rsa

# =================================================================================================
# Good certificates
# =================================================================================================

# Recreate directories
rm -r keys/
mkdir -p keys/ca
mkdir -p keys/listener
mkdir -p keys/client

# Create CA key and certificate
openssl genrsa -out keys/ca/ca.key 2048
openssl req -new -x509 -days 365 -key keys/ca/ca.key -out keys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create listener key and certificate signed by CA
openssl genrsa -out keys/listener/listener.key 2048
openssl req -new -key keys/listener/listener.key -out keys/listener/listener.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Listener/CN=localhost"
openssl x509 -req -days 365 -in keys/listener/listener.csr -CA keys/ca/ca.crt -CAkey keys/ca/ca.key -CAcreateserial -out keys/listener/listener.crt

# Create client key and certificate signed by CA
openssl genrsa -out keys/client/client.key 2048
openssl req -new -key keys/client/client.key -out keys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in keys/client/client.csr -CA keys/ca/ca.crt -CAkey keys/ca/ca.key -CAcreateserial -out keys/client/client.crt

# =================================================================================================
# Fake certificates
# =================================================================================================

# Recreate directories
rm -r fakeKeys/
mkdir -p fakeKeys/ca
mkdir -p fakeKeys/listener
mkdir -p fakeKeys/client

# Create CA key and certificate
echo "This is a fake CA key" > fakeKeys/ca/ca.key
echo "This is a fake CA certificate" > fakeKeys/ca/ca.crt

# Create listener key and certificate
echo "This is a fake listener key" > fakeKeys/listener/listener.key
echo "This is a fake listener certificate request" > fakeKeys/listener/listener.csr
echo "This is a fake listener certificate" > fakeKeys/listener/listener.crt

# Create client key and certificate
echo "This is a fake client key" > fakeKeys/client/client.key
echo "This is a fake client certificate request" > fakeKeys/client/client.csr
echo "This is a fake client certificate" > fakeKeys/client/client.crt

# =================================================================================================
# Self signed certificates
# =================================================================================================

# Recreate directories
rm -r selfSignedKeys/
mkdir -p selfSignedKeys/ca
mkdir -p selfSignedKeys/listener
mkdir -p selfSignedKeys/client

# Create CA key and certificate
openssl genrsa -out selfSignedKeys/ca/ca.key 2048
openssl req -new -x509 -days 365 -key selfSignedKeys/ca/ca.key -out selfSignedKeys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create listener key and certificate signed by self
openssl genrsa -out selfSignedKeys/listener/listener.key 2048
openssl req -new -x509 -days 365 -key selfSignedKeys/listener/listener.key -out selfSignedKeys/listener/listener.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Listener/CN=localhost"

# Create client key and certificate signed by self
openssl genrsa -out selfSignedKeys/client/client.key 2048
openssl req -new -x509 -days 365 -key selfSignedKeys/client/client.key -out selfSignedKeys/client/client.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"

# =================================================================================================
# Second certificates signed by other CA
# =================================================================================================

# Recreate directories
rm -r secondKeys/
mkdir -p secondKeys/ca
mkdir -p secondKeys/listener
mkdir -p secondKeys/client

# Create CA key and certificate
openssl genrsa -out secondKeys/ca/ca.key 2048
openssl req -new -x509 -days 365 -key secondKeys/ca/ca.key -out secondKeys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create listener key and certificate signed by second CA
openssl genrsa -out secondKeys/listener/listener.key 2048
openssl req -new -key secondKeys/listener/listener.key -out secondKeys/listener/listener.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Listener/CN=localhost"
openssl x509 -req -days 365 -in secondKeys/listener/listener.csr -CA secondKeys/ca/ca.crt -CAkey secondKeys/ca/ca.key -CAcreateserial -out secondKeys/listener/listener.crt

# Create client key and certificate signed by second CA
openssl genrsa -out secondKeys/client/client.key 2048
openssl req -new -key secondKeys/client/client.key -out secondKeys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in secondKeys/client/client.csr -CA secondKeys/ca/ca.crt -CAkey secondKeys/ca/ca.key -CAcreateserial -out secondKeys/client/client.crt

# =================================================================================================
# Certificates signed by listener cert signed by CA (cert chain depth 2)
# =================================================================================================

# Recreate directories
rm -r certChainDepth2Keys/
mkdir -p certChainDepth2Keys/ca
mkdir -p certChainDepth2Keys/listener
mkdir -p certChainDepth2Keys/client

# Create CA key and certificate (CA in this case is the listener)
cp keys/listener/listener.key certChainDepth2Keys/ca/ca.key
cp keys/listener/listener.crt certChainDepth2Keys/ca/ca.crt

# Create listener key and certificate signed by listener cert (listener in this case is the CA)
openssl genrsa -out certChainDepth2Keys/listener/listener.key 2048
openssl req -new -key certChainDepth2Keys/listener/listener.key -out certChainDepth2Keys/listener/listener.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Listener/CN=localhost"
openssl x509 -req -days 365 -in certChainDepth2Keys/listener/listener.csr -CA certChainDepth2Keys/ca/ca.crt -CAkey certChainDepth2Keys/ca/ca.key -CAcreateserial -out certChainDepth2Keys/listener/listener.crt

# Create client key and certificate signed by listener cert (listener in this case is the CA)
openssl genrsa -out certChainDepth2Keys/client/client.key 2048
openssl req -new -key certChainDepth2Keys/client/client.key -out certChainDepth2Keys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in certChainDepth2Keys/client/client.csr -CA certChainDepth2Keys/ca/ca.crt -CAkey certChainDepth2Keys/ca/ca.key -CAcreateserial -out certChainDepth2Keys/client/client.crt
