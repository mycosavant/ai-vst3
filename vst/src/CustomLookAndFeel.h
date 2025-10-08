#pragma once
#include <JuceHeader.h>
#include "ColourPalette.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
	CustomLookAndFeel()
	{
		setColour(juce::TextButton::buttonColourId, soften(ColourPalette::backgroundLight));
		setColour(juce::TextButton::buttonOnColourId, soften(ColourPalette::buttonSuccess));
		setColour(juce::TextButton::textColourOffId, ColourPalette::textPrimary);
		setColour(juce::TextButton::textColourOnId, ColourPalette::textPrimary);
		setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
		setColour(juce::ToggleButton::tickColourId, soften(ColourPalette::buttonSuccess));
		setColour(juce::TextEditor::backgroundColourId, ColourPalette::backgroundDeep);
		setColour(juce::TextEditor::textColourId, ColourPalette::textPrimary);
		setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
		setColour(juce::TextEditor::focusedOutlineColourId, ColourPalette::backgroundLight.brighter(0.2f));
		setColour(juce::TextEditor::highlightColourId, soften(ColourPalette::backgroundLight).brighter(0.3f));
		setColour(juce::TextEditor::shadowColourId, juce::Colours::transparentBlack);
		setColour(juce::ScrollBar::thumbColourId, ColourPalette::sliderThumb);
		setColour(juce::ScrollBar::backgroundColourId, ColourPalette::backgroundDeep);
	}

private:
	static juce::Colour soften(const juce::Colour& colour)
	{
		return colour.withSaturation(colour.getSaturation() * 0.8f)
			.brighter(0.15f);
	}

