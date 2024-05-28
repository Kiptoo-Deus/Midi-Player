#include "MidiFilePlayer.h"

#ifndef FLUID_FAILED
#define FLUID_FAILED (-1)
#endif

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
    if (!isPlaying || isPaused) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

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
    isPlaying = false;
    isPaused = false;
}

void MidiFilePlayer::renderToWav(const juce::File& file)
{
    auto outFile = file.createOutputStream();

    if (outFile == nullptr)
    {
        DBG("Failed to create output file stream.");
        return;
    }

    juce::WavAudioFormat wavFormat;
    auto audioWriter = std::unique_ptr<juce::AudioFormatWriter>(
        wavFormat.createWriterFor(outFile.release(),
                                  44100,
                                  2,
                                  16,
                                  {},
                                  0)
    );

    if (audioWriter == nullptr)
    {
        DBG("Failed to create WAV writer.");
        return;
    }

    juce::AudioSourceChannelInfo info;
    info.buffer = new juce::AudioBuffer<float>(2, 44100 * 60);  // 1 minute buffer
    info.startSample = 0;
    info.numSamples = info.buffer->getNumSamples();

    // Assume renderNextBlock is a method that renders audio into the buffer
    //renderNextBlock(*info.buffer, 0, info.numSamples);

    audioWriter->writeFromAudioSampleBuffer(*info.buffer, 0, info.numSamples);

    delete info.buffer;  // Clean up the buffer
}


bool MidiFilePlayer::isReadyToRender() const {
    return midiSequence.getNumEvents() > 0;
}

void MidiFilePlayer::startPlayback() {
    isPlaying = true;
    isPaused = false;
}

void MidiFilePlayer::pausePlayback() {
    isPaused = true;
}

void MidiFilePlayer::stopPlayback() {
    isPlaying = false;
    isPaused = false;
    currentEventIndex = 0;
    playHead = 0.0;
}

void MidiFilePlayer::initializeFluidSynth(double sampleRate) {
    fluid_synth_set_sample_rate(synth.get(), sampleRate);
}

