#include "MidiFilePlayer.h"
#include "Fluidlite/src/fluidsynth_priv.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>


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

    if (currentEventIndex >= midiSequence.getNumEvents()) {
        stopPlayback(); // I'm Stopping playback if the end of the sequence is reached
        return;
    }

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
    if (settings == nullptr) {
        juce::Logger::writeToLog("Failed to create FluidSynth settings.");
        return;
    }

    synth = new_fluid_synth(settings);
    if (synth == nullptr) {
        juce::Logger::writeToLog("Failed to create FluidSynth synthesizer.");
        delete_fluid_settings(settings);
        settings = nullptr;
    }
}

void MidiFilePlayer::muteAudio() {
    shutdownAudio(); // This will stop audio processing
}

void MidiFilePlayer::unmuteAudio() {
    setAudioChannels(0, 2); // Reinitialize audio channels with no inputs, stereo output
}

void MidiFilePlayer::exportToWavSilent(const juce::File& outputFile) {
    try {
        muteAudio(); // Mute the audio output during rendering

        const double sampleRate = 44100.0; 
        const int blockSize = 512; 
        const int numChannels = 2; // Stereo output

        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        juce::WavAudioFormat wavFormat;

        std::unique_ptr<juce::FileOutputStream> fileStream(outputFile.createOutputStream());
        if (fileStream == nullptr || !fileStream->openedOk()) {
            juce::Logger::writeToLog("Failed to open output file for writing.");
            return;
        }

        std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(fileStream.get(), sampleRate, numChannels, 16, {}, 0));
        if (writer == nullptr) {
            juce::Logger::writeToLog("Failed to create WAV writer.");
            return;
        }

        juce::AudioBuffer<float> buffer(numChannels, blockSize);

        double currentTime = 0.0;
        currentEventIndex = 0;

        while (currentEventIndex < midiSequence.getNumEvents()) {
            buffer.clear();

            for (int i = 0; i < blockSize; ++i) {
                if (currentEventIndex >= midiSequence.getNumEvents()) break;

                auto message = midiSequence.getEventPointer(currentEventIndex)->message;
                if (message.getTimeStamp() <= currentTime) {
                    if (message.isNoteOn()) {
                        fluid_synth_noteon(synth, 0, message.getNoteNumber(), message.getVelocity());
                    }
                    else if (message.isNoteOff()) {
                        fluid_synth_noteoff(synth, 0, message.getNoteNumber());
                    }
                    currentEventIndex++;
                }
                currentTime += 1.0 / sampleRate;

                fluid_synth_write_float(synth, 1,
                    buffer.getWritePointer(0, i), 0, 1,
                    buffer.getWritePointer(1, i), 0, 1);
            }

            writer->writeFromAudioSampleBuffer(buffer, 0, blockSize);
        }

        writer.reset(); // Ensure the writer is deleted and the file is closed correctly
        juce::Logger::writeToLog("Finished exporting to WAV.");

        unmuteAudio(); // Unmute the audio output after rendering is done
    }
    catch (const std::exception& e) {
        juce::Logger::writeToLog("Exception during WAV export: " + juce::String(e.what()));
    }
    catch (...) {
        juce::Logger::writeToLog("Unknown exception during WAV export.");
    }
}

void MidiFilePlayer::startExportToWavInBackground(const juce::File& outputFile) {
    juce::Thread::launch([this, outputFile]() {
        exportToWavSilent(outputFile);
        });
}
