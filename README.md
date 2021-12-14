# magnumOpus
Binaural surround spatialization app for streaming audio



<img width="1200" alt="magnumOpus_ss" src="https://user-images.githubusercontent.com/62677644/144144375-5668028c-d47a-4773-ba65-f7e465806aa8.png">


## TODOs
- make real-time convolution to create downmixed binaural output
- connect the GUI with callback functions
- Modify UI design:
  - switch between mono, stereo, binaural and 5.1
  - add host and port input, and a connect button

## How To Run


## Backend Structure
The overall structure is still the same as provided: source client, server, and client. The source client and server are modified to encode and transfer multichannel opus stream, and the client is almost rewritten with C++ and JUCE.

### Source Client and Server
The source client is modified using `opus multistream` API to handle 5.1 inputs. The multistream API will pack multiple opus stream in one packet and send them together as the UDP packet. Each opus stream is the same as the standard opus API.
First of all, the `opus_encoder_create()` in source client is modified to:
```C
// udp_source_client.c, line 126:
cb_data.encoder = opus_multistream_encoder_create(cb_data.samplerate, cb_data.channels,
                                                      NUM_STREAMS, NUM_COUPLED_STREAMS,
                                                      mapping, APPLICATION, &err);
```
Where `NUM_STREAMS` is the number of encoded streams, and `NUM_COUPLED_STREAMS` is the number of encoded streams that are stereo. Since the standard opus API has two different kinds of streams (mono and stereo), we are allowed to choose how we want to encoder our 6 channel input. `mapping` is a list to indicate the relationship between the channel of raw audio and the channel of encoded streams.

The `opus_multistream_encoder_ctl()`, `opus_multistream_encoder_destroy()` and `opus_multistream_encode_float()` functions replaced their standard version as well.

Besides the source code, we need to modify the bitrate during execution as well. We experienced significant quality loss with bitrate less than `64000`.

As of the server, we actually keep it as the original version. Since the server is just forwarding the UDP packet given by source client, it is able to forward the multichannel packet just fine.
