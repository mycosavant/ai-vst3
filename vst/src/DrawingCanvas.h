#pragma once
#include "JuceHeader.h"

class DrawingCanvas : public juce::Component, private juce::Timer
{
public:
	enum class BrushType
	{
		Pencil,
		Brush,
		Airbrush,
		Eraser
	};

	DrawingCanvas()
	{
		canvas = juce::Image(juce::Image::RGB, 512, 512, true);
		clearCanvas();

		setupUI();
		setSize(600, 750);
		startTimerHz(60);
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(juce::Colour(0xff1a1a1a));

		g.setColour(juce::Colours::white);
		g.fillRect(canvasAreaBounds);

		g.drawImageAt(canvas, canvasAreaBounds.getX(), canvasAreaBounds.getY());

		g.setColour(juce::Colour(0xff3a3a3a));
		g.drawRect(canvasAreaBounds, 2);
	}

	void resized() override
	{
		auto bounds = getLocalBounds().reduced(10);

		bounds.removeFromTop(10);

		auto canvasContainer = bounds.removeFromTop(512);
		int canvasX = (canvasContainer.getWidth() - 512) / 2;
		canvasAreaBounds = juce::Rectangle<int>(canvasX + 10, canvasContainer.getY(), 512, 512);

		bounds.removeFromTop(15);

		auto toolsArea = bounds;

		auto brushRow = toolsArea.removeFromTop(45);
		int btnW = (brushRow.getWidth() - 15) / 4;

		pencilButton.setBounds(brushRow.removeFromLeft(btnW));
		brushRow.removeFromLeft(5);
		brushButton.setBounds(brushRow.removeFromLeft(btnW));
		brushRow.removeFromLeft(5);
		airbrushButton.setBounds(brushRow.removeFromLeft(btnW));
		brushRow.removeFromLeft(5);
		eraserButton.setBounds(brushRow.removeFromLeft(btnW));

		toolsArea.removeFromTop(12);

		auto sizeRow = toolsArea.removeFromTop(45);
		brushSizeLabel.setBounds(sizeRow.removeFromLeft(60));
		sizeRow.removeFromLeft(5);
		brushSizeSlider.setBounds(sizeRow);

		toolsArea.removeFromTop(12);

		auto colorRow = toolsArea.removeFromTop(35);
		colorLabel.setBounds(colorRow.removeFromLeft(60));
		colorRow.removeFromLeft(5);

		int swatchSize = 28;
		int spacing = 5;
		for (auto* swatch : colorSwatches)
		{
			if (colorRow.getWidth() >= swatchSize)
			{
				swatch->setBounds(colorRow.removeFromLeft(swatchSize));
				colorRow.removeFromLeft(spacing);
			}
		}

		toolsArea.removeFromTop(15);

		auto actionRow = toolsArea.removeFromTop(45);
		int halfWidth = (actionRow.getWidth() - 10) / 2;
		clearButton.setBounds(actionRow.removeFromLeft(halfWidth));
		actionRow.removeFromLeft(10);
		generateButton.setBounds(actionRow);
	}

	void mouseDown(const juce::MouseEvent& e) override
	{
		if (isPointInCanvas(e.getPosition()))
		{
			isDrawing = true;
			lastPoint = getCanvasPoint(e.getPosition());

			juce::Graphics g(canvas);
			drawAtPoint(g, lastPoint);
			needsRepaint = true;
		}
	}

	void mouseDrag(const juce::MouseEvent& e) override
	{
		if (isDrawing && isPointInCanvas(e.getPosition()))
		{
			auto currentPoint = getCanvasPoint(e.getPosition());
			if (lastPoint != currentPoint)
			{
				juce::Graphics g(canvas);
				drawLine(g, lastPoint, currentPoint);
				lastPoint = currentPoint;
				needsRepaint = true;
			}
		}
	}

	void mouseUp(const juce::MouseEvent&) override
	{
		isDrawing = false;
	}

