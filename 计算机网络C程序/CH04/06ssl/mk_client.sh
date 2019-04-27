#!/bin/sh
gcc -g -o ssl_client ssl_client.c -lssl -lcrypto -ldl
