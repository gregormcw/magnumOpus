#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    
    juce::TextButton buttonPlay{"PLAY"};
    juce::TextButton buttonMute{"MUTE"};
    juce::TextButton buttonStop{"STOP"};
    juce::Slider volSlider;
    
    juce::TextButton buttonMono{"MONO"};
    juce::TextButton buttonStereo{"STEREO"};
    juce::TextButton button5_1{"5.1"};
    juce::TextButton button7_1{"7.1"};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