	void drawAtPoint(juce::Graphics& g, juce::Point<int> point)
	{
		switch (currentBrushType)
		{
		case BrushType::Pencil:
			g.setColour(currentColor);
			g.fillRect(point.x, point.y, 2, 2);
			break;

		case BrushType::Brush:
			g.setColour(currentColor);
			g.fillEllipse(
				point.x - currentBrushSize / 2.0f,
				point.y - currentBrushSize / 2.0f,
				currentBrushSize,
				currentBrushSize
			);
			break;

		case BrushType::Airbrush:
		{
			int sprayRadius = std::max(5, (int)(currentBrushSize * 1.5f));
			int numParticles = std::max(10, (int)(currentBrushSize * 3));

			for (int i = 0; i < numParticles; ++i)
			{
				float angle = random.nextFloat() * juce::MathConstants<float>::twoPi;
				float dist = random.nextFloat() * sprayRadius;
				int px = point.x + (int)(std::cos(angle) * dist);
				int py = point.y + (int)(std::sin(angle) * dist);

				if (canvas.getBounds().contains(px, py))
				{
					g.setColour(currentColor.withAlpha(0.15f));
					g.fillEllipse(px - 1.0f, py - 1.0f, 2.0f, 2.0f);
				}
			}
		}
		break;

		case BrushType::Eraser:
			g.setColour(juce::Colours::white);
			g.fillEllipse(
				point.x - currentBrushSize / 2.0f,
				point.y - currentBrushSize / 2.0f,
				currentBrushSize,
				currentBrushSize
			);
			break;
		}
	}

	void drawLine(juce::Graphics& g, juce::Point<int> from, juce::Point<int> to)
	{
		float dx = (float)(to.x - from.x);
		float dy = (float)(to.y - from.y);
		float distance = std::sqrt(dx * dx + dy * dy);

		if (distance < 1.0f)
		{
			drawAtPoint(g, to);
			return;
		}

		float stepSize = 1.0f;

		switch (currentBrushType)
		{
		case BrushType::Pencil:
			stepSize = 1.0f;
			break;
		case BrushType::Brush:
			stepSize = currentBrushSize * 0.25f;
			break;
		case BrushType::Airbrush:
			stepSize = currentBrushSize * 0.5f;
			break;
		case BrushType::Eraser:
			stepSize = currentBrushSize * 0.25f;
			break;
		}

		stepSize = std::max(1.0f, stepSize);
		int steps = std::max(1, (int)(distance / stepSize));

		for (int i = 0; i <= steps; ++i)
		{
			float t = (float)i / steps;
			int x = (int)(from.x + t * dx);
			int y = (int)(from.y + t * dy);
			drawAtPoint(g, { x, y });
		}
	}

	void clearCanvas()
	{
		juce::Graphics g(canvas);
		g.fillAll(juce::Colours::white);
		repaint();
	}

	juce::String getBase64Image()
	{
		juce::MemoryOutputStream memStream;
		juce::PNGImageFormat pngFormat;

		if (pngFormat.writeImageToStream(canvas, memStream))
		{
			juce::String base64 = memStream.getMemoryBlock().toBase64Encoding();

			base64 = base64.removeCharacters("\r\n\t ");

			return base64;
		}

		return {};
	}

	void loadFromBase64(const juce::String& base64Data)
	{
		if (base64Data.isEmpty())
			return;

		juce::MemoryBlock block;
		if (block.fromBase64Encoding(base64Data))
		{
			juce::MemoryInputStream memStream(block, false);
			juce::PNGImageFormat pngFormat;

			auto loadedImage = pngFormat.decodeImage(memStream);
			if (loadedImage.isValid())
			{
				canvas = loadedImage;
				repaint();
			}
		}
	}

	std::function<void(const juce::String&)> onGenerate;
	std::function<void()> onClose;

private:
	void timerCallback() override
	{
		if (needsRepaint)
		{
			repaint(canvasAreaBounds);
			needsRepaint = false;
		}
	}

	bool isPointInCanvas(juce::Point<int> p)
	{
		return canvasAreaBounds.contains(p);
	}

	juce::Point<int> getCanvasPoint(juce::Point<int> screenPoint)
	{
		return screenPoint - canvasAreaBounds.getPosition();
	}

