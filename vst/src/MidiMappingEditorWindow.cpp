#include "MidiMappingEditorWindow.h"
#include "ColourPalette.h"


MidiMappingRow::MidiMappingRow(const MidiMapping& mapping, MidiLearnManager* manager)
	: mapping(mapping), midiLearnManager(manager)
{
	parameterLabel.setText(mapping.parameterName, juce::dontSendNotification);
	parameterLabel.setJustificationType(juce::Justification::centredLeft);
	parameterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
	addAndMakeVisible(parameterLabel);

	descriptionLabel.setText(mapping.description, juce::dontSendNotification);
	descriptionLabel.setJustificationType(juce::Justification::centredLeft);
	descriptionLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
	addAndMakeVisible(descriptionLabel);

	midiInfoLabel.setText(getMidiInfoString(), juce::dontSendNotification);
	midiInfoLabel.setJustificationType(juce::Justification::centredLeft);
	midiInfoLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
	addAndMakeVisible(midiInfoLabel);

	deleteButton.setButtonText("Delete");
	deleteButton.addListener(this);
	deleteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
	addAndMakeVisible(deleteButton);

	editButton.setButtonText("Edit");
	editButton.addListener(this);
	addAndMakeVisible(editButton);

	learnButton.setButtonText("Re-Learn");
	learnButton.addListener(this);
	learnButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
	addAndMakeVisible(learnButton);

	setSize(800, 50);
}

MidiMappingRow::~MidiMappingRow()
{
}

void MidiMappingRow::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::transparentBlack);

	g.setColour(juce::Colours::darkgrey);
	g.drawLine(0, getHeight() - 1, getWidth(), getHeight() - 1, 0.5f);
}

void MidiMappingRow::resized()
{
	auto bounds = getLocalBounds().reduced(5);
	auto buttonArea = bounds.removeFromRight(250);
	deleteButton.setBounds(buttonArea.removeFromRight(80).reduced(2));
	learnButton.setBounds(buttonArea.removeFromRight(80).reduced(2));
	editButton.setBounds(buttonArea.removeFromRight(80).reduced(2));
	auto labelArea = bounds;
	parameterLabel.setBounds(labelArea.removeFromLeft(200));
	midiInfoLabel.setBounds(labelArea.removeFromLeft(150));
	descriptionLabel.setBounds(labelArea);
}


void MidiMappingRow::buttonClicked(juce::Button* button)
{
	if (button == &deleteButton && onDeleteClicked)
		onDeleteClicked();
	else if (button == &editButton && onEditClicked)
		onEditClicked();
	else if (button == &learnButton && onLearnClicked)
		onLearnClicked();
}

juce::String MidiMappingRow::getMidiInfoString() const
{
	juce::String typeStr;

	switch (mapping.midiType)
	{
	case 1: typeStr = "CC"; break;
	case 0: typeStr = "Note"; break;
	case 2: typeStr = "Pitch Bend"; break;
	default: typeStr = "Unknown"; break;
	}

	return typeStr + " "
		+ juce::String(mapping.midiNumber)
		+ " (Ch " + juce::String(mapping.midiChannel + 1) + ")";
}



void MidiMappingRow::updateMapping(const MidiMapping& newMapping)
{
	mapping = newMapping;
	parameterLabel.setText(mapping.parameterName, juce::dontSendNotification);
	descriptionLabel.setText(mapping.description, juce::dontSendNotification);
	midiInfoLabel.setText(getMidiInfoString(), juce::dontSendNotification);
	repaint();
}

