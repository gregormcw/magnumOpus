#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  :  public juce::Component,
                        public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void play();
    void stop();
    void mute();
    void buttonClicked(juce::Button* button) override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    // Enum for button-click lambda functions
    enum class PlayState {
        Play,
        Stop,
        Mute
    };
    
    // Instance of PlayState (DEVELOP FURTHER)
    PlayState playState {PlayState::Stop};
    
    // Atomic int for audio format
    // 0 = Stereo, 1 = Binaural, 2 = 5.1
    juce::Atomic<int> playbackFormat = 0;
    
    // Button and callback flags
    bool isConnected = false;
    bool isPlaying = false;
    bool isMuted = false;
    
    // Variable determined by volSlider
    float volume = 0.7;
    
    std::string serverAdd = "serverAddHere:Port";
    
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