	void setupUI()
	{
		addAndMakeVisible(pencilButton);
		pencilButton.setButtonText("Pencil");
		pencilButton.setRadioGroupId(1);
		pencilButton.setClickingTogglesState(true);
		pencilButton.setToggleState(true, juce::dontSendNotification);
		pencilButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a));
		pencilButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff6a6a6a));
		pencilButton.onClick = [this] { currentBrushType = BrushType::Pencil; };

		addAndMakeVisible(brushButton);
		brushButton.setButtonText("Brush");
		brushButton.setRadioGroupId(1);
		brushButton.setClickingTogglesState(true);
		brushButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a));
		brushButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff6a6a6a));
		brushButton.onClick = [this] { currentBrushType = BrushType::Brush; };

		addAndMakeVisible(airbrushButton);
		airbrushButton.setButtonText("Spray");
		airbrushButton.setRadioGroupId(1);
		airbrushButton.setClickingTogglesState(true);
		airbrushButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a));
		airbrushButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff6a6a6a));
		airbrushButton.onClick = [this] { currentBrushType = BrushType::Airbrush; };

		addAndMakeVisible(eraserButton);
		eraserButton.setButtonText("Eraser");
		eraserButton.setRadioGroupId(1);
		eraserButton.setClickingTogglesState(true);
		eraserButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a3a3a));
		eraserButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff6a6a6a));
		eraserButton.onClick = [this] { currentBrushType = BrushType::Eraser; };

		addAndMakeVisible(brushSizeLabel);
		brushSizeLabel.setText("Size:", juce::dontSendNotification);
		brushSizeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
		brushSizeLabel.setJustificationType(juce::Justification::centredRight);

		addAndMakeVisible(brushSizeSlider);
		brushSizeSlider.setRange(1, 50, 1);
		brushSizeSlider.setValue(5, juce::dontSendNotification);
		brushSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
		brushSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
		brushSizeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff6a6a6a));
		brushSizeSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a4a));
		brushSizeSlider.onValueChange = [this]
			{
				currentBrushSize = (float)brushSizeSlider.getValue();
				DBG("Brush size changed: " << currentBrushSize);
			};

		addAndMakeVisible(colorLabel);
		colorLabel.setText("Color:", juce::dontSendNotification);
		colorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
		colorLabel.setJustificationType(juce::Justification::centredRight);

		juce::Array<juce::Colour> colors = {
			juce::Colours::black, juce::Colours::red, juce::Colours::blue,
			juce::Colours::green, juce::Colours::yellow, juce::Colours::orange,
			juce::Colours::purple, juce::Colours::brown, juce::Colours::grey,
			juce::Colours::white
		};

		for (auto c : colors)
		{
			auto* b = new juce::TextButton();
			addAndMakeVisible(b);
			b->setColour(juce::TextButton::buttonColourId, c);
			b->onClick = [this, c] { currentColor = c; };
			colorSwatches.add(b);
		}

		addAndMakeVisible(clearButton);
		clearButton.setButtonText("Clear");
		clearButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8a3a3a));
		clearButton.onClick = [this] { clearCanvas(); };

		addAndMakeVisible(generateButton);
		generateButton.setButtonText("Generate");
		generateButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3a8a3a));
		generateButton.onClick = [this]
			{
				if (onGenerate)
					onGenerate(getBase64Image());
				if (onClose)
					onClose();
			};
	}

	juce::Image canvas;
	juce::Rectangle<int> canvasAreaBounds;
	bool isDrawing = false;
	bool needsRepaint = false;
	juce::Point<int> lastPoint;
	juce::Random random;

	BrushType currentBrushType = BrushType::Pencil;
	float currentBrushSize = 5.0f;
	juce::Colour currentColor = juce::Colours::black;

	juce::TextButton pencilButton;
	juce::TextButton brushButton;
	juce::TextButton airbrushButton;
	juce::TextButton eraserButton;
	juce::Label brushSizeLabel;
	juce::Slider brushSizeSlider;
	juce::Label colorLabel;
	juce::OwnedArray<juce::TextButton> colorSwatches;
	juce::TextButton clearButton;
	juce::TextButton generateButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrawingCanvas)
};