MidiMappingEditorWindow::MidiMappingEditorContent::MidiMappingEditorContent(MidiLearnManager* manager)
	: midiLearnManager(manager)
{
	titleLabel.setText("MIDI Mappings Editor", juce::dontSendNotification);
	titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
	titleLabel.setJustificationType(juce::Justification::centred);
	titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
	addAndMakeVisible(titleLabel);

	clearAllButton.setButtonText("Clear All");
	clearAllButton.addListener(this);
	clearAllButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
	addAndMakeVisible(clearAllButton);

	exportButton.setButtonText("Export...");
	exportButton.addListener(this);
	addAndMakeVisible(exportButton);

	importButton.setButtonText("Import...");
	importButton.addListener(this);
	addAndMakeVisible(importButton);

	refreshButton.setButtonText("Refresh");
	refreshButton.addListener(this);
	addAndMakeVisible(refreshButton);

	sortLabel.setText("Sort by:", juce::dontSendNotification);
	sortLabel.setColour(juce::Label::textColourId, juce::Colours::white);
	addAndMakeVisible(sortLabel);

	sortComboBox.addItem("Parameter Name", 1);
	sortComboBox.addItem("Description", 2);
	sortComboBox.addItem("MIDI Number", 3);
	sortComboBox.addItem("MIDI Channel", 4);
	sortComboBox.setSelectedId(1);
	sortComboBox.addListener(this);
	addAndMakeVisible(sortComboBox);

	searchLabel.setText("Search:", juce::dontSendNotification);
	searchLabel.setColour(juce::Label::textColourId, juce::Colours::white);
	addAndMakeVisible(searchLabel);

	searchBox.setTextToShowWhenEmpty("Type to filter...", juce::Colours::grey);
	searchBox.onTextChange = [this] { filterMappings(); };
	addAndMakeVisible(searchBox);

	mappingsViewport.setViewedComponent(&mappingsContainer, false);
	mappingsViewport.setScrollBarsShown(true, false);
	addAndMakeVisible(mappingsViewport);

	addMappingButton.setButtonText("Add Mapping");
	addMappingButton.addListener(this);
	addMappingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
	addAndMakeVisible(addMappingButton);

	refreshMappingsList();
}

MidiMappingEditorWindow::MidiMappingEditorContent::~MidiMappingEditorContent()
{
}

void MidiMappingEditorWindow::MidiMappingEditorContent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xff1a1a1a));

	g.setColour(juce::Colours::darkgrey);
	g.drawRect(mappingsViewport.getBounds(), 1);
}

void MidiMappingEditorWindow::MidiMappingEditorContent::resized()
{
	auto bounds = getLocalBounds().reduced(10);

	titleLabel.setBounds(bounds.removeFromTop(40));

	bounds.removeFromTop(10);

	auto toolbarBounds = bounds.removeFromTop(35);


	addMappingButton.setBounds(toolbarBounds.removeFromLeft(100).reduced(2));
	toolbarBounds.removeFromLeft(5);

	clearAllButton.setBounds(toolbarBounds.removeFromLeft(100).reduced(2));
	toolbarBounds.removeFromLeft(5);
	exportButton.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));
	toolbarBounds.removeFromLeft(5);
	importButton.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));
	toolbarBounds.removeFromLeft(5);
	refreshButton.setBounds(toolbarBounds.removeFromLeft(80).reduced(2));

	auto rightTools = toolbarBounds.removeFromRight(400);
	searchLabel.setBounds(rightTools.removeFromRight(50));
	searchBox.setBounds(rightTools.removeFromRight(150).reduced(2));
	rightTools.removeFromRight(20);
	sortLabel.setBounds(rightTools.removeFromRight(50));
	sortComboBox.setBounds(rightTools.removeFromRight(120).reduced(2));

	bounds.removeFromTop(10);

	mappingsViewport.setBounds(bounds);
}

void MidiMappingEditorWindow::MidiMappingEditorContent::buttonClicked(juce::Button* button)
{
	if (button == &clearAllButton)
	{
		showConfirmationDialog("Are you sure you want to clear all MIDI mappings?",
			[this] {
				midiLearnManager->clearAllMappings();
				refreshMappingsList();
			});
	}
	else if (button == &exportButton)
	{
		exportMappings();
	}
	else if (button == &importButton)
	{
		importMappings();
	}
	else if (button == &refreshButton)
	{
		refreshMappingsList();
	}
	else if (button == &addMappingButton)
	{
		createNewMapping();
	}

}

void MidiMappingEditorWindow::MidiMappingEditorContent::createNewMapping()
{
	MidiMapping newMapping;
	newMapping.parameterName = "NewParameter";
	newMapping.description = "New MIDI mapping";
	newMapping.midiType = 1;
	newMapping.midiNumber = 0;
	newMapping.midiChannel = 0;
	newMapping.processor = midiLearnManager->getProcessor();

	auto* dialog = new MidiMappingEditDialog(newMapping, midiLearnManager, midiLearnManager->getProcessor());

	dialog->onMappingUpdated = [this](const MidiMapping& updated)
		{
			midiLearnManager->addMapping(updated);
			refreshMappingsList();
		};

	dialog->setVisible(true);
}


