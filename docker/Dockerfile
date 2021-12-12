FROM ubuntu:20.04
FROM docker.io/eyalamirmusic/juce_dev_machine:latest
FROM linuxbrew/brew:latest

USER root

RUN apt-get update && apt-get install -y gcc g++ lldb

RUN brew install libsndfile portaudio ncurses opus

ENTRYPOINT	[ "bash" ]