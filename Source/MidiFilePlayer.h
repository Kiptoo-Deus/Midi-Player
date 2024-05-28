#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"

class MidiFilePlayer : public juce::AudioAppComponent {
public:
    MidiFilePlayer();
    ~MidiFilePlayer() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadSoundFont(const juce::File& sf2File);
    void loadMidiFile(const juce::File& midiFile);
    void renderToWav(const juce::File& wavFile);

    bool isReadyToRender() const;

    void startPlayback();
    void pausePlayback();
    void stopPlayback();

private:
    std::unique_ptr<fluid_settings_t, decltype(&delete_fluid_settings)> settings;
    std::unique_ptr<fluid_synth_t, decltype(&delete_fluid_synth)> synth;
    juce::MidiMessageSequence midiSequence;
    int currentEventIndex = 0;
    double playHead = 0.0;
    bool isPlaying = false;
    bool isPaused = false;

    void initializeFluidSynth(double sampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiFilePlayer)
};

