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
    
    // ===================================================
    // CHANGE LAMBDA FUNCTION CALLS - CURRENTLY ALL PLAY()
    // ===================================================
    
    buttonConnect.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonConnect.onClick = [this]() { play(); };
    buttonConnect.addListener(this);
    
    buttonStereo.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonStereo.onClick = [this]() { play(); };
    buttonStereo.addListener(this);
    
    buttonBinaural.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonBinaural.onClick = [this]() { play(); };
    buttonBinaural.addListener(this);
    
    button5_1.setToggleState(false, juce::NotificationType::dontSendNotification);
    button5_1.onClick = [this]() { play(); };
    button5_1.addListener(this);

    addAndMakeVisible(buttonStereo);
    addAndMakeVisible(button5_1);
    addAndMakeVisible(buttonBinaural);
    
    buttonPlay.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonPlay.onClick = [this]() { play(); };
    buttonPlay.addListener(this);
    
    buttonMute.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonMute.onClick = [this]() { mute(); };
    buttonMute.addListener(this);
    
    buttonStop.setToggleState(false, juce::NotificationType::dontSendNotification);
    buttonStop.onClick = [this]() { stop(); };
    buttonStop.addListener(this);
    
    addAndMakeVisible(buttonConnect);
    addAndMakeVisible(buttonPlay);
    addAndMakeVisible(buttonMute);
    addAndMakeVisible(buttonStop);
    
}

void MainComponent::play() {
    
    if (playState == PlayState::Stop) {
        playState = PlayState::Play;
        
//        edit.getTransport().play(false);
//        juce::Timer::startTimer(100);
        
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

void MainComponent::buttonClicked(juce::Button* button) {
    
    if (button == &buttonConnect) {
        if (!isConnected) {
            buttonConnect.setColour(0x1000100, juce::Colours::orange);
        }
        else {
            buttonConnect.setColour(0x1000100, juce::Colours::black);
        }
        isConnected = !isConnected;
    }
    
    if (button == &buttonStereo) {
        
        playbackFormat = 0;
//        std::cout << "Format: Stereo" << std::endl;
        
        isStereo = true;
        isBinaural = false;
        is5_1 = false;
        
        buttonStereo.setColour(0x1000100, juce::Colours::orange);
        buttonBinaural.setColour(0x1000100, juce::Colours::black);
        button5_1.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &buttonBinaural) {
        
        playbackFormat = 1;
//        std::cout << "Format: Binaural" << std::endl;
        
        isStereo = false;
        isBinaural = true;
        is5_1 = false;
        
        buttonBinaural.setColour(0x1000100, juce::Colours::orange);
        buttonStereo.setColour(0x1000100, juce::Colours::black);
        button5_1.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &button5_1) {
        
        playbackFormat = 2;
//        std::cout << "Format: 5.1" << std::endl;
        
        isStereo = false;
        isBinaural = false;
        is5_1 = true;
        
        button5_1.setColour(0x1000100, juce::Colours::orange);
        buttonBinaural.setColour(0x1000100, juce::Colours::black);
        buttonStereo.setColour(0x1000100, juce::Colours::black);
    }
    
    if (button == &buttonPlay) {
        
        isPlaying = !isPlaying;
        
        buttonPlay.setColour(0x1000100, juce::Colours::limegreen);
        buttonStop.setColour(0x1000100, juce::Colours::whitesmoke);
        buttonStop.setColour(0x1000102, juce::Colours::black);
        buttonPlay.colourChanged();
        buttonStop.colourChanged();
    }
    
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
    
    if (button == &buttonStop) {
        
        isPlaying = false;
        
        buttonStop.setColour(0x1000100, juce::Colours::indianred);
        buttonStop.setColour(0x1000102, juce::Colours::whitesmoke);
        buttonPlay.setColour(0x1000100, juce::Colours::whitesmoke);
        buttonStop.colourChanged();
        buttonPlay.colourChanged();
    }
}

//void sliderValueChanged(Slider* slider) override {
//    if (slider == &volSlider)
//        volSlider.setValue(volSlider)
//}

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
    g.setFont (18.0f);
//    g.drawFittedText("Binaural Surround", 0, 210, getWidth(), 60, juce::Justification::centredTop, 1);
//    g.drawFittedText("Binaural Surround", -120, 95+65, getWidth(), 60, juce::Justification::centredTop, 1);
    
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
    
//    buttonMono.setState(juce::Button::buttonDown);
    
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
