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
// source_lient/udp_source_client.c, line 126:
cb_data.encoder = opus_multistream_encoder_create(cb_data.samplerate, cb_data.channels,
                                                      NUM_STREAMS, NUM_COUPLED_STREAMS,
                                                      mapping, APPLICATION, &err);
```
Where `NUM_STREAMS` is the number of encoded streams, and `NUM_COUPLED_STREAMS` is the number of encoded streams that are stereo. Since the standard opus API has two different kinds of streams (mono and stereo), we are allowed to choose how we want to encoder our 6 channel input. `mapping` is a list to indicate the relationship between the channel of raw audio and the channel of encoded streams.

The `opus_multistream_encoder_ctl()`, `opus_multistream_encoder_destroy()` and `opus_multistream_encode_float()` functions replaced their standard version as well.

Besides the source code, we need to modify the bitrate during execution as well. We experienced significant quality loss with bitrate less than `64000`.

As of the server, we actually keep it as the original version. Since the server is just forwarding the UDP packet given by source client, it is able to forward the multichannel packet just fine.


### UDP Client

As our first step, we attempted to modify the given UDP client to be able to handle 5.1 audio as well. We did several things here:
1. Change all the `opus_decode*()` call to `opus_multistream_decode*()`, and use the same `NUM_STREAMS`, `NUM_COUPLED_STREAMS`, `mapping` as the encoder.
2. Modify the `startupPa(1, output_channels)` function call according to the desired output format (`output_channels=1, 2 or 6`).
3. Define how to convert the 5.1 stream into other formats. Here's an example of stereo downmix: 
```C
// client/Source/udp_client.c, Line 482:
/* prepare the output stream
 * input is 5.1 buffer, output is a stereo downmixed buffer
 * using ffmpeg setup of downmixing: (https://superuser.com/questions/852400/properly-downmix-5-1-to-stereo-using-ffmpeg)
 * L = 0.374107*FC + 0.529067*FL + 0.458186*BL + 0.264534*BR + 0.374107*LFE
 * R = 0.374107*FC + 0.529067*FR + 0.458186*BR + 0.264534*BL + 0.374107*LFE
 * input channel order: front left, center, front right, rear left, rear right, LFE
 */
int prepare_output_stereo(float *input_buffer, float *output_buffer, unsigned long framesPerBuffer)
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
```

And it worked fine. The client is able to output mono, stereo and 5.1 stream to the audio device. The audio device will hopefully convert the sream again to fit the actual output channel, but our program doesn't need to care about things in the analog world.


### JUCE Client Backend

However, we encoutered problem when copy-pasting `udp_client.c` into our JUCE application. Eventhough the UDP client and the decoder all worked fine, but we noticed that the FIFO is quickly drained to the lowest level after only a few interations. We figured out several problems here:
1. The frame size of a opus frame is fixed to integer miliseconds, in our case it's 20ms with 48,000Hz so `opus_buffer_size=960`. In the portaudio application, we just opened up a output stream with the same buffer size. This is actually an additional layer of buffer created by portaudio.
2. In JUCE, the buffer size of output stream is set according to the actual audio device. For my device (and probably most of the sound interfaces on the market), it must be numbers like 256, 512, 1024, etc.
3. Seems like using a mutex lock within and audio callback such as `juce::AudioAppComponent::getNextAudioBlock()` is pretty bad idea...

So we made these improvements:
1. Create a new FIFO buffer class that is not buffering UDP packets, but buffering raw audio samples. In such case, we can write with size of 960 and read with 512.
2. Instead of `pthread`, use the juce multi-threading class `juce::Thread` So that we can set the priority of our UDP rx thread.
3. Instead of a mutex lock, design the FIFO buffer class as a ring buffer with separate read and write pointers.

The FIFO buffer class is in `client/Source/fifo.cpp`. We used the `juce::AbstractFifo` class to help create the read/write function, and used `juce::FloatVectorOperations::copy()` to read/write data from FIFO to other buffers.

### JUCE Client GUI

The client GUI was developed using JUCE, due to this framework's utility for both audio applications and frontend design. It is designed with simplicity in mind, and thus buttons and sliders were kept to a minimum.

The GUI consists of the following key components:

- `buttonStereo`: `juce::TextButton` object that switches output format to stereo
- `buttonBinaural`: `juce::TextButton` object that switches output format to binaural
- `buttonStereo`: `juce::TextButton` object that switches output format to Dolby 5.1
- `buttonPlay`: `juce::TextButton` object that initiates playback, or resumes playback mute toggled
- `buttonMute`: `juce::TextButton` object that mutes all audio output
- `buttonConnect`: `juce::TextButton` object that establishes connection with the server
- `volSlider`: `juce::Slider` object that allows control of output volume

Selection of audio format was provided via three instances of the `juce::TextButton` class, with `buttonStereo` for stereo downmix, `buttonBinaural` for binaural output, and `button5_1` for play back in Dolby 5.1 Surround. When one of these three buttons is selected, its related boolean variable is set to true and all others set to false.

The interoperation off the playback control buttons, `buttonPlay` and `buttonMute`, follow a similar logic, whereby if one is `true` the other is always `false`. This specific design consideration was made due to the nature of the incoming UDP packets, which are transmitted regardless of any play/stop states. As a result, it was decided that `buttonPlay` and `buttonMute` would more intuitively interact in this play/stop manner.

All of the GUI client buttons were positioned, sized, and formatted using member functions and variables of the `juce::TextButton` class, in a manner similar to the example below:

```C++
buttonPlay.setBounds(getWidth()/2 - ctlButtonWidth/2, getHeight()/2 + 1.24*ctlButtonHeight,
                          ctlButtonWidth, ctlButtonHeight);
```

All buttons except `buttonStereo` are initialized as `false`, with the `juce::TextButton` `.onClick` method used to connect button clicks to their respective callback functions via lambda expressions:

```C++
buttonPlay.setToggleState(false, juce::NotificationType::dontSendNotification);
buttonPlay.onClick = [this]() { play(); };
buttonPlay.addListener(this);
addAndMakeVisible(buttonPlay);
```
```C++
void MainComponent::play() {
    
    if (playState == PlayState::Mute) {
        playState = PlayState::Play;
    }
}
```

Each of the playback buttons, in particular, is linked to a custom enum `PlayState` class created in `MainComponent.h`, with the play state altered depending on the selected button's logic:

```
enum class PlayState {
        Play,
        Stop,
        Mute
};
```

`volSlider` - a `juce::Slider` instance that linearly changes the output volume - follows a similar logic, though its functionality can be implemented in a more concise manner:

```C++
volSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
volSlider.setRange(0.0f, 1.0f, 0.02f);
volSlider.setValue(0.7f);
volSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
volSlider.onValueChange = [this] { volume = volSlider.getValue(); };
addAndMakeVisible(volSlider);
```

During the frontend development and debugging phases, placeholder variables were created for the above frontend components in order to ensure playback states were correctly handled. Once the visual and functional aspects of the GUI were debugged, this component was connected to the backend, and the placeholder variables replaced with those in the `udp_client.c` code. The integration of frontend and backend was relatively seamless, due largely to the backend component's standalone functionality.
