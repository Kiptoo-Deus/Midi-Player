#include "MidiFilePlayer.h"

//==============================================================================
MidiFilePlayer::MidiFilePlayer(const MidiMessageSequence& sequence, SoundfontAudioSource* soundfontSource)
    : midiSequence(sequence), soundfontSource(soundfontSource), startTime(0), pausedTime(0), isPlaying(false)
{
    startTimer(1); 
}

void MidiFilePlayer::start()
{
    if (!isPlaying) {
        startTime = Time::getMillisecondCounterHiRes() - pausedTime;
        isPlaying = true;
    }
}

void MidiFilePlayer::pause()
{
    if (isPlaying) {
        pausedTime = Time::getMillisecondCounterHiRes() - startTime;
        isPlaying = false;
    }
}

void MidiFilePlayer::stop()
{
    isPlaying = false;
    pausedTime = 0;
}

void MidiFilePlayer::setPosition(double positionInSeconds)
{
    startTime = Time::getMillisecondCounterHiRes() - positionInSeconds * 1000.0;
}

void MidiFilePlayer::timerCallback()
{
    if (!isPlaying) {
        return;
    }

    const double currentTime = (Time::getMillisecondCounterHiRes() - startTime) / 1000.0;
    for (int i = 0; i < midiSequence.getNumEvents(); ++i) {
        const MidiMessageSequence::MidiEventHolder* event = midiSequence.getEventPointer(i);
        if (event->message.getTimeStamp() <= currentTime) {
            soundfontSource->processMidi(event->message);
        }
    }
}
