#include "PluginEditor.h"
#include "PluginProcessor.h" 
#include <cmath>

// ==============================================================================
ViaTransformerProcessorEditor::ViaTransformerProcessorEditor(ViaTransformerProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : juce::AudioProcessorEditor(&p),
    processor(p)
{
    setSize(400, 300);
    setResizable(true, true);

    // --- 1. Turns Ratio Slider ---
    ratioSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    ratioSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    addAndMakeVisible(ratioSlider);

    ratioLabel.setText("Turns Ratio (Ns/Np)", juce::dontSendNotification);
    // 🟢 JUCE 8 FIX: Use standard juce::Font(height, styleFlags) constructor
    ratioLabel.setFont(juce::Font(14.0f, juce::Font::plain)); // Line 20
    ratioLabel.attachToComponent(&ratioSlider, false);
    addAndMakeVisible(ratioLabel);

    // Link slider to the parameter using the JUCE 8 APVTS attachment
    ratioAttachment = std::make_unique<SliderAttachment>(vts, "TURNS_RATIO", ratioSlider);

    // --- 2. Gain Display (dB) ---
    gainDisplayLabel.setText("Gain (dB)", juce::dontSendNotification);
    gainDisplayLabel.setFont(juce::FontOptions().withHeight(14.0f));
    gainDisplayLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(gainDisplayLabel);

    gainDisplayValue.setText("0.0 dB", juce::dontSendNotification);
    // 🟢 JUCE 8 FIX: Use standard juce::Font(height, styleFlags) constructor
    gainDisplayValue.setFont(juce::Font(24.0f, juce::Font::bold)); // Line 35
    gainDisplayValue.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(gainDisplayValue);

    startTimerHz(30); // Update the dB display 30 times per second
}

// ==============================================================================
void ViaTransformerProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Slider placement
    auto sliderArea = area.removeFromTop(area.getHeight() * 0.7f);
    // 🟢 JUCE 8 FIX: Use .toInt() on the centre point to resolve C4244 warning
    ratioSlider.setBounds(sliderArea.withSize(200, 200).withCentre(sliderArea.getCentre().toInt()));

    // Gain display placement
    auto displayArea = area.removeFromBottom(40).reduced(10, 0);
    // 🟢 JUCE 8 FIX: Use static_cast<int> for the float calculation to resolve C4244 warning
    gainDisplayLabel.setBounds(displayArea.removeFromLeft(static_cast<int>(displayArea.getWidth() * 0.5f)));
    gainDisplayValue.setBounds(displayArea);
}

// ==============================================================================
void ViaTransformerProcessorEditor::timerCallback()
{
    // Fetch the current calculated dB gain from the processor
    const float gainDb = processor.currentGainDB.load();

    juce::String gainText;

    // Format the text to include a '+' sign for positive gain
    if (gainDb >= 0.0f)
        gainText = "+" + juce::String(gainDb, 1) + " dB";
    else
        gainText = juce::String(gainDb, 1) + " dB";

    gainDisplayValue.setText(gainText, juce::dontSendNotification);
}

// ==============================================================================
void ViaTransformerProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    g.setColour(juce::Colours::white);
    // 🟢 JUCE 8 FIX: Use standard juce::Font(height, styleFlags) constructor
    g.setFont(juce::Font(18.0f, juce::Font::bold)); // Line 83
    g.drawText("ViaTransformer - Turns Ratio Gain", getLocalBounds().removeFromTop(30).reduced(10, 0), juce::Justification::centred);
}