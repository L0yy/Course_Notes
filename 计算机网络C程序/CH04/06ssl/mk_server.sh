#!/bin/sh
gcc -g -o ssl_server ssl_server.c -lssl -lcrypto -ldl
