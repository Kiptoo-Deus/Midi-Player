#include "MidiFilePlayer.h"

//==============================================================================

// MidiFilePlayer.cpp
#include "MidiFilePlayer.h"
#include "Fluidlite/src/fluidsynth_priv.h"


MidiFilePlayer::MidiFilePlayer() {
    initializeFluidSynth();
    setAudioChannels(0, 2); // no inputs, stereo output
}

MidiFilePlayer::~MidiFilePlayer() {
    shutdownAudio();
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void MidiFilePlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    fluid_synth_set_sample_rate(synth, sampleRate);
}

void MidiFilePlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    bufferToFill.clearActiveBufferRegion();
    fluid_synth_write_float(synth, bufferToFill.numSamples,
        bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), 0, 1,
        bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), 0, 1);
}

void MidiFilePlayer::releaseResources() {
    // Clean up resources
}

void MidiFilePlayer::loadSoundFont(const juce::File& sf2File) {
    if (fluid_synth_sfload(synth, sf2File.getFullPathName().toRawUTF8(), 1) == FLUID_FAILED) {
        juce::Logger::writeToLog("Failed to load SoundFont.");
    }
}

void MidiFilePlayer::loadMidiFile(const juce::File& midiFile) {
    juce::FileInputStream midiStream(midiFile);
    if (!midiStream.openedOk()) {
        juce::Logger::writeToLog("Failed to open MIDI file.");
        return;
    }

    juce::MidiFile midi;
    midi.readFrom(midiStream);
    midi.convertTimestampTicksToSeconds();

    juce::MidiMessageSequence sequence;
    for (int i = 0; i < midi.getNumTracks(); ++i) {
        sequence.addSequence(*midi.getTrack(i), 0.0);
    }

    midiSequence = sequence;
    currentEventIndex = 0;
    playHead = 0.0;
}

void MidiFilePlayer::startPlayback() {
    if (!isPlaying || isPaused) {
        startTimer(10);
        isPlaying = true;
        isPaused = false;
    }
}

void MidiFilePlayer::stopPlayback() {
    stopTimer();
    isPlaying = false;
    isPaused = false;
    currentEventIndex = 0;
    playHead = 0.0;
}

void MidiFilePlayer::pausePlayback() {
    if (isPlaying && !isPaused) {
        stopTimer();
        isPaused = true;
    }
}

void MidiFilePlayer::timerCallback() {
    if (!isPlaying || midiSequence.getNumEvents() == 0)
        return;

    auto message = midiSequence.getEventPointer(currentEventIndex)->message;
    if (message.getTimeStamp() <= playHead) {
        if (message.isNoteOn()) {
            fluid_synth_noteon(synth, 0, message.getNoteNumber(), message.getVelocity());
        }
        else if (message.isNoteOff()) {
            fluid_synth_noteoff(synth, 0, message.getNoteNumber());
        }
        currentEventIndex++;
    }
    playHead += 0.01;
}

void MidiFilePlayer::initializeFluidSynth() {
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);
}

