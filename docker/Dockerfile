FROM ubuntu:20.04
# FROM docker.io/eyalamirmusic/juce_dev_machine:latest AS base
FROM linuxbrew/brew:latest

USER root

RUN brew install libsndfile portaudio ncurses opus

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc g++ lldb \
    tzdata \
    git \
    zip \
    ca-certificates \ 
    build-essential \
    cmake \
    g++ \
    libgtk-3-dev \
    libfreetype6-dev \
    libx11-dev \
    libxinerama-dev \
    libxrandr-dev \
    libxcursor-dev \
    mesa-common-dev \
    libasound2-dev \
    freeglut3-dev \
    libxcomposite-dev \
    libcurl4-openssl-dev \
    libwebkit2gtk-4.0-dev \
    ladspa-sdk

COPY ["resources/modules", "/usr/lib/JUCE/modules"]

ENTRYPOINT	[ "bash" ]