#include <stdio.h>
#include <stdlib.h>     //exit()
#include <portaudio.h>

/* start up Port Audio */
PaStream *startupPa(int inputChanCount, int outputChanCount, 
    int samp_rate, int frames_per_buffer, PaStreamCallback *paCallback, void *cbData)
{
    PaStream *stream;
    PaError err;
    PaStreamParameters outputParams;
    PaStreamParameters inputParams;

    /* initialize PortAudio */
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }

    /* Input stream parameters */
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = inputChanCount;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency =
        Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = NULL;

    /* Ouput stream parameters */
    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = outputChanCount;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = NULL;

    /* Open audio stream */
    err = Pa_OpenStream(&stream,
        &inputParams, /* no input */
        &outputParams,
        samp_rate, frames_per_buffer,
        paNoFlag, /* flags */
        paCallback,
        cbData);

    if (err != paNoError) {
        printf("PortAudio error: open stream: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }

    /* start audio stream */
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: start stream: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }

    return stream;
}

/* shut down Port Audio */
void shutdownPa(PaStream *stream) 
{
    PaError err;
    /* stop stream */
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: stop stream: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }

    /* close stream */
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        printf("PortAudio error: close stream: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }

    /* terminate PortAudio */
    err = Pa_Terminate();
    if (err != paNoError) {
        printf("PortAudio error: terminate: %s\n", Pa_GetErrorText(err));
        printf("Exiting.\n");
        exit(1);
    }
}
