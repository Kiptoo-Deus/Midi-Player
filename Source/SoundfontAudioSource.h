#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Fluidlite/include/fluidlite.h"

//==============================================================================
class SoundfontAudioSource : public AudioSource
{
public:
    SoundfontAudioSource(int numberOfVoices = 256);
    ~SoundfontAudioSource();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    bool loadSoundfont(const File file);
    void processMidi(const MidiMessage& message);

private:
    CriticalSection lock;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    int sfontID;
    File loadedSoundfont;
};
