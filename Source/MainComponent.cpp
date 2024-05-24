#include "MainComponent.h"

//==============================================================================



MainComponent::MainComponent() {
    addAndMakeVisible(openMidiButton);
    addAndMakeVisible(openSf2Button);
    addAndMakeVisible(playButton);
    addAndMakeVisible(pauseButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(renderButton);

    openMidiButton.onClick = [this] { openMidiFile(); };
    openSf2Button.onClick = [this] { openSoundFont(); };
    playButton.onClick = [this] { midiPlayer.startPlayback(); };
    pauseButton.onClick = [this] { midiPlayer.pausePlayback(); };
    stopButton.onClick = [this] { midiPlayer.stopPlayback(); };
    renderButton.onClick = [this] { renderToWav(); };

    setSize(600, 400);
}

void MainComponent::resized() {
    openMidiButton.setBounds(10, 10, getWidth() - 20, 40);
    openSf2Button.setBounds(10, 60, getWidth() - 20, 40);
    playButton.setBounds(10, 110, getWidth() - 20, 40);
    pauseButton.setBounds(10, 160, getWidth() - 20, 40);
    stopButton.setBounds(10, 210, getWidth() - 20, 40);
    renderButton.setBounds(10, 260, getWidth() - 20, 40);
}

void MainComponent::openMidiFile() {
    juce::FileChooser chooser("Select a MIDI file...", {}, "*.mid");
    if (chooser.browseForFileToOpen()) {
        midiPlayer.loadMidiFile(chooser.getResult());
    }
}

void MainComponent::openSoundFont() {
    juce::FileChooser chooser("Select a SoundFont file...", {}, "*.sf2");
    if (chooser.browseForFileToOpen()) {
        midiPlayer.loadSoundFont(chooser.getResult());
    }
}

void MainComponent::renderToWav() {
    juce::FileChooser chooser("Save WAV file as...", {}, "*.wav");
    if (chooser.browseForFileToSave(true)) {
        midiPlayer.renderToWav(chooser.getResult());
    }
}
