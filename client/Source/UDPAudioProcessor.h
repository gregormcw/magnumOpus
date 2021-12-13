/*
  ==============================================================================

    UDPAudioProcessor.h
    Created: 12 Dec 2021 4:58:21pm
    Author:  arthurg

  ==============================================================================
*/

#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <JuceHeader.h>
#include <unistd.h>     /* for socket functions */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <opus.h>
#include <opus_multistream.h>
#include <unistd.h>     /* for socket functions */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <time.h>
#include "fifo.cpp"

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 960 //for Opus
#define FRAME_SIZE FRAMES_PER_BUFFER
#define MAX_FRAME_SIZE 6*FRAME_SIZE
#define MAX_PACKET_SIZE (3*1276)
#define SHORT_MAX 32767
#define MAX_LOG (180*1000) //1 hour of run time

// Multi-channel setups
// should work for 5.1 audio (6 channels)
#define NUM_CHAN 6
#define NUM_STREAMS 6
#define NUM_COUPLED_STREAMS 0

enum PlaybackMode {
    PBMODE_MONO = 0,
    PBMODE_STEREO,
    PBMODE_BINAURAL,
    PBMODE_5_1
};

//==============================================================================
/*
*/
class UDPAudioProcessor  : public juce::AudioAppComponent, private juce::Thread
{
public:
    UDPAudioProcessor();
    ~UDPAudioProcessor() override;

    // void setAudioChannels(int numInputChannels, int numOutputChannels, const juce::XmlElement *const storedSettings=nullptr);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

    /* Callback structure */
    class CBData
    {
    public:
        class Log
        {
        public:
            int n;
            int pkt_bytes;
            int write_seq_num;
            int read_seq_num;
            int fifo_len;
        };
        int samplerate;
        int channels;
        int frames;
        /* socket */
        int sock;
        struct sockaddr_in server;
        struct sockaddr_in from;
        Log log;
    };

    void send_server_msg(char c);
    void fill_fifo_buffer();
    int decode_and_write_to_fifo(unsigned char* rcbits, int n);
    unsigned char rcbits[MAX_PACKET_SIZE];
    unsigned int slen = sizeof(struct sockaddr_in);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UDPAudioProcessor)
    CBData cb_data;
    /* sockets */
    int portno;
    char *host;
    struct hostent *hp;
    unsigned char mapping[6] = {0,1,2,3,4,5};
    /*Holds the state of the and decoder */
    OpusMSDecoder *decoder;

    // void prepare_output_mono(float *input_buffer, const juce::AudioSourceChannelInfo &bufferToFill, unsigned long num_samples, int max_output_channels);
    // void prepare_output_stereo(float *input_buffer, const juce::AudioSourceChannelInfo &bufferToFill, unsigned long num_samples, int max_output_channels);
    // void prepare_output_binaural(float *input_buffer, const juce::AudioSourceChannelInfo &bufferToFill, unsigned long num_samples, int max_output_channels);
    // void prepare_output_5_1(float *input_buffer, const juce::AudioSourceChannelInfo &bufferToFill, unsigned long num_samples, int max_output_channels);

    void run() override;
    // juce::SpinLock fifo_mutex;


    // flags for function in preparetoplay
    int prepared_to_play = 0;
    int resources_released = 0;

    // saves decoded float samples
    FIFO raw_audio_fifo;

    enum PlaybackMode playback_mode = PBMODE_MONO;
};