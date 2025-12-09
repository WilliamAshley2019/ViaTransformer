#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>

class ViaTransformerProcessor final : public juce::AudioProcessor
{
public:
    ViaTransformerProcessor();
    ~ViaTransformerProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ViaTransformer"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // APVTS (AudioProcessorValueTreeState) for parameter control
    juce::AudioProcessorValueTreeState apvts;

    // Thread-safe value for editor to display the calculated dB gain
    std::atomic<float> currentGainDB{ 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // dsp::Gain is used to apply the calculated Turns Ratio (linear gain)
    juce::dsp::Gain<float> transformerGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViaTransformerProcessor)
};