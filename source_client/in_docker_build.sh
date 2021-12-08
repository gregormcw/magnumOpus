#!/bin/bash

# Source Client
gcc -Wall \
	-o build/udp_source_client udp_source_client.c paUtils.c \
	-I/home/linuxbrew/.linuxbrew/include\
	-I/home/linuxbrew/.linuxbrew/include/opus\
	-L/home/linuxbrew/.linuxbrew/lib \
	-lsndfile -lopus -lportaudio -lncurses