public:

	void drawButtonBackground(juce::Graphics& g,
		juce::Button& button,
		const juce::Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override
	{
		auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
		auto baseColour = soften(backgroundColour);

		if (shouldDrawButtonAsDown)
			baseColour = baseColour.darker(0.15f);
		else if (shouldDrawButtonAsHighlighted)
			baseColour = baseColour.brighter(0.08f);

		g.setColour(baseColour);
		g.fillRoundedRectangle(bounds, 4.0f);
	}

	void drawButtonText(juce::Graphics& g,
		juce::TextButton& button,
		bool /*shouldDrawButtonAsHighlighted*/,
		bool /*shouldDrawButtonAsDown*/) override
	{
		auto textColour = button.findColour(button.getToggleState()
			? juce::TextButton::textColourOnId
			: juce::TextButton::textColourOffId);

		if (!button.isEnabled())
			textColour = textColour.withAlpha(0.5f);

		g.setColour(textColour);
		g.setFont(juce::FontOptions(14.0f));

		g.drawText(button.getButtonText(),
			button.getLocalBounds(),
			juce::Justification::centred);
	}

	void drawToggleButton(juce::Graphics& g,
		juce::ToggleButton& button,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override
	{
		auto bounds = button.getLocalBounds().toFloat();

		auto bgColour = button.getToggleState()
			? soften(button.findColour(juce::ToggleButton::tickColourId))
			: ColourPalette::backgroundDark;

		if (shouldDrawButtonAsDown)
			bgColour = bgColour.darker(0.15f);
		else if (shouldDrawButtonAsHighlighted)
			bgColour = bgColour.brighter(0.08f);

		g.setColour(bgColour);
		g.fillRoundedRectangle(bounds, 4.0f);

		g.setColour(button.getToggleState()
			? ColourPalette::textPrimary
			: ColourPalette::textSecondary);
		g.setFont(juce::FontOptions(14.0f));
		g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
	}

	void drawComboBox(juce::Graphics& g,
		int width, int height,
		bool /*isButtonDown*/,
		int buttonX, int buttonY,
		int buttonW, int buttonH,
		juce::ComboBox& /*box*/) override
	{
		auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

		g.setColour(ColourPalette::backgroundDark);
		g.fillRoundedRectangle(bounds, 4.0f);

		auto arrowZone = juce::Rectangle<float>((float)buttonX, (float)buttonY, (float)buttonW, (float)buttonH);
		auto arrowBounds = arrowZone.reduced(4.0f);

		juce::Path arrow;
		arrow.addTriangle(
			arrowBounds.getCentreX() - 3.0f, arrowBounds.getCentreY() - 2.0f,
			arrowBounds.getCentreX() + 3.0f, arrowBounds.getCentreY() - 2.0f,
			arrowBounds.getCentreX(), arrowBounds.getCentreY() + 2.0f
		);

		g.setColour(soften(ColourPalette::textSecondary));
		g.fillPath(arrow);
	}

	void drawLinearSlider(juce::Graphics& g,
		int x, int y, int width, int height,
		float sliderPos,
		float minSliderPos, float maxSliderPos,
		const juce::Slider::SliderStyle style,
		juce::Slider& /* slider */) override
	{
		if (style == juce::Slider::LinearVertical || style == juce::Slider::LinearHorizontal)
		{
			auto trackWidth = juce::jmin(6.0f, (float)(style == juce::Slider::LinearVertical ? width : height) * 0.25f);

			juce::Point<float> startPoint, endPoint;

			if (style == juce::Slider::LinearVertical)
			{
				auto cx = (float)x + (float)width * 0.5f;
				startPoint = { cx, (float)y };
				endPoint = { cx, (float)(y + height) };
			}
			else
			{
				auto cy = (float)y + (float)height * 0.5f;
				startPoint = { (float)x, cy };
				endPoint = { (float)(x + width), cy };
			}

			g.setColour(ColourPalette::backgroundDeep);
			g.fillRoundedRectangle(juce::Rectangle<float>(startPoint, endPoint).expanded(trackWidth * 0.5f), trackWidth * 0.5f);
			auto filledEnd = startPoint + (endPoint - startPoint) * ((sliderPos - minSliderPos) / (maxSliderPos - minSliderPos));
			g.setColour(soften(ColourPalette::sliderThumb));
			g.fillRoundedRectangle(juce::Rectangle<float>(startPoint, filledEnd).expanded(trackWidth * 0.5f), trackWidth * 0.5f);
		}

		auto thumbWidth = 16.0f;
		g.setColour(soften(ColourPalette::sliderThumb));

		if (style == juce::Slider::LinearVertical)
			g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre({ (float)x + (float)width * 0.5f, sliderPos }));
		else
			g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre({ sliderPos, (float)y + (float)height * 0.5f }));
	}

	void drawRotarySlider(juce::Graphics& g,
		int x, int y, int width, int height,
		float sliderPosProportional,
		float rotaryStartAngle, float rotaryEndAngle,
		juce::Slider& /* slider */) override
	{
		auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(8.0f);
		auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
		auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = radius * 0.2f;
		auto arcRadius = radius - lineW * 0.5f;

		juce::Path backgroundArc;
		backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
			arcRadius, arcRadius,
			0.0f, rotaryStartAngle, rotaryEndAngle, true);

		g.setColour(ColourPalette::backgroundDeep);
		g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		juce::Path valueArc;
		valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
			arcRadius, arcRadius,
			0.0f, rotaryStartAngle, toAngle, true);

		g.setColour(soften(ColourPalette::sliderThumb));
		g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		juce::Path pointer;
		auto pointerLength = radius * 0.6f;
		auto pointerThickness = lineW * 1.5f;
		pointer.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
		pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));

		g.setColour(soften(ColourPalette::textPrimary));
		g.fillPath(pointer);
	}

	void drawTextEditorOutline(juce::Graphics& g,
		int width, int height,
		juce::TextEditor& textEditor) override
	{
		if (textEditor.isEnabled())
		{
			if (textEditor.hasKeyboardFocus(true))
			{
				g.setColour(ColourPalette::backgroundLight.brighter(0.2f));
				g.drawRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 4.0f, 2.0f);
			}
			else
			{
				g.setColour(ColourPalette::backgroundLight);
				g.drawRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 4.0f, 1.0f);
			}
		}
	}

	void drawScrollbar(juce::Graphics& g,
		juce::ScrollBar& /*scrollbar*/,
		int x, int y,
		int width, int height,
		bool isScrollbarVertical,
		int thumbStartPosition,
		int thumbSize,
		bool isMouseOver,
		bool isMouseDown) override
	{
		g.setColour(findColour(juce::ScrollBar::backgroundColourId));
		g.fillRoundedRectangle((float)x, (float)y, (float)width, (float)height, 4.0f);

		juce::Rectangle<float> thumbBounds;

		if (isScrollbarVertical)
			thumbBounds = juce::Rectangle<float>((float)x,
				(float)thumbStartPosition,
				(float)width,
				(float)thumbSize);
		else
			thumbBounds = juce::Rectangle<float>((float)thumbStartPosition,
				(float)y,
				(float)thumbSize,
				(float)height);

		auto thumbColour = findColour(juce::ScrollBar::thumbColourId);

		if (isMouseDown)
			thumbColour = thumbColour.brighter(0.2f);
		else if (isMouseOver)
			thumbColour = thumbColour.brighter(0.1f);

		g.setColour(thumbColour);
		g.fillRoundedRectangle(thumbBounds, 4.0f);
	}



};