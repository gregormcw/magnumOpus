/* Copyright (c) 2013 Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* This is meant to be a simple example of encoding and decoding audio
   using Opus. It should make it easy to understand how the Opus API
   works. For more information, see the full API documentation at:
   http://www.opus-codec.org/docs/ */
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <opus.h>
#include <stdio.h>
#include <string.h>  /* memset() */
#include <sndfile.h> /* libsndfile */

/*The frame size is hardcoded for this sample code but it doesn't have to be*/
#define FRAME_SIZE 960
// #define SAMPLE_RATE 48000
#define CHANNELS 2
#define APPLICATION OPUS_APPLICATION_AUDIO
// #define BITRATE 64000
#define MAX_FRAME_SIZE 6*960
#define MAX_PACKET_SIZE (3*1276)

int main(int argc, char **argv)
{
   char *inFile;
   char *outFile;
   int channels, samplerate, bitrate;
   /* ibsndfile structures */
   SNDFILE *isndfile, *osndfile; 
   SF_INFO isfinfo, osfinfo;
   /* opus variables and types */
   opus_int16 in[FRAME_SIZE*CHANNELS];
   opus_int16 out[MAX_FRAME_SIZE*CHANNELS];
   unsigned char cbits[MAX_PACKET_SIZE];
   int nbBytes;
   /*Holds the state of the encoder and decoder */
   OpusEncoder *encoder;
   OpusDecoder *decoder;
   int err;

   if (argc != 4) {
      fprintf(stderr, "usage: %s bitrate input.wav output.wav\n", argv[0]);
      return EXIT_FAILURE;
   }
   bitrate = atoi(argv[1]);
   inFile = argv[2];
   outFile = argv[3];

   /* zero libsndfile structures */
    memset(&isfinfo, 0, sizeof(isfinfo));
    memset(&osfinfo, 0, sizeof(osfinfo));

   /* open input WAV file */
   if ( (isndfile = sf_open (inFile, SFM_READ, &isfinfo)) == NULL ) {
      fprintf (stderr, "Error: could not open wav file: %s\n", inFile);
      return EXIT_FAILURE;
   }
   /* Print input file parameters */
   printf("Input audio file %s: Frames: %d Channels: %d Samplerate: %d\n", 
     inFile, (int)isfinfo.frames, isfinfo.channels, isfinfo.samplerate);

   /* Set output file parameters */
   osfinfo.samplerate = isfinfo.samplerate;
   osfinfo.channels = isfinfo.channels;
   osfinfo.format = isfinfo.format;

   /* Open output file */
   if ( (osndfile = sf_open (outFile, SFM_WRITE, &osfinfo)) == NULL ) {
      fprintf (stderr, "Error: could not open wav file: %s\n", outFile);
      return EXIT_FAILURE;
   }
   /* set signal parameters */
   samplerate = isfinfo.samplerate;
   channels = isfinfo.channels;

   /*Create a new encoder state */
   encoder = opus_encoder_create(samplerate, channels, APPLICATION, &err);
   if (err<0) {
      fprintf(stderr, "failed to create an encoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }
   /* Set the desired bit-rate. You can also set other parameters if needed.
      The Opus library is designed to have good defaults, so only set
      parameters you know you need. Doing otherwise is likely to result
      in worse quality, but better. */
   err = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
   if (err<0) {
      fprintf(stderr, "failed to set bitrate: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }


   /* Create a new decoder state. */
   decoder = opus_decoder_create(samplerate, channels, &err);
   if (err<0) {
      fprintf(stderr, "failed to create decoder: %s\n", opus_strerror(err));
      return EXIT_FAILURE;
   }

   for (int i=0; ; i++) {
      int count, frame_size;
      /* Read a 16 bits/sample audio frame. */
      if ( (count = sf_readf_short (isndfile, in, FRAME_SIZE)) != FRAME_SIZE) {
         break;
      }

      /* Encode the frame. */
      if ( (nbBytes = opus_encode(encoder, in, FRAME_SIZE, cbits, MAX_PACKET_SIZE)) < 0) {
         fprintf(stderr, "encode failed: %s\n", opus_strerror(nbBytes));
         return EXIT_FAILURE;
      }
      //printf("%3d %d\n", i, nbBytes);

      /* Decode the data. In this example, frame_size will be constant because
         the encoder is using a constant frame size. However, that may not
         be the case for all encoders, so the decoder must always check
         the frame size returned. */
      if ( (frame_size = opus_decode(decoder, cbits, nbBytes, out, MAX_FRAME_SIZE, 0)) < 0) {
         fprintf(stderr, "decoder failed: %s\n", opus_strerror(frame_size));
         return EXIT_FAILURE;
      }

      /* Write the decoded audio to file. */
      count = FRAME_SIZE;
      if ( (count = sf_writef_short (osndfile, out, FRAME_SIZE)) != FRAME_SIZE) {
         fprintf(stderr, "Write to WAV file failed\n");
         return EXIT_FAILURE;
      }

   }

   /*Destroy the encoder state*/
   opus_encoder_destroy(encoder);
   opus_decoder_destroy(decoder);

   /* close input and output fWAV ile */
   sf_close(isndfile);
   sf_close(osndfile);

   return EXIT_SUCCESS;
}
