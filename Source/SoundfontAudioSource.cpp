#include "SoundfontAudioSource.h"

//==============================================================================
SoundfontAudioSource::SoundfontAudioSource(int numberOfVoices)
{
    settings = new_fluid_settings();
    synth = new_fluid_synth(settings);

    fluid_synth_set_polyphony(synth, numberOfVoices);
}

SoundfontAudioSource::~SoundfontAudioSource()
{
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

void SoundfontAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    fluid_synth_set_sample_rate(synth, (float)sampleRate);
}

void SoundfontAudioSource::releaseResources()
{
    fluid_synth_system_reset(synth);
}

void SoundfontAudioSource::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    const ScopedLock l(lock);

    fluid_synth_write_float(synth,
        bufferToFill.buffer->getNumSamples(),
        bufferToFill.buffer->getWritePointer(0), 0, 1,
        bufferToFill.buffer->getWritePointer(1), 0, 1);
}

bool SoundfontAudioSource::loadSoundfont(const File file)
{
    if (file == loadedSoundfont) {
        return false;
    }
    loadedSoundfont = file;

    const ScopedLock l(lock);

    fluid_synth_system_reset(synth);

    if (fluid_synth_sfcount(synth) > 0) {
        int err = fluid_synth_sfunload(synth, (unsigned int)sfontID, true);
        if (err == -1) {
            return false;
        }
    }

    sfontID = fluid_synth_sfload(synth, file.getFullPathName().toRawUTF8(), true);
    return sfontID != -1;
}

void SoundfontAudioSource::processMidi(const MidiMessage& message)
{
    if (message.isNoteOn()) {
        fluid_synth_noteon(synth, message.getChannel(), message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff()) {
        fluid_synth_noteoff(synth, message.getChannel(), message.getNoteNumber());
    }
    else if (message.isController()) {
        fluid_synth_cc(synth, message.getChannel(), message.getControllerNumber(), message.getControllerValue());
    }
    else if (message.isPitchWheel()) {
        fluid_synth_pitch_bend(synth, message.getChannel(), message.getPitchWheelValue());
    }
    else if (message.isChannelPressure()) {
        fluid_synth_channel_pressure(synth, message.getChannel(), message.getChannelPressureValue());
    }
}
