#!/bin/sh

##################
# Initialization #
##################

# Delete the old files
rm -rf ca
rm -rf newcerts
rm -rf *.key *.csr *.crt *.p12

# Re-create the initial files.
mkdir ca
mkdir newcerts
echo 01 > ca/serial
touch ca/index.txt


###########
# CA part #
###########

# CA private key
openssl genrsa -out ca/ca.key 4096

# CA certificate request
openssl req -new -out ca/ca.csr -key ca/ca.key -config ./openssl.cnf

# CA self-signed certificate
openssl x509 -req -in ca/ca.csr -out ca/ca.crt -signkey ca/ca.key -days 3650

# P12 Formatted certificate
openssl pkcs12 -export -clcerts -in ca/ca.crt -inkey ca/ca.key -out ca/ca.p12


###############
# Server part #
###############

# Server private key
openssl genrsa -out server.key 1024

# Server certificate request
openssl req -new -key server.key -out server.csr -config ./openssl.cnf

# Server certificate signed by CA
openssl ca -in server.csr -out server.crt -cert ca/ca.crt -keyfile ca/ca.key -config ./openssl.cnf -days 1095

# P12 Formatted certificate
openssl pkcs12 -export -clcerts -in server.crt -inkey server.key -out server.p12 
