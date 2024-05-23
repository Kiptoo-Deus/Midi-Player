#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "MidiFilePlayer.h"

//==============================================================================

#pragma once

#include "JuceHeader.h"
#include "MidiFilePlayer.h"

#pragma once

#include "JuceHeader.h"
#include "MidiFilePlayer.h"

class MainComponent : public juce::Component {
public:
    MainComponent();
    ~MainComponent() override = default;

    void resized() override;

private:
    void openMidiFile();
    void openSoundFont();

    juce::TextButton openMidiButton{ "Open MIDI File" };
    juce::TextButton openSf2Button{ "Open SoundFont File" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton pauseButton{ "Pause" };
    juce::TextButton stopButton{ "Stop" };

    MidiFilePlayer midiPlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
