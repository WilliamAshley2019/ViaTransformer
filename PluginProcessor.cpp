#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

#if _MSC_VER
#pragma float_control(precise, off)
#pragma fp_contract(on)
#endif

// ==============================================================================
// Parameter Layout Definition
juce::AudioProcessorValueTreeState::ParameterLayout ViaTransformerProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 1. TURNS_RATIO (n = Ns/Np)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "TURNS_RATIO", 1 },
        "Turns Ratio (n)",
        juce::NormalisableRange<float>(0.01f, 100.0f, 0.01f, 0.5f),
        1.0f, // Default to 1.0 (0 dB)
        // 🟢 JUCE 8 FIX: Use AudioParameterFloatAttributes to resolve C4996 warning
        juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float value, int) {
            return juce::String(value, 2); // Display 2 decimal places
            })
        .withValueFromStringFunction([](const juce::String& text) {
            return text.getFloatValue();
            })
    ));

    return { params.begin(), params.end() };
}

// ==============================================================================
// Constructor
ViaTransformerProcessor::ViaTransformerProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

// ==============================================================================
// isBusesLayoutSupported
bool ViaTransformerProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    return mainOut == juce::AudioChannelSet::mono()
        || mainOut == juce::AudioChannelSet::stereo();
}

// ==============================================================================
// prepareToPlay
void ViaTransformerProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = (juce::uint32)getTotalNumOutputChannels();

    transformerGain.prepare(spec);
    transformerGain.reset();

    currentGainDB.store(0.0f, std::memory_order_relaxed);
    setLatencySamples(0);
}

// ==============================================================================
// releaseResources
void ViaTransformerProcessor::releaseResources()
{
}

// ==============================================================================
// processBlock (DSP Core)
void ViaTransformerProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    midiMessages.clear();

    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0) return;

    // 1. Read the Turns Ratio parameter value (n = Ns/Np)
    auto turnsRatioParameter = apvts.getRawParameterValue("TURNS_RATIO");
    jassert(turnsRatioParameter != nullptr);
    float n = turnsRatioParameter->load();

    // 2. Calculate the gain in dB for the display: G_dB = 20 * log10(n)
    float gainDb = 20.0f * std::log10(juce::jmax(0.000001f, n));
    currentGainDB.store(gainDb, std::memory_order_relaxed);

    // 3. Set the linear gain (n) and apply the gain stage
    transformerGain.setGainLinear(n);

    juce::dsp::AudioBlock<float> block(buffer);
    transformerGain.process(juce::dsp::ProcessContextReplacing<float>(block));

    // The output signal is V_out = n * V_in
}

// ==============================================================================
// createEditor
juce::AudioProcessorEditor* ViaTransformerProcessor::createEditor()
{
    return new ViaTransformerProcessorEditor(*this, apvts);
}

// ==============================================================================
// State Information (Saving/Loading parameters)
void ViaTransformerProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void ViaTransformerProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

// juce_createPluginFilter
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViaTransformerProcessor();
}