void MidiMappingEditorWindow::MidiMappingEditorContent::comboBoxChanged(juce::ComboBox* comboBox)
{
	if (comboBox == &sortComboBox)
	{
		currentSortMode = static_cast<SortMode>(sortComboBox.getSelectedId() - 1);
		sortMappings();
	}
}

void MidiMappingEditorWindow::MidiMappingEditorContent::refreshMappingsList()
{
	filteredMappings = midiLearnManager->getAllMappings();
	updateFilteredMappings();
}

void MidiMappingEditorWindow::MidiMappingEditorContent::updateFilteredMappings()
{
	if (searchFilter.isNotEmpty())
	{
		std::vector<MidiMapping> filtered;
		for (const auto& mapping : filteredMappings)
		{
			if (mapping.parameterName.containsIgnoreCase(searchFilter) ||
				mapping.description.containsIgnoreCase(searchFilter))
			{
				filtered.push_back(mapping);
			}
		}
		filteredMappings = filtered;
	}

	sortMappings();

	createMappingRows();
}

void MidiMappingEditorWindow::MidiMappingEditorContent::sortMappings()
{
	std::sort(filteredMappings.begin(), filteredMappings.end(),
		[this](const MidiMapping& a, const MidiMapping& b)
		{
			switch (currentSortMode)
			{
			case SORT_PARAMETER_NAME:
				return a.parameterName.compare(b.parameterName) < 0;
			case SORT_DESCRIPTION:
				return a.description.compare(b.description) < 0;
			case SORT_MIDI_NUMBER:
				return a.midiNumber < b.midiNumber;
			case SORT_MIDI_CHANNEL:
				return a.midiChannel < b.midiChannel;
			default:
				return false;
			}
		});

	createMappingRows();
}

void MidiMappingEditorWindow::MidiMappingEditorContent::filterMappings()
{
	searchFilter = searchBox.getText();
	refreshMappingsList();
}

void MidiMappingEditorWindow::MidiMappingEditorContent::createMappingRows()
{
	mappingRows.clear();

	int y = 0;
	for (const auto& mapping : filteredMappings)
	{
		auto* row = new MidiMappingRow(mapping, midiLearnManager);

		row->onDeleteClicked = [this, mapping] { deleteMapping(mapping); };
		row->onEditClicked = [this, mapping] { editMapping(mapping); };
		row->onLearnClicked = [this, mapping] { startLearningForMapping(mapping); };

		mappingRows.add(row);
		mappingsContainer.addAndMakeVisible(row);
		row->setBounds(0, y, 800, 50);
		y += 50;
	}

	mappingsContainer.setSize(800, y);
}

void MidiMappingEditorWindow::MidiMappingEditorContent::deleteMapping(const MidiMapping& mapping)
{
	showConfirmationDialog("Delete mapping for \"" + mapping.parameterName + "\"?",
		[this, mapping] {
			midiLearnManager->removeMapping(mapping.parameterName);
			refreshMappingsList();
		});
}

void MidiMappingEditorWindow::MidiMappingEditorContent::startLearningForMapping(const MidiMapping& mapping)
{
	midiLearnManager->startLearning(mapping.parameterName,
		mapping.processor,
		mapping.uiCallback,
		mapping.description);

	for (auto* row : mappingRows)
		row->setLearningActive(false);

	for (auto* row : mappingRows)
	{
		if (row->getMapping().parameterName == mapping.parameterName)
		{
			row->setLearningActive(true);
			break;
		}
	}
}


void MidiMappingEditorWindow::MidiMappingEditorContent::editMapping(const MidiMapping& mapping)
{
	auto editableMapping = mapping;

	auto* dialog = new MidiMappingEditDialog(editableMapping, midiLearnManager, midiLearnManager->getProcessor());
	dialog->onMappingUpdated = [this, oldName = mapping.parameterName](const MidiMapping& updated)
		{
			midiLearnManager->removeMapping(oldName);
			midiLearnManager->addMapping(updated);
			refreshMappingsList();
		};

	dialog->setVisible(true);
}

