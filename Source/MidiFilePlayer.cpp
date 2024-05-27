/*
  ==============================================================================

    MidiFilePlayer.h
    Created: 17 May 2024 7:09:53pm
    Author:  JOEL

  ==============================================================================
*/


#include "MidiFilePlayer.h"
#include "Fluidlite/src/fluidsynth_priv.h"


MidiFilePlayer::MidiFilePlayer()
    : settings(new_fluid_settings(), delete_fluid_settings),
      synth(new_fluid_synth(settings.get()), delete_fluid_synth) {
    setAudioChannels(0, 2); // no inputs, stereo output
}

MidiFilePlayer::~MidiFilePlayer() {
    shutdownAudio();
}

void MidiFilePlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    initializeFluidSynth(sampleRate);
}

void MidiFilePlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    bufferToFill.clearActiveBufferRegion();
    fluid_synth_write_float(synth.get(), bufferToFill.numSamples,
        bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample), 0, 1,
        bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample), 0, 1);
}

void MidiFilePlayer::releaseResources() {
    // Clean up resources
}

void MidiFilePlayer::loadSoundFont(const juce::File& sf2File) {
    if (fluid_synth_sfload(synth.get(), sf2File.getFullPathName().toRawUTF8(), 1) == FLUID_FAILED) {
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

    while (currentEventIndex < midiSequence.getNumEvents()) {
        auto message = midiSequence.getEventPointer(currentEventIndex)->message;
        if (message.getTimeStamp() > playHead)
            break;

        if (message.isNoteOn()) {
            fluid_synth_noteon(synth.get(), 0, message.getNoteNumber(), message.getVelocity());
        }
        else if (message.isNoteOff()) {
            fluid_synth_noteoff(synth.get(), 0, message.getNoteNumber());
        }
        currentEventIndex++;
    }

    playHead += 0.01;
}

void MidiFilePlayer::initializeFluidSynth(double sampleRate) {
    fluid_synth_set_sample_rate(synth.get(), sampleRate);
}

void MidiFilePlayer::renderToWav(const juce::File& wavFile) {
    juce::WavAudioFormat wavFormat;
    std::shared_ptr<juce::FileOutputStream> fileStream(wavFile.createOutputStream());
    if (!fileStream || fileStream->failedToOpen()) {
        juce::Logger::writeToLog("Failed to create output file.");
        return;
    }


    const double sampleRate = 44100.0;
    const int bufferSize = 512; // Adjust buffer size as needed
    juce::AudioBuffer<float> buffer(2, bufferSize);
    buffer.clear();

    double renderTime = 0.0;
    currentEventIndex = 0;
    playHead = 0.0;

    while (currentEventIndex < midiSequence.getNumEvents()) {
        buffer.clear();

        while (currentEventIndex < midiSequence.getNumEvents()) {
            auto message = midiSequence.getEventPointer(currentEventIndex)->message;
            if (message.getTimeStamp() > renderTime)
                break;

            if (message.isNoteOn()) {
                fluid_synth_noteon(synth.get(), 0, message.getNoteNumber(), message.getVelocity());
            }
            else if (message.isNoteOff()) {
                fluid_synth_noteoff(synth.get(), 0, message.getNoteNumber());
            }
            currentEventIndex++;
        }

        fluid_synth_write_float(synth.get(), bufferSize,
            buffer.getWritePointer(0), 0, 1,
            buffer.getWritePointer(1), 0, 1);

    

        renderTime += static_cast<double>(bufferSize) / sampleRate;
    }

    //writer->flush();
    fileStream->flush();
    juce::Logger::writeToLog("Rendering completed.");
}

bool MidiFilePlayer::isReadyToRender() const {
    return midiSequence.getNumEvents() > 0;
}
