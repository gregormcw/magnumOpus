#include "MainComponent.h"

const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 800;

//==============================================================================

MainComponent::MainComponent()
{
    
    setSize (WIN_WIDTH, WIN_HEIGHT);
    
    volSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    volSlider.setRange(0.0f, 1.0f, 0.02f);
    volSlider.setValue(0.7f);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    volSlider.onValueChange = [this] { volume = volSlider.getValue(); };
    addAndMakeVisible(volSlider);
    
    // =========================================================
    // == CHANGE LAMBDA FUNCTION CALLS - CURRENTLY ALL PLAY() ==
    // =========================================================
    
    // Set all initial toggle states to false
    buttonConnect.setToggleState(false, juce::NotificationType::dontSendNotification);
    
    // Call associated function on click
    buttonConnect.onClick = [this]() { play(); };
    
    // Add listener to button
    buttonConnect.addListener(this);
    
    // Make button visible
    addAndMakeVisible(buttonConnect);
    
    // Repeat for remaining buttons...
    
    // Audio format buttons
    buttonStereo.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonStereo.onClick = [this]() { play(); };
    buttonStereo.addListener(this);
    addAndMakeVisible(buttonStereo);
    
    buttonBinaural.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonBinaural.onClick = [this]() { play(); };
    buttonBinaural.addListener(this);
    addAndMakeVisible(buttonBinaural);
    
    button5_1.setToggleState(false, juce::NotificationType::dontSendNotification);
    button5_1.onClick = [this]() { play(); };
    button5_1.addListener(this);
    addAndMakeVisible(button5_1);
    
    // Playback-related buttons
    buttonPlay.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonPlay.onClick = [this]() { play(); };
    buttonPlay.addListener(this);
    addAndMakeVisible(buttonPlay);
    
    buttonMute.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonMute.onClick = [this]() { mute(); };
    buttonMute.addListener(this);
    addAndMakeVisible(buttonMute);
    
    buttonStop.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonStop.onClick = [this]() { stop(); };
    buttonStop.addListener(this);
    addAndMakeVisible(buttonStop);
    
}

// ===========================================================
// == UPDATE BELOW FUNCTIONS BELOW AS NECESSARY FOR BACKEND ==
// ===========================================================

void MainComponent::play() {
    
    if (playState == PlayState::Stop) {
        playState = PlayState::Play;
        
    }
}

void MainComponent::mute() {
    
    if (playState == PlayState::Play) {
        playState = PlayState::Mute;
    
    }
}

void MainComponent::stop() {
    
    if (playState == PlayState::Play) {
        playState = PlayState::Stop;
        
    }
}

// ====================================
// == END OF FUNCTIONS TO BE UPDATED ==
// ====================================