void MidiMappingEditorWindow::MidiMappingEditorContent::exportMappings()
{
	fileChooser = std::make_unique<juce::FileChooser>(
		"Save MIDI Mapping",
		juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
		"*.json");

	fileChooser->launchAsync(juce::FileBrowserComponent::saveMode |
		juce::FileBrowserComponent::canSelectFiles,
		[this](const juce::FileChooser& chooser)
		{
			auto result = chooser.getResult();
			if (result != juce::File())
			{
				juce::XmlElement rootElement("MidiMappings");
				for (const auto& mapping : filteredMappings)
				{
					auto* mappingElement = rootElement.createNewChildElement("Mapping");
					mappingElement->setAttribute("parameter", mapping.parameterName);
					mappingElement->setAttribute("description", mapping.description);
					mappingElement->setAttribute("midiType", mapping.midiType);
					mappingElement->setAttribute("midiNumber", mapping.midiNumber);
					mappingElement->setAttribute("midiChannel", mapping.midiChannel);
				}
				if (!rootElement.writeTo(result))
				{
					juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
						"Export Failed",
						"Could not save the mappings file.");
				}
			}
		});
}

void MidiMappingEditorWindow::MidiMappingEditorContent::importMappings()
{
	fileChooser = std::make_unique<juce::FileChooser>(
		"Load MIDI Mapping",
		juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
		"*.json");

	fileChooser->launchAsync(juce::FileBrowserComponent::openMode |
		juce::FileBrowserComponent::canSelectFiles,
		[this](const juce::FileChooser& chooser)
		{
			auto result = chooser.getResult();
			if (result.existsAsFile())
			{
				auto xml = juce::XmlDocument::parse(result);
				if (xml != nullptr && xml->hasTagName("MidiMappings"))
				{
					auto sharedXml = std::shared_ptr<juce::XmlElement>(std::move(xml));

					auto options = juce::MessageBoxOptions()
						.withIconType(juce::MessageBoxIconType::QuestionIcon)
						.withTitle("Confirm Import")
						.withMessage("This will replace all current mappings. Continue?")
						.withButton("Yes")
						.withButton("No");

					juce::AlertWindow::showAsync(options, [this, sharedXml](int result)
						{
							if (result == 1)
							{
								midiLearnManager->clearAllMappings();
								for (auto* mappingElement : sharedXml->getChildIterator())
								{
									if (mappingElement->hasTagName("Mapping"))
									{
										MidiMapping mapping;
										mapping.parameterName = mappingElement->getStringAttribute("parameter");
										mapping.description = mappingElement->getStringAttribute("description");
										mapping.midiType = mappingElement->getIntAttribute("midiType");
										mapping.midiNumber = mappingElement->getIntAttribute("midiNumber");
										mapping.midiChannel = mappingElement->getIntAttribute("midiChannel");
										midiLearnManager->addMapping(mapping);
									}
								}
								refreshMappingsList();
							}
						});
				}
				else
				{
					juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
						"Import Failed",
						"Invalid mappings file.");
				}
			}
		});
}

void MidiMappingEditorWindow::MidiMappingEditorContent::showConfirmationDialog(
	const juce::String& message, std::function<void()> onConfirm)
{
	auto options = juce::MessageBoxOptions()
		.withIconType(juce::MessageBoxIconType::QuestionIcon)
		.withTitle("Confirmation")
		.withMessage(message)
		.withButton("Yes")
		.withButton("No");

	juce::AlertWindow::showAsync(options, [onConfirm](int result)
		{
			if (result == 1 && onConfirm)
				onConfirm();
		});
}


MidiMappingEditorWindow::MidiMappingEditorWindow(MidiLearnManager* manager)
	: DocumentWindow("MIDI Mappings Editor",
		juce::Colour(0xff2a2a2a),
		DocumentWindow::allButtons),
	midiLearnManager(manager)
{
	content = std::make_unique<MidiMappingEditorContent>(manager);
	setContentOwned(content.release(), true);

	setResizable(true, true);
	setResizeLimits(600, 400, 1200, 800);
	setBounds(100, 100, 850, 600);

	setVisible(true);

	startTimerHz(2);
}

MidiMappingEditorWindow::~MidiMappingEditorWindow()
{
	stopTimer();
}

void MidiMappingEditorWindow::closeButtonPressed()
{
	if (onWindowClosed)
		onWindowClosed();

	setVisible(false);
	delete this;
}

void MidiMappingEditorWindow::buttonClicked(juce::Button* button)
{
}

