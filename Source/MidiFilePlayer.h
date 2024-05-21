/*
  ==============================================================================

    MidiFilePlayer.h
    Created: 17 May 2024 7:09:53pm
    Author:  JOEL

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SoundfontAudioSource.h"

class MidiFilePlayer : public Timer
{
public:
    MidiFilePlayer(const MidiMessageSequence& sequence, SoundfontAudioSource* soundfontSource);

    void start();
    void pause();
    void stop();
    void setPosition(double positionInSeconds);

private:
    void timerCallback() override;

    const MidiMessageSequence& midiSequence;
    SoundfontAudioSource* soundfontSource;
    double startTime;
    double pausedTime;
    bool isPlaying;
};
