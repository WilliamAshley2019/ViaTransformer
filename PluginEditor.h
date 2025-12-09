#pragma once

#include <juce_audio_processors/juce_audio_processors.h> 
#include <juce_gui_extra/juce_gui_extra.h>

class ViaTransformerProcessor;

class ViaTransformerProcessorEditor final : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    explicit ViaTransformerProcessorEditor(ViaTransformerProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~ViaTransformerProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    ViaTransformerProcessor& processor;

    // UI Elements
    juce::Slider ratioSlider;
    juce::Label ratioLabel;
    juce::Label gainDisplayLabel;
    juce::Label gainDisplayValue;

    // Attachment
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> ratioAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViaTransformerProcessorEditor)
};