void MidiMappingEditorWindow::timerCallback()
{
	if (auto* content = dynamic_cast<MidiMappingEditorContent*>(getContentComponent()))
	{
		if (midiLearnManager->isLearningActive())
		{
			for (auto* row : content->mappingRows)
				row->toggleBlink();
		}
		else
		{
			for (auto* row : content->mappingRows)
				row->setLearningActive(false);
		}
	}
}

MidiMappingEditDialog::EditContent::EditContent(MidiMapping& mapping, MidiLearnManager* manager, DjIaVstProcessor* processor)
	: originalMapping(mapping), midiLearnManager(manager), processorRef(processor)
{
	parameterLabel.setText("Parameter:", juce::dontSendNotification);
	addAndMakeVisible(parameterLabel);

	populateParameterNameComboBox();
	parameterNameComboBox.setEditableText(true);
	parameterNameComboBox.setText(originalMapping.parameterName, juce::dontSendNotification);
	addAndMakeVisible(parameterNameComboBox);

	descriptionLabel.setText("Description:", juce::dontSendNotification);
	addAndMakeVisible(descriptionLabel);

	descriptionEditor.setText(mapping.description);
	addAndMakeVisible(descriptionEditor);

	midiChannelLabel.setText("MIDI Channel:", juce::dontSendNotification);
	addAndMakeVisible(midiChannelLabel);

	midiChannelSlider.setRange(1, 16, 1);
	midiChannelSlider.setValue(mapping.midiChannel + 1);
	midiChannelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	midiChannelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
	addAndMakeVisible(midiChannelSlider);

	midiNumberLabel.setText("MIDI Number:", juce::dontSendNotification);
	addAndMakeVisible(midiNumberLabel);

	midiNumberSlider.setRange(0, 127, 1);
	midiNumberSlider.setValue(mapping.midiNumber);
	midiNumberSlider.setSliderStyle(juce::Slider::LinearHorizontal);
	midiNumberSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
	addAndMakeVisible(midiNumberSlider);

	midiTypeLabel.setText("MIDI Type:", juce::dontSendNotification);
	addAndMakeVisible(midiTypeLabel);

	populateMidiTypeComboBox();
	addAndMakeVisible(midiTypeComboBox);

	applyButton.setButtonText("Apply");
	applyButton.addListener(this);
	addAndMakeVisible(applyButton);

	cancelButton.setButtonText("Cancel");
	cancelButton.addListener(this);
	addAndMakeVisible(cancelButton);

	learnButton.setButtonText("MIDI Learn");
	learnButton.addListener(this);
	learnButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
	addAndMakeVisible(learnButton);

	setSize(400, 350);
}

void MidiMappingEditDialog::EditContent::populateParameterNameComboBox()
{
	parameterNameComboBox.clear();

	auto* processor = processorRef;
	if (processor == nullptr)
	{
		DBG("populateParameterNameComboBox: processor is null!");
		return;
	}

	auto& apvts = processor->getParameters();
	const auto& params = apvts.processor.getParameters();

	DBG("Listing parameters from AudioProcessorValueTreeState...");

	for (auto* param : params)
	{
		if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(param))
		{
			juce::String id = ranged->paramID;
			juce::String name = ranged->getName(64);

			bool alreadyAdded = false;
			for (int i = 0; i < parameterNameComboBox.getNumItems(); ++i)
			{
				if (parameterNameComboBox.getItemText(i).startsWith(id))
				{
					alreadyAdded = true;
					break;
				}
			}

			if (!alreadyAdded)
			{
				parameterNameComboBox.addItem(id + " (" + name + ")", parameterNameComboBox.getNumItems() + 1);
				DBG("Added parameter: " + id);
			}
		}
	}

	DBG("Total parameters found: " + juce::String(parameterNameComboBox.getNumItems()));

	for (int i = 0; i < parameterNameComboBox.getNumItems(); ++i)
	{
		if (parameterNameComboBox.getItemText(i).contains(originalMapping.parameterName))
		{
			parameterNameComboBox.setSelectedId(i + 1, juce::dontSendNotification);
			break;
		}
	}
}



MidiMappingEditDialog::EditContent::~EditContent()
{
}

void MidiMappingEditDialog::EditContent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xff2a2a2a));
}

