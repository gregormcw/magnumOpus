/*
  ==============================================================================

    UDPAudioProcessor.cpp
    Created: 12 Dec 2021 4:58:21pm
    Author:  arthurg

  ==============================================================================
*/

#include "UDPAudioProcessor.h"

UDPAudioProcessor::UDPAudioProcessor(): juce::Thread("UDP client thread")
{
    this->host = (char *)"127.0.0.1";
    this->portno = 5002;
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    /* default values for audio output */
    this->cb_data.samplerate = SAMPLE_RATE;
    this->cb_data.channels = NUM_CHAN;
    this->cb_data.frames = FRAMES_PER_BUFFER;

    /* initialize log */
    this->cb_data.log.n = 0;
    this->cb_data.log.pkt_bytes = 0;
    this->cb_data.log.write_seq_num = 0;
    this->cb_data.log.read_seq_num = 0;
    // this->cb_data.log.fifo_len = 0;

    int output_channels;
    // if (playback_mode == PBMODE_MONO)
    //     output_channels = 1;
    // if ((playback_mode == PBMODE_BINAURAL) || (playback_mode == PBMODE_STEREO))
    //     output_channels = 2;
    // if (playback_mode == PBMODE_5_1)
        output_channels = 6;

    juce::String audioError;
    juce::AudioDeviceManager::AudioDeviceSetup audio_device_setup = juce::AudioDeviceManager::AudioDeviceSetup();
    // audio_device_setup.bufferSize = 256;
    audio_device_setup.sampleRate = 48000.0;
    audioError = deviceManager.initialise(0, output_channels, nullptr, true, juce::String(), &audio_device_setup);
    // deviceManager.setAudioDeviceSetup(this->audio_device_setup, true);
    jassert (audioError.isEmpty());
    // deviceManager.addAudioCallback (&audioSourcePlayer);
    // audioSourcePlayer.setSource (this);
    setAudioChannels(0, output_channels);
}

UDPAudioProcessor::~UDPAudioProcessor()
{
    this->shutdownAudio();
}

void UDPAudioProcessor::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    if (prepared_to_play == 0){
        juce::String message;
        message << "Preparing to play audio...\n";
        message << " samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
        message << " sampleRate = " << sampleRate;
        juce::Logger::getCurrentLogger()->writeToLog (message);

        raw_audio_fifo.setFrameSize(samplesPerBlockExpected * NUM_CHAN);

        int err;
        this->decoder = opus_multistream_decoder_create(this->cb_data.samplerate, NUM_CHAN, NUM_STREAMS, 
                                                        NUM_COUPLED_STREAMS, this->mapping, &err);
        if (err<0) {
            fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
            exit(EXIT_FAILURE);
        }



        prepared_to_play = 1;
    }else{}

}

void UDPAudioProcessor::releaseResources()
{
    if (resources_released == 0)
    {
        opus_multistream_decoder_destroy(this->decoder);
        disconnect_server();

        resources_released = 1;
    }else{}
}

