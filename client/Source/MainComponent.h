#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  :  public juce::Component,
                        public juce::Button::Listener
//                        public juce::Slider::Listener
//                        public juce::Slider::Listener
//                        public juce::Timer,
//                        public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    int volume = 0.7;
    
    void play();
    void stop();
    void mute();
//    void timerCallback() override;
    void buttonClicked(juce::Button* button) override;
    
//
//    void sliderValueChanged(juce::Slider* slider) override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    enum class PlayState {
        Play,
        Stop,
        Mute
    };
    
    PlayState playState {PlayState::Stop};
    
    juce::Atomic<int> playbackFormat = 0;
    bool isConnected = false;
    
    bool isStereo = false;
    bool isBinaural = false;
    bool is5_1 = false;
    
//    juce::Atomic<bool> isStereo = true;
//    juce::Atomic<bool> isBinaural = false;
//    juce::Atomic<bool> is5_1 = false;
    
    bool isPlaying = false;
    bool isMuted = false;
    
    juce::TextButton buttonConnect{"CONNECT"};
    juce::TextButton buttonPlay{"PLAY"};
    juce::TextButton buttonMute{"MUTE"};
    juce::TextButton buttonStop{"STOP"};
    juce::Slider volSlider;
    
    juce::TextButton buttonStereo{"STEREO"};
    juce::TextButton buttonBinaural{"BINAURAL"};
    juce::TextButton button5_1{"5.1"};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
