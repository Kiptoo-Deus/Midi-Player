#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // I'm Populating Soundfonts
    soundfontFiles = getSoundfontsDirectory().findChildFiles(File::findFiles, false, "*.sf2");
    for (auto f : soundfontFiles) {
        soundfontNames.add(f.getFileNameWithoutExtension());
    }

    addAndMakeVisible(soundfontSelector = new ComboBox("SoundfontSelector"));
    soundfontSelector->addListener(this);
    soundfontSelector->addItemList(soundfontNames, 1);
    soundfontSelector->setSelectedItemIndex(0);

    addAndMakeVisible(playButton = new TextButton("Play"));
    playButton->addListener(this);

    addAndMakeVisible(pauseButton = new TextButton("Pause"));
    pauseButton->addListener(this);

    addAndMakeVisible(stopButton = new TextButton("Stop"));
    stopButton->addListener(this);

    addAndMakeVisible(loadMidiButton = new TextButton("Load MIDI"));
    loadMidiButton->addListener(this);

    setSize(800, 200);
    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    soundfontPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    soundfontPlayer.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    soundfontPlayer.releaseResources();
}

//==============================================================================
void MainComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    const int selectorHeight = 50;
    const int buttonHeight = 30;

    soundfontSelector->setBounds(0, 0, getWidth(), selectorHeight);
    playButton->setBounds(10, selectorHeight + 10, 70, buttonHeight);
    pauseButton->setBounds(90, selectorHeight + 10, 70, buttonHeight);
    stopButton->setBounds(170, selectorHeight + 10, 70, buttonHeight);
    loadMidiButton->setBounds(250, selectorHeight + 10, 100, buttonHeight);
}

void MainComponent::comboBoxChanged(ComboBox* comboBoxThatWasChanged)
{
    if (comboBoxThatWasChanged == soundfontSelector) {
        File soundfontFile = getFileFromName(soundfontSelector->getText());
        if (!soundfontPlayer.loadSoundfont(soundfontFile)) {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Invalid .sf2 File",
                "Could not find your soundfont file: " + soundfontFile.getFullPathName());
        }
    }
}

void MainComponent::buttonClicked(Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == playButton) {
        if (midiPlayer != nullptr) {
            midiPlayer->start();
        }
    }
    else if (buttonThatWasClicked == pauseButton) {
        if (midiPlayer != nullptr) {
            midiPlayer->pause();
        }
    }
    else if (buttonThatWasClicked == stopButton) {
        if (midiPlayer != nullptr) {
            midiPlayer->stop();
        }
    }
    else if (buttonThatWasClicked == loadMidiButton) {
        FileChooser chooser("Select a MIDI file...", {}, "*.mid");
        if (chooser.browseForFileToOpen()) {
            loadMidiFile(chooser.getResult());
        }
    }
}

void MainComponent::loadMidiFile(const File& file)
{
    FileInputStream inputStream(file);
    MidiFile midiFile;
    if (midiFile.readFrom(inputStream)) {
        midiSequence.clear();
        for (int i = 0; i < midiFile.getNumTracks(); ++i) {
            midiSequence.addSequence(*midiFile.getTrack(i), 0.0);
        }
        midiPlayer = new MidiFilePlayer(midiSequence, &soundfontPlayer);
    }
}


File MainComponent::getSoundfontsDirectory() const
{
    // The soundfonts are stored next to the Source folder that contains this file
    return File(String(__FILE__))
        .getParentDirectory()
        .getParentDirectory()
        .getChildFile("Soundfonts");
}

File MainComponent::getFileFromName(const String& name) const
{
    for (auto f : soundfontFiles) {
        if (f.getFileNameWithoutExtension() == name) {
            return f;
        }
    }
    return File();
}
