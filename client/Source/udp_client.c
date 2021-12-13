/*****************************************************************************
 * 
 * Recive from streaming server using UDP protocol
 * Decode using Opus and write to
 * PortAudio output buffer
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>     /* for socket functions */
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>     /* memset() */
#include <ctype.h>      /* tolower() */
#include <stdatomic.h>  /* atomic read/write */
// #include <sndfile.h>    /* libsndfile */
#include <portaudio.h>  /* portaudio */
#include <ncurses.h>    /* ncurses */
#include <opus.h>       /* opus */
#include <pthread.h>    /* POSIX threads */
#include "paUtils.h"
#include "fifo.h"
#include <opus_multistream.h>

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

/* log structure */
typedef struct {
    int n;
    int pkt_bytes;
    int write_seq_num;
    int read_seq_num;
    int fifo_len;
} Log;

enum PlaybackMode {
    PBMODE_MONO = 0,
    PBMODE_STEREO,
    PBMODE_BINAURAL,
    PBMODE_5_1
};

/* Callback structure */
typedef struct {
    enum PlaybackMode playback_mode;
    int samplerate;
    int channels;
    int frames;
    /*Holds the state of the encoder and decoder */
    OpusMSDecoder *decoder;
    /* socket */
    int sock;
    struct sockaddr_in server;
    struct sockaddr_in from;
    Log log;

} CBdata;




/* local function prototypes */
void send_server_msg(char c, CBdata *p);
void fill_fifo_buffer(CBdata *p);
void *rcvFrmServer( void *id);
int prepare_output_mono(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer);
int prepare_output_stereo(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer);
int prepare_output_binaural(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer);
int prepare_output_5_1(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer);



/* Callback function protoype */
static int paCallback( const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData );

/* mutex */
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    int err;
    /* portaudio */
    PaStream *stream;
    CBdata cb_data;
    /* sockets */
    int portno;
    char *host;
    struct hostent *hp;
    unsigned char mapping[] = {0,1,2,3,4,5};
    /* POSIX threads */
    int rc1, mrc1;
    pthread_t thread1;
    enum PlaybackMode playback_mode = PBMODE_5_1;

    if (argc < 3) {
        printf("Usage: %s host_name port_num\n", argv[0]);
        exit(-1);
    }
    host = argv[1];
    portno = atoi(argv[2]);

    /* default values for audio output */
    cb_data.samplerate = SAMPLE_RATE;
    cb_data.channels = NUM_CHAN;
    cb_data.frames = FRAMES_PER_BUFFER;
    cb_data.playback_mode = playback_mode;

    /* initialize log */
    cb_data.log.n = 0;
    cb_data.log.pkt_bytes = 0;
    cb_data.log.write_seq_num = 0;
    cb_data.log.read_seq_num = 0;
    cb_data.log.fifo_len = 0;

    /* Create a new opus decoder state. */
    cb_data.decoder = opus_multistream_decoder_create(cb_data.samplerate,
                                                      NUM_CHAN,
                                                      NUM_STREAMS,
                                                      NUM_COUPLED_STREAMS,
                                                      mapping,
                                                      &err);
    if (err<0) {
        fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
        return EXIT_FAILURE;
    }

    /* Create socket */
    if ( (cb_data.sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("ERROR, cannot open socket");
        exit(1);
    }
    /* load host (server) data structure */
    cb_data.server.sin_family = AF_INET;
    if ( (hp = gethostbyname(host)) == 0) {
        perror("ERROR, no such host");
        exit(1);
    }
    /* load host (server) port (for client) */
    bcopy((char *)hp->h_addr, 
        (char *)&cb_data.server.sin_addr, hp->h_length);
    /* convert short from host to network byte order */
    cb_data.server.sin_port = htons(portno);

    /* connect to server */
    send_server_msg('C', &cb_data);

    /* initialize FIFO */
    initializeFifo();

    /* fill jitter buffer FIFO to target level */
    fill_fifo_buffer(&cb_data);
    //sprintf(cb_data.log.msg, "Done filling FIFO");

    /* start thread
     * receives UDP packets from server and writes them to FIFO
     */
    if ( (rc1 = pthread_create( &thread1, NULL, &rcvFrmServer, &cb_data)) ) {
        printf("Thread creation failed: %d\n", rc1);
        exit(0);
    }
    /* initialize mutex1 */
    if ( (mrc1 = pthread_mutex_init(&mutex1, NULL)) != 0 ) {
        printf("Mutex initializaton failed: %d\n", mrc1);
        exit(0);
    }

    /* start up Port Audio 
     * input defaults to mono
     */
    // stream = startupPa(1, cb_data.channels, cb_data.samplerate, 
    //     FRAMES_PER_BUFFER, paCallback, &cb_data);
    int output_channels;
    if (playback_mode == PBMODE_MONO)
        output_channels = 1;
    if ((playback_mode == PBMODE_BINAURAL) || (playback_mode == PBMODE_STEREO))
        output_channels = 2;
    if (playback_mode == PBMODE_5_1)
        output_channels = 6;
    stream = startupPa(1, output_channels, cb_data.samplerate, FRAMES_PER_BUFFER, paCallback, &cb_data);

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
        Log *p = &cb_data.log;
        ch = tolower(getch()); 
        mvprintw(1, 0, "CPU Load: %lf\n", Pa_GetStreamCpuLoad (stream) );
        mvprintw(2, 0, "Block %d, %6d bytes, Len %3d, %d %d\n",  
            p->n, p->pkt_bytes, p->fifo_len,  
            p->write_seq_num, p->read_seq_num);
        refresh();
    }
    /* shut down Ncurses */
    endwin();

    /* kill thread */
    pthread_cancel(thread1);

    /* shut down Port Audio */
    shutdownPa(stream);

    /* destroy mutex1 */
    if ( (mrc1 = pthread_mutex_destroy(&mutex1)) != 0 ) {
        printf("Mutex destroy failed: %d\n", mrc1);
        exit(0);
    }

    /* shut down Opus decoder */
    opus_multistream_decoder_destroy(cb_data.decoder);

    /* disconnect from server */
    send_server_msg('D', &cb_data);

    /* close socket */
    close(cb_data.sock);  

    return 0;
}

