/*
  ==============================================================================

    MidiFilePlayer.h
    Created: 17 May 2024 7:09:53pm
    Author:  JOEL

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"



class MidiFilePlayer : public juce::AudioAppComponent, private juce::Timer {
public:
    MidiFilePlayer();
    ~MidiFilePlayer() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadSoundFont(const juce::File& sf2File);
    void loadMidiFile(const juce::File& midiFile);
    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    void exportToWav(const juce::File& outputFile);

private:
    void timerCallback() override;

    fluid_settings_t* settings;
    fluid_synth_t* synth;
    juce::MidiMessageSequence midiSequence;
    int currentEventIndex = 0;
    double playHead = 0.0;
    bool isPlaying = false;
    bool isPaused = false;

    void initializeFluidSynth();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiFilePlayer)
};
