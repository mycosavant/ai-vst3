#pragma once
#include "JuceHeader.h"
#include "TrackManager.h"
#include "MidiLearnableComponents.h"
#include "ColourPalette.h"

class WaveformDisplay;
class SequencerComponent;
class DjIaVstProcessor;

class CustomInfoLabelLookAndFeel : public juce::LookAndFeel_V4
{
public:
	void drawLabel(juce::Graphics& g, juce::Label& label) override
	{
		auto bounds = label.getLocalBounds().toFloat();
		g.setColour(ColourPalette::backgroundDeep);
		g.fillRoundedRectangle(bounds, 4.0f);
		g.setColour(ColourPalette::textAccent.withAlpha(0.4f));
		g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
		g.setColour(ColourPalette::textAccent);
		g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::plain));
		g.drawText(label.getText(), bounds.reduced(8, 2),
			juce::Justification::centredLeft, false);
	}
};

class TrackComponent : public juce::Component, public juce::Timer, public juce::AudioProcessorParameter::Listener, public juce::DragAndDropTarget
{
public:
	TrackComponent(const juce::String& trackId, DjIaVstProcessor& processor);
	~TrackComponent();
	juce::String getTrackId() const
	{
		return trackId;
	}

	std::function<void(const juce::String&)> onDeleteTrack;
	std::function<void(const juce::String&)> onSelectTrack;
	std::function<void(const juce::String&)> onGenerateForTrack;
	std::function<void(const juce::String&, const juce::String&)> onTrackRenamed;
	std::function<void(const juce::String&, const juce::String&)> onTrackPromptChanged;
	std::function<void(const juce::String&)> onStatusMessage;

	bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
	void itemDragEnter(const SourceDetails& dragSourceDetails) override;
	void itemDragMove(const SourceDetails& dragSourceDetails) override;
	void itemDragExit(const SourceDetails& dragSourceDetails) override;
	void itemDropped(const SourceDetails& dragSourceDetails) override;

	static const int BASE_HEIGHT = 60;
	static const int WAVEFORM_HEIGHT = 100;
	static const int SEQUENCER_HEIGHT = 100;

	juce::TextButton showWaveformButton;
	juce::TextButton sequencerToggleButton;

	TrackData* getTrack() const { return track; }

	std::function<void(const juce::String&, const juce::String&)> onReorderTrack;
	std::function<void(const juce::String&)> onPreviewTrack;

	void setSelected(bool selected);
	void setTrackData(TrackData* trackData);
	void refreshWaveformDisplay();
	bool isWaveformVisible() const;
	void startGeneratingAnimation();
	void stopGeneratingAnimation();
	void updateFromTrackData();
	void setGenerateButtonEnabled(bool enabled);
	void updateWaveformWithTimeStretch();
	void updatePlaybackPosition(double timeInSeconds);
	void toggleWaveformDisplay();
	void refreshWaveformIfNeeded();
	void toggleSequencerDisplay();
	void updatePromptPresets(const juce::StringArray& presets);
	void setupMidiLearn();
	void updatePromptSelection(const juce::String& promptText);
	void onPageSelected(int pageIndex);
	void performPageChange(int pageIndex);
	void updatePagesDisplay();

	bool isEditingLabel = false;
	MidiLearnableComboBox promptPresetSelector;

	juce::TextButton* getGenerateButton() { return &generateButton; }
	juce::Slider* getBpmOffsetSlider() { return &bpmOffsetSlider; }

	SequencerComponent* getSequencer() const { return sequencer.get(); }

private:
	juce::String trackId;
	TrackData* track;
	bool isSelected = false;
	std::unique_ptr<WaveformDisplay> waveformDisplay;
	std::unique_ptr<SequencerComponent> sequencer;
	DjIaVstProcessor& audioProcessor;
	CustomInfoLabelLookAndFeel customLookAndFeel;
	juce::Label trackNumberLabel;
	bool isDragOver = false;
	juce::TextButton selectButton;
	juce::Label trackNameLabel;
	juce::TextButton deleteButton;
	MidiLearnableButton generateButton;
	juce::Label infoLabel;
	juce::TextButton previewButton;
	juce::TextButton originalSyncButton;

	juce::StringArray promptPresets;

	juce::ComboBox timeStretchModeSelector;

	MidiLearnableButton randomRetriggerButton;
	MidiLearnableSlider intervalKnob;
	juce::Label intervalLabel;

	juce::ToggleButton randomDurationToggle;

	juce::Slider bpmOffsetSlider;
	juce::Label bpmOffsetLabel;

	std::atomic<bool> isDestroyed{ false };

	bool isGenerating = false;
	bool blinkState = false;
	bool sequencerVisible = false;

	MidiLearnableButton pageButtons[4];
	juce::TextButton togglePagesButton;
	bool pagesMode = false;
	static const int PAGE_BUTTON_SIZE = 14;

	bool pageBlinkState = false;

	void setupPagesUI();
	void onTogglePagesMode();
	void loadPageIfNeeded(int pageIndex);
	void loadPageAudioFile(int pageIndex, const juce::File& audioFile);
	void layoutPagesButtons(juce::Rectangle<int> area);
	void calculateHostBasedDisplay();
	void paint(juce::Graphics& g);
	void resized();
	void timerCallback() override;
	void parameterValueChanged(int parameterIndex, float newValue) override;
	void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;
	void setupUI();
	void adjustLoopPointsToTempo();
	void updateTrackInfo();
	void learn(juce::String param, std::function<void(float)> uiCallback = nullptr);
	void removeMidiMapping(const juce::String& param);
	void addListener(juce::String name);
	void removeListener(juce::String name);
	void setButtonParameter(juce::String name);
	void updateUIFromParameter(const juce::String& paramName,
		const juce::String& slotPrefix,
		float newValue);
	void loadPromptPresets();
	void onTrackPresetSelected();
	void toggleOriginalSync();
	void statusCallback(const juce::String& message);
	void onRandomRetriggerToggled();
	void onIntervalChanged();
	void setSliderParameter(juce::String name, juce::Slider& slider);
	void addEventListeners();

	float calculateEffectiveBpm();

	juce::String getIntervalName(int value);

	JUCE_DECLARE_WEAK_REFERENCEABLE(TrackComponent);
};