void send_server_msg(char c, CBdata *p)
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
        //printf("Send connect message\n");
        if ( (n = sendto(p->sock, buf, nbBytes, 0,
            (struct sockaddr*)&p->server, slen)) < 0) {
            perror("recvfrom client");
            exit(0);
        }

        /* Check if there is a message from server, with timeout */
        //printf("Checking for messages\n");
        setsockopt(p->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        n = recvfrom(p->sock, buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&p->from, &slen);
         if (n < -1) {
            perror("recvfrom client");
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

void fill_fifo_buffer(CBdata *p)
{
    unsigned char buf[MAX_PACKET_SIZE];
    int n;
    unsigned int slen = sizeof(struct sockaddr_in);
    while ( fifoLength() < FIFO_HI ) {
        n = recvfrom(p->sock, buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&p->from, &slen);
        if (n < 0) {
            perror("recvfrom client");
            exit(0);
        }
        else if (n > 0) {
            writeToFifo(buf, n);
        }
    }
}

/* this is a separate thread that 
 * receives UDP packets from server and writes them to FIFO
 */
void *rcvFrmServer( void *id)
{
    CBdata *p = (CBdata *)id;
    int n;
    unsigned char rcbits[MAX_PACKET_SIZE];
    unsigned int slen = sizeof(struct sockaddr_in);
    int mrc1;

    while (1) {
        /* Wait for message from server */
        n = recvfrom(p->sock, rcbits, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&p->from, &slen);
        if (n < 0) {
            perror("rcvFrmServer");
            exit(0);
        }
        else if (n == 0) {
            ; //zero length packet, so do nothing
        }
        else {
            /* lock mutex */
            if ( (mrc1 = pthread_mutex_lock( &mutex1)) != 0 ){
                printf("PortAudio callback mutex lock error\n");
                exit(0);
            }

            /* write to jitter buffer FIFO */
            writeToFifo(rcbits, n);
            p->log.write_seq_num = rcbits[0];
            p->log.fifo_len = fifoLength();
            //p->log.fifo_free = fifoNumFree();

            /* release mutex */
            if ( (mrc1 = pthread_mutex_unlock( &mutex1)) != 0 ){
                printf("PortAudio callback mutex unlock error\n");
                exit(0);
            }

            /* unlock */
        }
    }
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
    //float *input  = (float *)inputBuffer; //not used
    float *output = (float *)outputBuffer;
    float output_full[NUM_CHAN * FRAME_SIZE];
    unsigned char rcbits[MAX_PACKET_SIZE];
    int nbBytes;
    int n;
    int mrc1;
    //unsigned int slen = sizeof(struct sockaddr_in);
    // /* socket timeout */
    // struct timeval tv;
    // tv.tv_sec = 0;
    // tv.tv_usec = 5*1000; //5 ms

    // /* Get message from server with timeout */
    // setsockopt(p->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // n = recvfrom(p->sock, rcbits, MAX_PACKET_SIZE, 0,
    //     (struct sockaddr*)&p->from, &slen);
    // if (n < -1) {
    //     perror("recvfrom client");
    //     exit(0);
    // }
    // else if (n == -1) {
    //     /* timeout from waiting -- no UDP packet */
    //     p->log.timeouts++;
    //     nbBytes = -1; 
    // }
    // else {
    //     /* received packet */
    //     p->log.pkt_bytes = n;
    //     p->log.n++;

    //     /* write to jitter buffer FIFO */
    //     writeToFifo(rcbits, n);
    //     p->log.write_seq_num = rcbits[0];
    // }

    /* if not sequential seq num, then treat as missing
     * do not decode, and just zero output buffer 
     */
//readFrmFifo takes seq num as arg?

    /* read from jitter buffer FIFO 
     * if FIFO level is too low, do not read until rcvFrmServer() thread
     * fills FIFO above low watermark
     */
    if (p->log.fifo_len > FIFO_LO) {

        /* lock mutex */
        if ( (mrc1 = pthread_mutex_lock( &mutex1)) != 0 ){
            printf("PortAudio callback mutex lock error\n");
            exit(0);
        }

        /* read from FIFO */
        n = readFrmFifo(rcbits);

        /* release mutex */
        if ( (mrc1 = pthread_mutex_unlock( &mutex1)) != 0 ){
            printf("PortAudio callback mutex unlock error\n");
            exit(0);
        }

        /* if we read a packet */
        if (n > 0 ) {
            p->log.pkt_bytes = n;
            p->log.read_seq_num = rcbits[0];
            nbBytes = n-1;
            /* Decode the data. In this example, frame_size will be constant 
             because the encoder is using a constant frame size. 
             However, that may not be the case for all encoders, 
             so the decoder must always check the frame size returned. 
             */
            memset(output_full, 0, sizeof(output_full));
            int frame_size;
            if ( (frame_size = opus_multistream_decode_float(p->decoder, &rcbits[1], nbBytes, output_full, MAX_FRAME_SIZE, 0)) < 0) {
                fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
                exit(1);
            }

            switch(p->playback_mode){
                case PBMODE_MONO:
                    prepare_output_mono(output_full, output, framesPerBuffer);
                    break;
                case PBMODE_STEREO:
                    prepare_output_stereo(output_full, output, framesPerBuffer);
                    break;
                case PBMODE_BINAURAL:
                    prepare_output_binaural(output_full, output, framesPerBuffer);
                    break;
                case PBMODE_5_1:
                    prepare_output_5_1(output_full, output, framesPerBuffer);
                    break;
            }
            p->log.n++;
        }
    }
    else {
        /* no data to decode, so just zero output buffer */
        int k=0;
        for (int i=0; i<framesPerBuffer; i++) {
            for (int j=0; j<p->channels; j++) {
                output[k] = 0.0;
                k++;
            }
        }
    }

    return 0;
}

/* prepare the output stream
 * input is 5.1 buffer, output is a mono buffer
 */
int prepare_output_mono(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer)
{
    for (int i=0; i<framesPerBuffer; i++){
        output_buffer[i] = 0.374107 * input_buffer[i*6 + 1] + \
                           0.529067 * input_buffer[i*6 + 0] + \
                           0.458186 * input_buffer[i*6 + 3] + \
                           0.264534 * input_buffer[i*6 + 4] + \
                           0.374107 * input_buffer[i*6 + 5] + \
                           0.374107 * input_buffer[i*6 + 1] + \
                           0.529067 * input_buffer[i*6 + 2] + \
                           0.458186 * input_buffer[i*6 + 4] + \
                           0.264534 * input_buffer[i*6 + 3] + \
                           0.374107 * input_buffer[i*6 + 5];
        output_buffer[i] /= 2.0;
    }
    return 0;
}

/* prepare the output stream
 * input is 5.1 buffer, output is a stereo downmixed buffer
 * using ffmpeg setup of downmixing: (https://superuser.com/questions/852400/properly-downmix-5-1-to-stereo-using-ffmpeg)
 * L = 0.374107*FC + 0.529067*FL + 0.458186*BL + 0.264534*BR + 0.374107*LFE
 * R = 0.374107*FC + 0.529067*FR + 0.458186*BR + 0.264534*BL + 0.374107*LFE
 * input channel order: front left, center, front right, rear left, rear right, LFE
 */
int prepare_output_stereo(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer)
{
    int k=0;
    for (int i=0; i<framesPerBuffer; i++){
        // left
        output_buffer[k] = 0.374107 * input_buffer[i*6 + 1] + \
                           0.529067 * input_buffer[i*6 + 0] + \
                           0.458186 * input_buffer[i*6 + 3] + \
                           0.264534 * input_buffer[i*6 + 4] + \
                           0.374107 * input_buffer[i*6 + 5];
        k++;
        // right
        output_buffer[k] = 0.374107 * input_buffer[i*6 + 1] + \
                           0.529067 * input_buffer[i*6 + 2] + \
                           0.458186 * input_buffer[i*6 + 4] + \
                           0.264534 * input_buffer[i*6 + 3] + \
                           0.374107 * input_buffer[i*6 + 5];
        k++;
    }
    return 0;
}

int prepare_output_binaural(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer)
{
    int k=0;
    for (int i=0; i<framesPerBuffer; i++) {
        for (int j=0; j<2; j++) {
            output_buffer[k] = 0.0;
            k++;
        }
    }
    return 0;
}

// Prepare the output stream
// simply copy-paste the buffers
int prepare_output_5_1(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer)
{
    for (int i=0; i<framesPerBuffer; i++){
        for (int j=0; j<6; j++){
            output_buffer[i*6+j] = input_buffer[i*6+j];
        }
    }
    return 0;
}