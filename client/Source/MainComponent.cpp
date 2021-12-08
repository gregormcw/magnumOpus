#include "MainComponent.h"

const int WIN_WIDTH = 1200;
const int WIN_HEIGHT = 800;

//==============================================================================
MainComponent::MainComponent()
{
    setSize (WIN_WIDTH, WIN_HEIGHT);
    
//    aziSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
//    aziSlider.setRange(-180.0f, 180.0f, 1.0f);
//    aziSlider.setValue(0.0f);
//    aziSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
//    aziSlider.addListener(this);
//    addAndMakeVisible(aziSlider);
//
    volSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    volSlider.setRange(0.0f, 100.0f, 1.0f);
    volSlider.setValue(70.0f);
    volSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
//    volSlider.addListener(this);
    addAndMakeVisible(volSlider);
    
    addAndMakeVisible(buttonMono);
    addAndMakeVisible(buttonStereo);
    addAndMakeVisible(button5_1);
    addAndMakeVisible(button7_1);
    
    addAndMakeVisible(buttonPlay);
    addAndMakeVisible(buttonMute);
    addAndMakeVisible(buttonStop);
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
    
    int buttonWidth = 240;
    int buttonHeight = 144;
    
    int ctlButtonWidth = 4*buttonWidth/5;
    int ctlButtonHeight = 4*buttonHeight/5;
    
    int border = buttonWidth / 12;
    
    buttonMute.setColour(0x1000100, juce::Colours::orange);
    buttonPlay.setColour(0x1000100, juce::Colours::orange);
    buttonStop.setColour(0x1000100, juce::Colours::orange);
    
//    buttonMono.setState(juce::Button::buttonDown);
    
    buttonMute.setBounds(getWidth()/2 - 1.5*ctlButtonWidth - border, getHeight()/2 + ctlButtonHeight/2,
                         ctlButtonWidth, ctlButtonHeight);
    
    buttonPlay.setBounds(getWidth()/2 - ctlButtonWidth/2, getHeight()/2 + ctlButtonHeight/2,
                          ctlButtonWidth, ctlButtonHeight);
    
    buttonStop.setBounds(getWidth()/2 + ctlButtonWidth/2 + border, getHeight()/2 + ctlButtonHeight/2,
                         ctlButtonWidth, ctlButtonHeight);
    
    
    
    buttonMono.setBounds(getWidth()/5 - buttonWidth/2 - border, getHeight()/4 + buttonHeight/2,
                         buttonWidth, buttonHeight);
    
    buttonStereo.setBounds(2*getWidth()/5 - buttonWidth/2 - border/3, getHeight()/4 + buttonHeight/2,
                           buttonWidth, buttonHeight);
    
    button5_1.setBounds(3*getWidth()/5 - buttonWidth/2 + border/3, getHeight()/4 + buttonHeight/2,
                        buttonWidth, buttonHeight);
    
    button7_1.setBounds(4*getWidth()/5 - buttonWidth/2 + border, getHeight()/4 + buttonHeight/2,
                        buttonWidth, buttonHeight);
    
    int volSliderWidth = 440;
    int volSliderHeight = 100;
    
    volSlider.setBounds(getWidth()/2 - volSliderWidth/2, getHeight()/2 + 200,
                            volSliderWidth, volSliderHeight);

}
