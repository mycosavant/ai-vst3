#pragma once
#include <JuceHeader.h>
#include "MidiMapping.h"
#include "MidiLearnManager.h"

class MidiMappingRow : public juce::Component,
	public juce::Button::Listener
{
public:
	MidiMappingRow(const MidiMapping& mapping, MidiLearnManager* manager);
	~MidiMappingRow() override;

	void paint(juce::Graphics& g) override;
	void resized() override;
	void buttonClicked(juce::Button* button) override;

	std::function<void()> onDeleteClicked;
	std::function<void()> onEditClicked;
	std::function<void()> onLearnClicked;

	const MidiMapping& getMapping() const { return mapping; }
	void updateMapping(const MidiMapping& newMapping);

private:
	MidiMapping mapping;
	MidiLearnManager* midiLearnManager;

	juce::Label parameterLabel;
	juce::Label descriptionLabel;
	juce::Label midiInfoLabel;
	juce::TextButton deleteButton;
	juce::TextButton editButton;
	juce::TextButton learnButton;

	juce::String getMidiInfoString() const;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingRow)
};

class MidiMappingEditorWindow : public juce::DocumentWindow,
	public juce::Button::Listener,
	public juce::Timer
{
public:
	MidiMappingEditorWindow(MidiLearnManager* manager);
	~MidiMappingEditorWindow() override;

	void closeButtonPressed() override;
	void buttonClicked(juce::Button* button) override;
	void timerCallback() override;

	std::function<void()> onWindowClosed;

private:
	class MidiMappingEditorContent : public juce::Component,
		public juce::Button::Listener,
		public juce::ComboBox::Listener
	{
	public:
		MidiMappingEditorContent(MidiLearnManager* manager);
		~MidiMappingEditorContent() override;

		void paint(juce::Graphics& g) override;
		void resized() override;
		void buttonClicked(juce::Button* button) override;
		void comboBoxChanged(juce::ComboBox* comboBox) override;

		void refreshMappingsList();
		void deleteMapping(const MidiMapping& mapping);
		void startLearningForMapping(const MidiMapping& mapping);
		void editMapping(const MidiMapping& mapping);
		void exportMappings();
		void importMappings();
		void sortMappings();
		void filterMappings();

	private:
		MidiLearnManager* midiLearnManager;

		juce::Label titleLabel;
		juce::TextButton clearAllButton;
		juce::TextButton exportButton;
		juce::TextButton importButton;
		juce::TextButton refreshButton;

		juce::ComboBox sortComboBox;
		juce::Label sortLabel;
		juce::TextEditor searchBox;
		juce::Label searchLabel;

		juce::Viewport mappingsViewport;
		juce::Component mappingsContainer;
		juce::OwnedArray<MidiMappingRow> mappingRows;

		juce::String searchFilter;
		enum SortMode {
			SORT_PARAMETER_NAME,
			SORT_DESCRIPTION,
			SORT_MIDI_NUMBER,
			SORT_MIDI_CHANNEL
		};
		SortMode currentSortMode = SORT_PARAMETER_NAME;

		std::vector<MidiMapping> filteredMappings;

		std::unique_ptr<juce::FileChooser> fileChooser;

		void updateFilteredMappings();
		void createMappingRows();
		void showConfirmationDialog(const juce::String& message,
			std::function<void()> onConfirm);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorContent)
	};

	MidiLearnManager* midiLearnManager;
	std::unique_ptr<MidiMappingEditorContent> content;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorWindow)
};


class MidiMappingEditDialog : public juce::DialogWindow
{
public:
	MidiMappingEditDialog(MidiMapping& mapping, MidiLearnManager* manager);
	~MidiMappingEditDialog() override;

	void closeButtonPressed() override;

	std::function<void(const MidiMapping&)> onMappingUpdated;

private:
	class EditContent : public juce::Component,
		public juce::Button::Listener
	{
	public:
		EditContent(MidiMapping& mapping, MidiLearnManager* manager);
		~EditContent() override;

		void paint(juce::Graphics& g) override;
		void resized() override;
		void buttonClicked(juce::Button* button) override;

		MidiMapping getUpdatedMapping() const;

	private:
		MidiMapping& originalMapping;
		MidiLearnManager* midiLearnManager;

		juce::Label parameterLabel;
		juce::Label parameterValueLabel;

		juce::Label descriptionLabel;
		juce::TextEditor descriptionEditor;

		juce::Label midiChannelLabel;
		juce::Slider midiChannelSlider;

		juce::Label midiNumberLabel;
		juce::Slider midiNumberSlider;

		juce::Label midiTypeLabel;
		juce::ComboBox midiTypeComboBox;

		juce::TextButton applyButton;
		juce::TextButton cancelButton;
		juce::TextButton learnButton;

		void populateMidiTypeComboBox();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditContent)
	};

	MidiMapping& mapping;
	MidiLearnManager* midiLearnManager;
	std::unique_ptr<EditContent> content;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditDialog)
};