void MidiMappingEditDialog::EditContent::resized()
{
	auto bounds = getLocalBounds().reduced(20);

	auto row = bounds.removeFromTop(30);
	parameterLabel.setBounds(row.removeFromLeft(100));
	parameterNameComboBox.setBounds(row);

	bounds.removeFromTop(10);

	row = bounds.removeFromTop(30);
	descriptionLabel.setBounds(row.removeFromLeft(100));
	descriptionEditor.setBounds(row);

	bounds.removeFromTop(10);

	row = bounds.removeFromTop(40);
	midiTypeLabel.setBounds(row.removeFromLeft(100));
	midiTypeComboBox.setBounds(row.removeFromLeft(150));

	bounds.removeFromTop(10);

	row = bounds.removeFromTop(40);
	midiChannelLabel.setBounds(row.removeFromLeft(100));
	midiChannelSlider.setBounds(row);

	bounds.removeFromTop(10);

	row = bounds.removeFromTop(40);
	midiNumberLabel.setBounds(row.removeFromLeft(100));
	midiNumberSlider.setBounds(row);

	bounds.removeFromTop(20);

	auto buttonArea = bounds.removeFromBottom(40);
	cancelButton.setBounds(buttonArea.removeFromRight(80).reduced(5));
	applyButton.setBounds(buttonArea.removeFromRight(80).reduced(5));
	learnButton.setBounds(buttonArea.removeFromLeft(100).reduced(5));
}


void MidiMappingEditDialog::EditContent::buttonClicked(juce::Button* button)
{
	if (button == &applyButton)
	{
		if (auto* dialog = findParentComponentOfClass<MidiMappingEditDialog>())
		{
			if (dialog->onMappingUpdated)
				dialog->onMappingUpdated(getUpdatedMapping());
			dialog->closeButtonPressed();
		}
	}
	else if (button == &cancelButton)
	{
		if (auto* dialog = findParentComponentOfClass<MidiMappingEditDialog>())
			dialog->closeButtonPressed();
	}
	else if (button == &learnButton)
	{
		midiLearnManager->startLearning(
			originalMapping.parameterName,
			originalMapping.processor,
			nullptr,
			descriptionEditor.getText());

		isLearning = true;
		blinkState = false;
		blinkTimer = std::make_unique<BlinkTimer>(this);
		blinkTimer->startTimerHz(2);
	}
}


MidiMapping MidiMappingEditDialog::EditContent::getUpdatedMapping() const
{
	MidiMapping updated = originalMapping;
	updated.parameterName = parameterNameComboBox.getText();
	updated.description = descriptionEditor.getText();
	updated.midiChannel = static_cast<int>(midiChannelSlider.getValue()) - 1;
	updated.midiNumber = static_cast<int>(midiNumberSlider.getValue());

	switch (midiTypeComboBox.getSelectedId())
	{
	case 1: updated.midiType = 1; break;
	case 2: updated.midiType = 0; break;
	case 3: updated.midiType = 2; break;
	}
	return updated;

}

void MidiMappingEditDialog::EditContent::populateMidiTypeComboBox()
{
	midiTypeComboBox.clear();
	midiTypeComboBox.addItem("Control Change (CC)", 1);
	midiTypeComboBox.addItem("Note", 2);
	midiTypeComboBox.addItem("Pitch Bend", 3);

	switch (originalMapping.midiType)
	{
	case 0xB0: midiTypeComboBox.setSelectedId(1); break;
	case 0x90: midiTypeComboBox.setSelectedId(2); break;
	case 0xE0: midiTypeComboBox.setSelectedId(3); break;
	default: midiTypeComboBox.setSelectedId(1); break;
	}
}

MidiMappingEditDialog::MidiMappingEditDialog(MidiMapping& mapping, MidiLearnManager* manager, DjIaVstProcessor* processor)
	: DialogWindow("Edit MIDI Mapping", juce::Colour(0xff2a2a2a), true),
	mapping(mapping), midiLearnManager(manager), processorRef(processor)
{
	content = std::make_unique<EditContent>(mapping, manager, processorRef);
	setContentOwned(content.release(), true);
	setResizable(false, false);
	centreAroundComponent(nullptr, getWidth(), getHeight());
	setVisible(true);
}


MidiMappingEditDialog::~MidiMappingEditDialog()
{
}

void MidiMappingEditDialog::closeButtonPressed()
{
	setVisible(false);
	delete this;
}