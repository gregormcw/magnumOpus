#!/bin/bash

# Client
gcc -Wall -g \
	-pthread \
	-o Builds/CLI/udp_client Source/udp_client.c Source/paUtils.c Source/fifo.c\
	-I/home/linuxbrew/.linuxbrew/include\
	-I/home/linuxbrew/.linuxbrew/include/opus\
	-L/home/linuxbrew/.linuxbrew/lib \
	-lopus -lportaudio -lncurses