#!/bin/bash

# =================================================================================================
# Good certificates
# =================================================================================================

# Recreate directories
rm -r keys/
mkdir -p keys/ca
mkdir -p keys/server
mkdir -p keys/client

# Create CA key and certificate
openssl ecparam -genkey -name prime256v1 -out keys/ca/ca.key
openssl req -new -x509 -days 365 -key keys/ca/ca.key -out keys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create server key and certificate signed by CA
openssl ecparam -genkey -name prime256v1 -out keys/server/server.key
openssl req -new -key keys/server/server.key -out keys/server/server.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=server/CN=localhost"
openssl x509 -req -days 365 -in keys/server/server.csr -CA keys/ca/ca.crt -CAkey keys/ca/ca.key -CAcreateserial -out keys/server/server.crt

# Create client key and certificate signed by CA
openssl ecparam -genkey -name prime256v1 -out keys/client/client.key
openssl req -new -key keys/client/client.key -out keys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in keys/client/client.csr -CA keys/ca/ca.crt -CAkey keys/ca/ca.key -CAcreateserial -out keys/client/client.crt

# =================================================================================================
# Fake certificates
# =================================================================================================

# Recreate directories
rm -r fakeKeys/
mkdir -p fakeKeys/ca
mkdir -p fakeKeys/server
mkdir -p fakeKeys/client

# Create CA key and certificate
echo "This is a fake CA key" > fakeKeys/ca/ca.key
echo "This is a fake CA certificate" > fakeKeys/ca/ca.crt

# Create server key and certificate
echo "This is a fake server key" > fakeKeys/server/server.key
echo "This is a fake server certificate request" > fakeKeys/server/server.csr
echo "This is a fake server certificate" > fakeKeys/server/server.crt

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
mkdir -p selfSignedKeys/server
mkdir -p selfSignedKeys/client

# Create CA key and certificate
openssl ecparam -genkey -name prime256v1 -out selfSignedKeys/ca/ca.key
openssl req -new -x509 -days 365 -key selfSignedKeys/ca/ca.key -out selfSignedKeys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create server key and certificate signed by self
openssl ecparam -genkey -name prime256v1 -out selfSignedKeys/server/server.key
openssl req -new -x509 -days 365 -key selfSignedKeys/server/server.key -out selfSignedKeys/server/server.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=server/CN=localhost"

# Create client key and certificate signed by self
openssl ecparam -genkey -name prime256v1 -out selfSignedKeys/client/client.key
openssl req -new -x509 -days 365 -key selfSignedKeys/client/client.key -out selfSignedKeys/client/client.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"

# =================================================================================================
# Second certificates signed by other CA
# =================================================================================================

# Recreate directories
rm -r secondKeys/
mkdir -p secondKeys/ca
mkdir -p secondKeys/server
mkdir -p secondKeys/client

# Create CA key and certificate
openssl ecparam -genkey -name prime256v1 -out secondKeys/ca/ca.key
openssl req -new -x509 -days 365 -key secondKeys/ca/ca.key -out secondKeys/ca/ca.crt -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=CA/CN=localhost"

# Create server key and certificate signed by second CA
openssl ecparam -genkey -name prime256v1 -out secondKeys/server/server.key
openssl req -new -key secondKeys/server/server.key -out secondKeys/server/server.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=server/CN=localhost"
openssl x509 -req -days 365 -in secondKeys/server/server.csr -CA secondKeys/ca/ca.crt -CAkey secondKeys/ca/ca.key -CAcreateserial -out secondKeys/server/server.crt

# Create client key and certificate signed by second CA
openssl ecparam -genkey -name prime256v1 -out secondKeys/client/client.key
openssl req -new -key secondKeys/client/client.key -out secondKeys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in secondKeys/client/client.csr -CA secondKeys/ca/ca.crt -CAkey secondKeys/ca/ca.key -CAcreateserial -out secondKeys/client/client.crt

# =================================================================================================
# Certificates signed by server cert signed by CA (cert chain depth 2)
# =================================================================================================

# Recreate directories
rm -r certChainDepth2Keys/
mkdir -p certChainDepth2Keys/ca
mkdir -p certChainDepth2Keys/server
mkdir -p certChainDepth2Keys/client

# Create CA key and certificate (CA in this case is the server)
cp keys/server/server.key certChainDepth2Keys/ca/ca.key
cp keys/server/server.crt certChainDepth2Keys/ca/ca.crt

# Create server key and certificate signed by server cert (server in this case is the CA)
openssl ecparam -genkey -name prime256v1 -out certChainDepth2Keys/server/server.key
openssl req -new -key certChainDepth2Keys/server/server.key -out certChainDepth2Keys/server/server.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=server/CN=localhost"
openssl x509 -req -days 365 -in certChainDepth2Keys/server/server.csr -CA certChainDepth2Keys/ca/ca.crt -CAkey certChainDepth2Keys/ca/ca.key -CAcreateserial -out certChainDepth2Keys/server/server.crt

# Create client key and certificate signed by server cert (server in this case is the CA)
openssl ecparam -genkey -name prime256v1 -out certChainDepth2Keys/client/client.key
openssl req -new -key certChainDepth2Keys/client/client.key -out certChainDepth2Keys/client/client.csr -subj "/C=DE/ST=<my state>/L=<my city>/O=NetworkTester/OU=Client/CN=localhost"
openssl x509 -req -days 365 -in certChainDepth2Keys/client/client.csr -CA certChainDepth2Keys/ca/ca.crt -CAkey certChainDepth2Keys/ca/ca.key -CAcreateserial -out certChainDepth2Keys/client/client.crt
