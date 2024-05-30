/*
  ==============================================================================

    MidiFilePlayer.h
    Created: 17 May 2024 7:09:53pm
    Author:  JOEL

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "Fluidlite/src/fluidsynth_priv.h"

class MidiFilePlayer : public juce::AudioAppComponent, private juce::Timer {
public:
    MidiFilePlayer();
    ~MidiFilePlayer();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadSoundFont(const juce::File& sf2File);
    void loadMidiFile(const juce::File& midiFile);

    void startPlayback();
    void stopPlayback();
    void pausePlayback();
    void exportToWavSilent(const juce::File& outputFile);
    void startExportToWavInBackground(const juce::File& outputFile);

    void muteAudio();
    void unmuteAudio();

private:
    void initializeFluidSynth();
    void timerCallback() override;
    void turnOffAllNotes(); 

    double lastTime = 0.0;
    fluid_settings_t* settings = nullptr;
    fluid_synth_t* synth = nullptr;
    juce::MidiMessageSequence midiSequence;
    int currentEventIndex = 0;
    double playHead = 0.0;
    double sampleRate = 44100.0; // Default sample rate
    bool isPlaying = false;
    bool isPaused = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiFilePlayer)
};

