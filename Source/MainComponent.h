#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "SoundfontAudioSource.h"
#include "MidiFilePlayer.h"

//==============================================================================
class MainComponent : public AudioAppComponent,
    public ComboBox::Listener,
    public Button::Listener
{
public:
    MainComponent();
    ~MainComponent();

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(Graphics& g) override;
    void resized() override;

    void comboBoxChanged(ComboBox* comboBoxThatWasChanged) override;
    void buttonClicked(Button* buttonThatWasClicked) override;

private:
    void loadMidiFile(const File& file);
    void exportMidiAsWav(const File& midiFile, const File& wavFile);
    void exportMidiAsWavWithSoundfont(const File& midiFile, const File& soundfontFile, const File& wavFile);

    File getSoundfontsDirectory() const;
    File getFileFromName(const String& name) const;

    SoundfontAudioSource soundfontPlayer;
    MidiMessageSequence midiSequence;
    ScopedPointer<MidiFilePlayer> midiPlayer;
    Array<File> soundfontFiles;
    StringArray soundfontNames;

    ScopedPointer<ComboBox> soundfontSelector;
    ScopedPointer<TextButton> playButton;
    ScopedPointer<TextButton> pauseButton;
    ScopedPointer<TextButton> stopButton;
    ScopedPointer<TextButton> loadMidiButton;
    ScopedPointer<TextButton> exportWavButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