void UDPAudioProcessor::send_server_msg(char c)
{
    unsigned char buf[MAX_PACKET_SIZE];
    int n, nbBytes, done;
    unsigned int slen = sizeof(struct sockaddr_in);
    /* socket timeout */
    struct timeval tv;

    /* check command */
    if ( !(c == 'C' || c == 'D') ) {
        fprintf(stderr, "Connection command %c must be C or D\n", c);
        exit(0);
    }
    /* connect to server */
    buf[0] = c; //connect if 'C' or disconnect if 'D'
    nbBytes = 1;
    done = 0;
    tv.tv_sec = 1; // 1 second
    tv.tv_usec = 0;
    while (!done) {
        /* send connect message to server */
        // printf("Sending connect message\n");
        if ( (n = sendto(this->cb_data.sock, buf, nbBytes, 0, (struct sockaddr*)&this->cb_data.server, slen)) < 0) {
            perror("Error sending connect message: ");
            printf("Attempting message: %c\n", c);
            exit(0);
        }

        /* Check if there is a message from server, with timeout */
        // printf("Checking for messages\n");
        setsockopt(this->cb_data.sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        n = recvfrom(this->cb_data.sock, buf, MAX_PACKET_SIZE, 0,
              (struct sockaddr*)&this->cb_data.from, &slen);
        if (n < -1) {
            perror("Error checking for messages...\n");
            exit(0);
        }
        else if (n == -1) {
            /* timeout from waiting */
            if (c == 'C') {
                /* Timeout on Connect: so client has NOT connected */
                ;
            }
            else {
                /* Timeout on Disconnect: so client HAS disconnected */
                //printf("Disconnected to server\n");
                done = 1;
            }
        }
        else {
            ; /* received a packet */
            if (c == 'C') {
                /* msg on Connect: so client HAS connected */
                //printf("Connected to server\n");
                done = 1;
            }
            else {
                /* msg on Disconnect: so client has NOT disconnected */
                ;
            }
        }
    }
}

void UDPAudioProcessor::fill_fifo_buffer()
{
    unsigned char buf[MAX_PACKET_SIZE];
    int n;
    unsigned int slen = sizeof(struct sockaddr_in);
    while ( raw_audio_fifo.getNumReady() < raw_audio_fifo.high ) {
            printf("%s  FIFO len: %d\n", "Filling buffer...", raw_audio_fifo.getNumReady());
        n = recvfrom(this->cb_data.sock, buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&this->cb_data.from, &slen);
        if (n < 0) {
            perror("ERROR in fill_fifo_buffer");
            exit(0);
        }
        else if (n > 0) {
            n = decode_and_write_to_fifo(buf, n);
            if (n < 0) {
                perror("ERROR in fill_fifo_buffer");
                exit(0);
            }
        }
    }
    printf("%s", "Buffer filled!!!\n");
}

/* prepare the output stream
 * input is 5.1 buffer, output is a stereo downmixed buffer
 * using ffmpeg setup of downmixing: (https://superuser.com/questions/852400/properly-downmix-5-1-to-stereo-using-ffmpeg)
 * L = 0.374107*FC + 0.529067*FL + 0.458186*BL + 0.264534*BR + 0.374107*LFE
 * R = 0.374107*FC + 0.529067*FR + 0.458186*BR + 0.264534*BL + 0.374107*LFE
 * input channel order: front left, center, front right, rear left, rear right, LFE
 */
void UDPAudioProcessor::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    auto* device = deviceManager.getCurrentAudioDevice();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto max_output_channels = activeOutputChannels.getHighestBit() + 1;
    int buffer_length = bufferToFill.numSamples*NUM_CHAN;

    // printf("output buffer len: %d\n", output.size());
    // printf("output buffer capacity: %d\n", buffer_length);
    // printf("FIFO len: %d\n", raw_audio_fifo.data.size());
    // printf("output channels: %d\n", max_output_channels);

    if (raw_audio_fifo.getNumReady() > raw_audio_fifo.low && connected_to_server == 1) {

        float output[buffer_length];
        memset(output, 0, sizeof(output));
        raw_audio_fifo.readFrom(output, buffer_length);
        switch(this->playback_mode){
            case PBMODE_MONO:
            {
                auto* outBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
                auto* outBuffer2 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
                for (int i=0; i<bufferToFill.numSamples; i++){
                    outBuffer[i] = mute * volume * (0.374107 * output[i*6 + 1] + \
                                    0.529067 * output[i*6 + 0] + \
                                    0.458186 * output[i*6 + 3] + \
                                    0.264534 * output[i*6 + 4] + \
                                    0.374107 * output[i*6 + 5] + \
                                    0.374107 * output[i*6 + 1] + \
                                    0.529067 * output[i*6 + 2] + \
                                    0.458186 * output[i*6 + 4] + \
                                    0.264534 * output[i*6 + 3] + \
                                    0.374107 * output[i*6 + 5])/2.0;
                    outBuffer2[i] = outBuffer[i];
                }
                // printf("%s\n", "rendering mono");
                // printf("num_sample to fill: %d\n", bufferToFill.numSamples);
                break;
            }
            case PBMODE_STEREO:
            {
                auto* outLeftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
                auto* outRightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
                for (int i=0; i<bufferToFill.numSamples; i++){
                    outLeftBuffer[i] = mute * volume * 0.374107 * output[i*6 + 1] + \
                                    0.529067 * output[i*6 + 0] + \
                                    0.458186 * output[i*6 + 3] + \
                                    0.264534 * output[i*6 + 4] + \
                                    0.374107 * output[i*6 + 5];
                    outRightBuffer[i] = mute * volume * 0.374107 * output[i*6 + 1] + \
                                    0.529067 * output[i*6 + 2] + \
                                    0.458186 * output[i*6 + 4] + \
                                    0.264534 * output[i*6 + 3] + \
                                    0.374107 * output[i*6 + 5];
                }
                // printf("%s\n", "rendering stereo");
                break;
            }
            case PBMODE_BINAURAL:
            {
                // prepare_output_binaural(output_full, bufferToFill, bufferToFill.numSamples, maxOutputChannels);
                break;
            }
            case PBMODE_5_1:
            {
                for (auto channel = 0; channel<6; ++channel)
                {
                    auto* outBuffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
                    for (int i=0; i<bufferToFill.numSamples; i++){
                        outBuffer[i] = mute * volume * output[i*6+channel];
                    }
                }
                break;
            }
        }
            this->cb_data.log.n++;
    }
    else {
        /* no data to decode, so just zero output buffer */
        bufferToFill.clearActiveBufferRegion();
        printf("%s\n", "No enough samples, output zero");
    }
}

