#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiFilePlayer.h"

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "MidiFilePlayer.h"

class MainComponent : public juce::Component {
public:
    MainComponent();
    ~MainComponent() override = default;

    void resized() override;

private:
    void openMidiFile();
    void openSoundFont();
    void renderToWav();
    void updateRenderButtonState();

    juce::TextButton openMidiButton{ "Open MIDI File" };
    juce::TextButton openSf2Button{ "Open SoundFont File" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton pauseButton{ "Pause" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton renderButton{ "Render to WAV" };

    std::unique_ptr<MidiFilePlayer> midiPlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
