FROM ubuntu:20.04
FROM linuxbrew/brew:latest

USER root

RUN apt-get update && apt-get install -y gcc g++ lldb

RUN brew install libsndfile portaudio ncurses opus