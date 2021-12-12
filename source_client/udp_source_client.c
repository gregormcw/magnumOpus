/*****************************************************************************
 * 
 * Read from PortAudio input buffer
 * Encode with and Opus and send to streaming server
 * using UDP protocol
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     /* for socket functions */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>     /* memset() */
#include <ctype.h>      /* tolower() */
#include <sndfile.h>    /* libsndfile */
#include <portaudio.h>  /* portaudio */
#include <ncurses.h>    /* ncurses */
#include <opus.h>       /* opus */
#include <opus_multistream.h>
#include "paUtils.h"

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 960 //for Opus
#define FRAME_SIZE FRAMES_PER_BUFFER
// #define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000
#define MAX_FRAME_SIZE 6*FRAME_SIZE
#define MAX_PACKET_SIZE (3*1276)
#define SHORT_MAX 32767

#define ENAB_OPUS   1
#define ENAB_SOC    1

// Multi-channel setups
// should work for 5.1 audio (6 channels)
#define CHANNELS 6
#define NUM_STREAMS 6
#define NUM_COUPLED_STREAMS 0


/* Callback structure */
typedef struct {
    int samplerate;
    int channels;
    int frames;
    SNDFILE *isndfile;
    /*Holds the state of the encoder */
    OpusMSEncoder *encoder;
    /* socket */
    int sock;
    struct sockaddr_in server;
    int block;
    unsigned char seq_num;
} CBdata;

/* Callback function protoype */
static int paCallback( const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );

int main(int argc, char *argv[])
{
    char *ifile;
    int bitrate, err;
    /* libsndfile structures */
    SNDFILE *isndfile; 
    SF_INFO isfinfo;
    /* portaudio */
    PaStream *stream;
    CBdata cb_data;
    /* sockets */
    int portno;
    char *host;
    struct hostent *hp;
    unsigned char mapping[] = {0,1,2,3,4,5};
    // mapping from I/O channels to stream channels
    // stream channel 0: L: I/O channel 0, R: I/O channel 3
    // stream channel 1: L: I/O channel 1, R: I/O channel 4
    // stream channel 2: L: I/O channel 2, R: I/O channel 5

    if (argc < 4) {
        printf("Usage: %s bitrate host_name port_num [ifile.wav]\n", argv[0]);
        exit(-1);
    }
    bitrate = atoi(argv[1]);
    host = argv[2];
    portno = atoi(argv[3]);
    if (argc == 4) {
        /* no WAV file, so set default values */
        cb_data.isndfile = 0;
        cb_data.samplerate = SAMPLE_RATE;
        cb_data.channels = CHANNELS;
        cb_data.frames = FRAMES_PER_BUFFER;
    }
    else {
        /* get WAV filename */
        ifile = argv[4];

        /* zero libsndfile structures */
        memset(&isfinfo, 0, sizeof(isfinfo));

        /* open input WAV file */
        if ( (isndfile = sf_open (ifile, SFM_READ, &isfinfo)) == NULL ) {
            fprintf (stderr, "Error: could not open wav file: %s\n", ifile);
            exit(-1);
        }
        /* Print input file parameters */
        printf("Input audio file %s: \nFrames: %d \nChannels: %d \nSamplerate: %d\n", 
            ifile, (int)isfinfo.frames, isfinfo.channels, isfinfo.samplerate);

        cb_data.samplerate = isfinfo.samplerate;
        cb_data.channels = isfinfo.channels;
        cb_data.frames = isfinfo.frames;
        cb_data.isndfile = isndfile;
    }
    cb_data.block = 0;

    /* Create a new opus encoder state */

    cb_data.encoder = opus_multistream_encoder_create(cb_data.samplerate,
                                                      cb_data.channels,
                                                      NUM_STREAMS,
                                                      NUM_COUPLED_STREAMS,
                                                      mapping,
                                                      APPLICATION,
                                                      &err);
    if (err<0) {
        fprintf(stderr, "Failed to create an encoder: %s\n", opus_strerror(err));
        return EXIT_FAILURE;
    }
    /* Set the desired bit-rate. You can also set other parameters if needed.
      The Opus library is designed to have good defaults, so only set
      parameters you know you need. Doing otherwise is likely to result
      in worse quality, but better. */
    err = opus_multistream_encoder_ctl(cb_data.encoder, OPUS_SET_BITRATE(bitrate));
    if (err<0) {
        fprintf(stderr, "Failed to set bitrate: %s\n", opus_strerror(err));
        return EXIT_FAILURE;
    }

    /* Create socket */
    if ( (cb_data.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("ERROR, cannot open socket");
        exit(1);
    }
    /* load host (server) data structure */
    cb_data.server.sin_family = AF_INET;
    if ( (hp = gethostbyname(host)) == 0) {
        perror("ERROR, no such host");
        exit(1);
    }
    /* load host (server) port (for source client) */
    bcopy((char *)hp->h_addr, 
        (char *)&cb_data.server.sin_addr,
         hp->h_length);
    /* convert short from host to network byte order */
    cb_data.server.sin_port = htons(portno);
    /* initialize pkt seq num */
    cb_data.seq_num = 0;

    /* start up Port Audio */
    stream = startupPa(cb_data.channels, cb_data.channels, cb_data.samplerate, 
        FRAMES_PER_BUFFER, paCallback, &cb_data);

    /* Initialize ncurses 
     * to permit interactive character input 
     */
    initscr(); /* Start curses mode */
    cbreak();  /* Line buffering disabled */
    noecho();  /* Uncomment this if you don't want to echo characters when typing */  
    printw("Hit SPACE to quit:\n"); //line 0
    printw("\n");
    printw("\n");
    refresh();
    int ch = '\0'; /* Init ch to null character */
    while (ch != ' ') {
        ch = tolower(getch()); 
        mvprintw(1, 0, "CPU Load: %lf\n", Pa_GetStreamCpuLoad (stream) );
        mvprintw(2, 0, "Block %d %d\n", cb_data.block, cb_data.seq_num);
        refresh();
    }
    /* shut down Ncurses */
    endwin();

    /* shut down Port Audio */
    shutdownPa(stream);

    /* shut down Opus encoder */
    opus_multistream_encoder_destroy(cb_data.encoder);

    /* close socket */
    close(cb_data.sock);    

    return 0;
}

/* This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * in the routine that would block or stall processing.
 */
static int paCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    /* Cast data passed through stream to our structure. */
    CBdata *p = (CBdata *)userData;
    float *input  = (float *)inputBuffer;
    float *output = (float *)outputBuffer;
    /* opus variables and types */
    // opus_int16 in[FRAME_SIZE*CHANNELS];
    // opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
    unsigned char tcbits[MAX_PACKET_SIZE];
    int nbBytes;
    int n;
    unsigned int slen = sizeof(struct sockaddr_in);

    p->block++;
    /* Get audio */
    if (p->isndfile) {
        /* read from WAV file and overwrite input[] buffer 
         * otherwise just use input buffer from external audio device
         */
        int count = sf_readf_float (p->isndfile, input, framesPerBuffer);
        if (count < framesPerBuffer) {
            /* if a partial buffer was read, 
             * seek back to the beginning of the data part of audio file 
             */
            //printf("loop\n");
            sf_seek(p->isndfile, 0, SF_SEEK_SET);
            /* and fill rest of buffer */
            sf_readf_float (p->isndfile, &input[count], framesPerBuffer-count);
        }
    }

    /* Encode the frame. */
    if ( (nbBytes = opus_multistream_encode_float(p->encoder, input, framesPerBuffer, &tcbits[1], MAX_PACKET_SIZE)) < 0) {
        printf("%d\n", nbBytes);
        fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
        exit(1);
    }

    /* insert pkt seq num */
    tcbits[0] = p->seq_num;
    nbBytes++;
    p->seq_num++;
    /* Send to server, UDP protocol */
    if ( (n = sendto(p->sock, tcbits, nbBytes, 0,
        (struct sockaddr*)&p->server, slen)) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    else if (n != nbBytes) {
        perror("ERROR wrote wrong number of bytes to socket");
        exit(1);
    }

    /* zero output buffer */
    int k=0;
    for (int i=0; i<framesPerBuffer; i++) {
        for (int j=0; j<p->channels; j++) {
            output[k] = 0.0;
            k++;
        }
    }

    return 0;
}