// ===========================================================
// == FUNCTION CONNECTING BUTTON CLICKS WITH AUDIO CALLBACK ==
// ===========================================================
void MainComponent::buttonClicked(juce::Button* button) {
    
    // If connect clicked, change button color and state
    if (button == &buttonConnect) {
        
        if (!isConnected) {
            
            // ============================================================
            // == REPLACE WITH VALID SERVER ADDRESS AND/OR FUNCTION CALL ==
            // ============================================================
            std::cout << "Connecting to server: " << serverAdd << std::endl;
            buttonConnect.setColour(0x1000100, juce::Colours::orange);
        }
        else {
            std::cout << "Disconnecting from server: " << serverAdd << std::endl;
            buttonConnect.setColour(0x1000100, juce::Colours::black);
        }
        
        isConnected = !isConnected;
    }
    
    // Set audio format and button color via button press
    if (button == &buttonStereo) {
        
        // 0 = Stereo
        playbackFormat = 0;
        
        buttonStereo.setColour(0x1000100, juce::Colours::orange);
        buttonBinaural.setColour(0x1000100, juce::Colours::black);
        button5_1.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &buttonBinaural) {
        
        // 1 = Binaural
        playbackFormat = 1;
        
        buttonBinaural.setColour(0x1000100, juce::Colours::orange);
        buttonStereo.setColour(0x1000100, juce::Colours::black);
        button5_1.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &button5_1) {
        
        // 2 = 5.1
        playbackFormat = 2;
        
        button5_1.setColour(0x1000100, juce::Colours::orange);
        buttonBinaural.setColour(0x1000100, juce::Colours::black);
        buttonStereo.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &buttonPlay) {
        
        // Change playback state
        isPlaying = !isPlaying;
        
        buttonPlay.setColour(0x1000100, juce::Colours::limegreen);
        buttonStop.setColour(0x1000100, juce::Colours::whitesmoke);
        buttonStop.setColour(0x1000102, juce::Colours::black);
        buttonPlay.colourChanged();
        buttonStop.colourChanged();
    }
    
    // If mute button clicked, change state and button color
    if (button == &buttonMute) {
        
        if (!isMuted) {
            buttonMute.setColour(0x1000100, juce::Colours::indianred);
            buttonMute.setColour(0x1000102, juce::Colours::whitesmoke);
        }
        else {
            buttonMute.setColour(0x1000100, juce::Colours::whitesmoke);
            buttonMute.setColour(0x1000102, juce::Colours::black);
        }
        
        isMuted = !isMuted;
        buttonMute.colourChanged();
        
    }
    
    // If stop button clicked, set isPlaying to false and change button color
    if (button == &buttonStop) {
        
        isPlaying = false;
        
        buttonStop.setColour(0x1000100, juce::Colours::indianred);
        buttonStop.setColour(0x1000102, juce::Colours::whitesmoke);
        buttonPlay.setColour(0x1000100, juce::Colours::whitesmoke);
        buttonStop.colourChanged();
        buttonPlay.colourChanged();
        
    }
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    
    g.fillAll(juce::Colours::lightslategrey);
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::darkorange);
    
    g.setFont(juce::Font("Avenir Next", 0, 0));
    g.setFont (120.0f);
    g.setColour (juce::Colours::orange);
    g.drawFittedText("Opus", 64+24, 95, getWidth(), 60, juce::Justification::centredTop, 1);
    g.setColour (juce::Colours::white);
    g.drawFittedText("Magnum", -100+72, 25, getWidth(), 60, juce::Justification::centredTop, 1);
    
    g.setFont(22.0f);
    int volTextWidth = 160;
    g.drawFittedText("VOLUME", getWidth()/2 - volTextWidth/2, getHeight()/2 + 254, volTextWidth, 20, juce::Justification::centred, 1);
    g.setFont(8.0f);
    g.drawFittedText("Designed and built by A. Guo & G. McWilliam", WIN_WIDTH-385, WIN_HEIGHT-20,
                     380, 20, juce::Justification::right, 1);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    int connectButtonWidth = 240*1.6;
    int connectButtonHeight = 144*0.9;
    
    int selectionButtonWidth = 240*1;
    int selectionButtonHeight = 144*0.9;
    
    int ctlButtonWidth = 4*selectionButtonWidth/5;
    int ctlButtonHeight = 4*selectionButtonHeight/5;
    
    int border = selectionButtonWidth / 12;
    
    buttonStereo.setColour(0x1000100, juce::Colours::black);
    buttonBinaural.setColour(0x1000100, juce::Colours::black);
    button5_1.setColour(0x1000100, juce::Colours::black);
    
    buttonConnect.setColour(0x1000100, juce::Colours::black);
    buttonMute.setColour(0x1000100, juce::Colours::whitesmoke);
    buttonMute.setColour(0x1000102, juce::Colours::black);
    buttonPlay.setColour(0x1000100, juce::Colours::whitesmoke);
    buttonPlay.setColour(0x1000102, juce::Colours::black);
    buttonStop.setColour(0x1000100, juce::Colours::whitesmoke);
    buttonStop.setColour(0x1000102, juce::Colours::black);
    
    buttonMute.setBounds(getWidth()/2 - 1.5*ctlButtonWidth - 1.36*border, getHeight()/2 + 1.24*ctlButtonHeight,
                         ctlButtonWidth, ctlButtonHeight);
    
    buttonPlay.setBounds(getWidth()/2 - ctlButtonWidth/2, getHeight()/2 + 1.24*ctlButtonHeight,
                          ctlButtonWidth, ctlButtonHeight);
    
    buttonStop.setBounds(getWidth()/2 + ctlButtonWidth/2 + 1.36*border, getHeight()/2 + 1.24*ctlButtonHeight,
                         ctlButtonWidth, ctlButtonHeight);
    
    buttonConnect.setBounds(getWidth()/2 - connectButtonWidth/2, getHeight()/4 + connectButtonHeight + 42,
                            connectButtonWidth, connectButtonHeight);
    
    // ====================================================================================
    // ====================================================================================
    // ====================================================================================
    
    buttonStereo.setBounds(getWidth()/2 - 1.5*selectionButtonWidth - 4*border, getHeight()/4 + selectionButtonHeight/5,
                           selectionButtonWidth, selectionButtonHeight);
    
    buttonBinaural.setBounds(getWidth()/2 - selectionButtonWidth/2, getHeight()/4 + selectionButtonHeight/5,
                        selectionButtonWidth, selectionButtonHeight);
    
    button5_1.setBounds(getWidth()/2 + selectionButtonWidth/2 + 4*border, getHeight()/4 + selectionButtonHeight/5,
                        selectionButtonWidth, selectionButtonHeight);
    
    int volSliderWidth = 440;
    int volSliderHeight = 100;
    
    volSlider.setBounds(getWidth()/2 - volSliderWidth/2, getHeight()/2 + 280,
                            volSliderWidth, volSliderHeight);

}