void UDPAudioProcessor::run()
{
    unsigned int slen = sizeof(struct sockaddr_in);
    int n;

    while (1) {
        /* Wait for message from server */
        n = recvfrom(cb_data.sock, rcbits, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&this->cb_data.from, &slen);
        // printf("received bytes: %d,  FIFO len: %d\n", n, raw_audio_fifo.getNumReady());
        if (n < 0) {
            perror("Error in udp client thread");
            exit(0);
        }
        else if (n == 0) {
            ; //zero length packet, so do nothing
        }
        else {
            n = decode_and_write_to_fifo(rcbits, n);
            if (n < 0){
                printf("%s\n", "ERROR: No sample decoded");
                exit(0);                
            }
        }
    }
}

int UDPAudioProcessor::decode_and_write_to_fifo(unsigned char* rcbits, int n)
{
    this->cb_data.log.pkt_bytes = n;
    this->cb_data.log.read_seq_num = rcbits[0];
    int mrc1;
    int frame_size;
    float output_full[NUM_CHAN * FRAME_SIZE];
    int nbBytes = n-1;

    if ( (frame_size = opus_multistream_decode_float(this->decoder, &rcbits[1], nbBytes, output_full, MAX_FRAME_SIZE, 0)) < 0) {
        fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
        exit(1);
    }
    // printf("Decoded %d frames\n", frame_size);

    // if (fifo_mutex.tryEnter()){
        raw_audio_fifo.writeTo(output_full, frame_size*NUM_CHAN);
        // fifo_mutex.exit();
    // }
    return frame_size;
}

void UDPAudioProcessor::change_playback_mode(PlaybackMode playback_mode)
{
    this->playback_mode = playback_mode;
    // int output_channels;
    // if (playback_mode == PBMODE_MONO)
    //     output_channels = 1;
    // if ((playback_mode == PBMODE_BINAURAL) || (playback_mode == PBMODE_STEREO))
    //     output_channels = 2;
    // if (playback_mode == PBMODE_5_1)
    //     output_channels = 6;
    // setAudioChannels(0, output_channels);
}

void UDPAudioProcessor::connect_to_server(char* host, int portno)
{
    this->host = host;
    this->portno = portno;
    if (connected_to_server == 0)
    {
        /* Create socket */
        if ( (this->cb_data.sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("ERROR, cannot open socket");
            exit(EXIT_FAILURE);
        }
        /* load host (server) data structure */
        this->cb_data.server.sin_family = AF_INET;
        if ( (this->hp = gethostbyname(this->host)) == 0) {
            perror("ERROR, no such host");
            exit(EXIT_FAILURE);
        }
        /* load host (server) port (for client) */
        bcopy((char *)hp->h_addr, 
            (char *)&cb_data.server.sin_addr, hp->h_length);
        /* convert short from host to network byte order */
        this->cb_data.server.sin_port = htons(this->portno);
        /* connect to server */
        send_server_msg('C');
        // printf("host: %s, port: %d\n", , this->portno);
        fill_fifo_buffer();
        connected_to_server = 1;
        printf("%s\n", "Connected to server!");
        startThread(10);
    }
}

void UDPAudioProcessor::disconnect_server()
{
    this->stopThread(10);
    /* disconnect from server */
    send_server_msg('D');
    /* close socket */
    close(this->cb_data.sock);
}