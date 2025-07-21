/*
 * AppLookAndFeel.cpp
 *
 *  Created on: Dec. 21, 2020
 *      Author: blackaddr
 */
#include <algorithm> // for std::min
#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/GuiUtil.h"
#include "Util/ErrorMessage.h"
#include "Util/AppLookAndFeel.h"

using namespace juce;

namespace stride {

BlackaddrAudioLookAndFeel::BlackaddrAudioLookAndFeel() {
    setColour(AlertWindow::backgroundColourId, AAPurple);
    setColour(AlertWindow::outlineColourId, AABlue);
}

void paintBasicWindow(Graphics& g, Component& component, bool isTransparentBackground)
{
    const float margin     = BORDER_MARGIN;
    const float thickness  = BORDER_THICKNESS;
    const float cornerSize = BORDER_CORNER_SIZE;
    if (isTransparentBackground) {
        g.setColour(Colour());
    } else {
        g.setColour(Colours::black);
    }
    g.fillAll();
    g.setColour(AABlue);
    g.drawRoundedRectangle(margin, margin, component.getWidth() - 2 * margin, component.getHeight() - 2 * margin, cornerSize, thickness);
}

void paintBasicWindow(Graphics& g, Component& component, Colour backgroundColour, Colour borderColour)
{
    const float thickness  = BORDER_THICKNESS;

    g.setColour(backgroundColour);
    g.fillAll();
    g.setColour(borderColour);
    g.drawRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness, thickness);
}

void paintBasicWindow(Graphics& g, Component& component, Colour backgroundColour, Colour borderColour, float borderThicknessRatio)
{
    float thickness = borderThicknessRatio * std::min(component.getWidth(), component.getHeight());
    g.setColour(backgroundColour);
    g.fillAll();
    g.setColour(borderColour);
    g.drawRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness, thickness);
}

void paintBasicWindow(Graphics& g, Component& component, Colour backgroundColour, Colour borderColour, float cornerRatio, float borderThicknessRatio)
{
    float thickness = cornerRatio * std::min(component.getWidth(), component.getHeight());
    float borderThickness = borderThicknessRatio * std::min(component.getWidth(), component.getHeight());
    g.setColour(backgroundColour);
    g.fillAll();

    g.setColour(borderColour);
    g.drawRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness, borderThickness);
}

void paintBasicWindowRounded(Graphics& g, Component& component, Colour backgroundColour, Colour borderColour)
{
    const float thickness  = BORDER_THICKNESS;

    g.setColour(backgroundColour);
    g.fillRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness);
    g.setColour(borderColour);
    g.drawRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness, thickness);
}

void paintBasicWindowRounded(Graphics& g, Component& component, Colour backgroundColour, Colour borderColour, float cornerRatio, float borderThicknessRatio)
{
    float thickness = cornerRatio * std::min(component.getWidth(), component.getHeight());
    float borderThickness = borderThicknessRatio * std::min(component.getWidth(), component.getHeight());
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness);

    g.setColour(borderColour);
    g.drawRoundedRectangle(thickness, thickness, component.getWidth() - 2 * thickness, component.getHeight() - 2 * thickness, thickness, borderThickness);
}

void paintAreaWindow(Graphics& g, Component& component, bool isTransparentBackground)
{
    const float margin     = BORDER_MARGIN;
    const float thickness  = BORDER_THICKNESS;
    const float cornerSize = BORDER_CORNER_SIZE;
    if (isTransparentBackground) {
        g.setColour(Colour());
    } else {
        g.setColour(AreaBackground);
    }
    //g.fillAll();
    g.fillRoundedRectangle(margin, margin, component.getWidth() - 2 * margin, component.getHeight() - 2 * margin, thickness);
    g.setColour(AABlue);
    g.drawRoundedRectangle(margin, margin, component.getWidth() - 2 * margin, component.getHeight() - 2 * margin, cornerSize, thickness);
}

void paintPopoutWindow(Graphics& g, Component& component, bool isTransparentBackground)
{
    const float margin     = BORDER_MARGIN;
    const float thickness  = BORDER_THICKNESS;
    const float cornerSize = BORDER_CORNER_SIZE;

    if (isTransparentBackground) {
        g.setColour(Colour());
    } else {
        g.setColour(PopoutWindowBackground);
    }

    g.fillRoundedRectangle(margin, margin, component.getWidth() - 2 * margin, component.getHeight() - 2 * margin, cornerSize);
    g.setColour(AABlue);
    g.drawRoundedRectangle(margin, margin, component.getWidth() - 2 * margin, component.getHeight() - 2 * margin, cornerSize, thickness);
}

void effectControlLookAndFeelKnob::drawRotarySlider(Graphics& g,
        int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
{
	UNUSED2(rotaryStartAngle, rotaryEndAngle);
	UNUSED(sliderPos);
    if (!knobImage.isValid() || (knobImage.getHeight() == 0) || (knobImage.getWidth() == 0)) {

        static const float textPpercent = 0.35f;
        Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f,
            0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(Colours::white);

        g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(),
            Justification::horizontallyCentred | Justification::centred, 1);
    }

    // TODO clean up these else branches, they are the same code
    else if (knobRangeType == KnobRangeType::FULL_RANGE) {
        if (knobImage.isValid())
        {
            const double rotation = (slider.getValue()
                - slider.getMinimum())
                / (slider.getMaximum()
                    - slider.getMinimum());

            if (knobFrameHeight <= 0) { // if height is not provided, assume = to width
                knobFrameHeight = knobImage.getWidth();
            }
            const int frames = knobImage.getHeight() / knobFrameHeight;
            const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
            const float halfWidth = width / 2.0f;
            const float halfHeight = height / 2.0f;
            const float centerX = x + width * 0.5f;
            const float centerY = y + height * 0.5f;
            const float rx = centerX - halfWidth - 1.0f;
            const float ry = centerY - halfHeight;

            g.drawImage(knobImage,
                (int)rx,
                (int)ry,
                2 * (int)halfWidth,
                2 * (int)halfHeight,
                0,
                frameId*knobFrameHeight,
                knobImage.getWidth(),
                knobFrameHeight);
        }
    } else if (knobRangeType == KnobRangeType::STOPS) {
        if (knobImage.isValid())
        {
            const double rotation = (slider.getValue()
                - slider.getMinimum())
                / (slider.getMaximum()
                    - slider.getMinimum());

            if (knobFrameHeight <= 0) { // if height is not provided, assume = to width
                knobFrameHeight = knobImage.getWidth();
            }
            const int frames = knobImage.getHeight() / knobFrameHeight;
            const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
            const float halfWidth = width / 2.0f;
            const float halfHeight = height / 2.0f;
            const float centerX = x + width * 0.5f;
            const float centerY = y + height * 0.5f;
            const float rx = centerX - halfWidth - 1.0f;
            const float ry = centerY - halfHeight;

            g.drawImage(knobImage,
                (int)rx,
                (int)ry,
                2 * (int)halfWidth,
                2 * (int)halfHeight,
                0,
                frameId*knobFrameHeight,
                knobImage.getWidth(),
                knobFrameHeight);
        }
    }
}

Image effectControlLookAndFeelKnob::getFrame(Slider& slider)
{
    Image imageFrame; // initial null image

    if (!knobImage.isValid()) { return imageFrame; } // return a null image

    const double rotation = (slider.getValue()
        - slider.getMinimum())
        / (slider.getMaximum()
            - slider.getMinimum());

    if (knobFrameHeight <= 0) { // if height is not provided, assume = to width
        knobFrameHeight = knobImage.getWidth();
    }
    const int frames = knobImage.getHeight() / knobFrameHeight;
    const int frameId = (int)ceil(rotation * ((double)frames - 1.0));

    Rectangle<int> frameArea(0, frameId*knobFrameHeight, knobImage.getWidth(), knobFrameHeight);
    imageFrame = knobImage.getClippedImage(frameArea);

    if (!imageFrame.isValid()) {
        errorMessage("effectControlLookAndFeelKnob::getFrame(): ERROR: imageFrame.isValid() is false");
    }

    return imageFrame;
}

void LedMeterLookAndFeelKnob::drawRotarySlider(Graphics& g,
        int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
{
    UNUSED2(rotaryStartAngle, rotaryEndAngle);
    UNUSED(sliderPos);
    if (!image.isValid()) {

        static const float textPpercent = 0.35f;
        Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f,
            0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(Colours::white);

        g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(),
            Justification::horizontallyCentred | Justification::centred, 1);
        return;
    }

    if (image.isValid())
    {
        const double rotation = (slider.getValue()
            - slider.getMinimum())
            / (slider.getMaximum()
                - slider.getMinimum());

        int frames;

        if (imageHeight > 0) {
            frames = image.getHeight() / imageHeight;
        } else {
            frames = image.getHeight() / image.getWidth();
        }

        const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
        const float halfWidth = width / 2.0f;
        const float halfHeight = height / 2.0f;
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;
        const float rx = centerX - halfWidth - 1.0f;
        const float ry = centerY - halfHeight;

        //printf("FRAME ID: %d\n", frameId);
        //printf("min:%0.1f max:%01.f rotation:%0.1f\n", slider.getMinimum(), slider.getMaximum(), slider.getValue());

        g.drawImage(image,
            (int)rx,
            (int)ry,
            2 * (int)halfWidth,
            2 * (int)halfHeight,
            0,
            frameId*imageHeight,
            image.getWidth(),
            imageHeight
            );
    }
}


void effectControlLookAndFeelExp::drawRotarySlider(Graphics& g,
        int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider)
{
    UNUSED2(rotaryStartAngle, rotaryEndAngle);
    UNUSED(sliderPos);

    if (!knobImage.isValid()) {

        static const float textPpercent = 0.35f;
        Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f,
            0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(Colours::white);

        g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(),
            Justification::horizontallyCentred | Justification::centred, 1);
    }
    if (knobImage.isValid())
    {
        const float halfWidth = width / 2.0f;
        const float halfHeight = height / 2.0f;
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;
        const float rx = centerX - halfWidth - 1.0f;
        const float ry = centerY - halfHeight;

        if (knobFrameHeight <= 0) { // if height is not provided, assume = to width
            knobFrameHeight = knobImage.getWidth();
        }

        g.drawImage(knobImage,
            (int)rx,
            (int)ry,
            2 * (int)halfWidth,
            2 * (int)halfHeight,
            0,
            0,
            knobImage.getWidth(),
            knobFrameHeight);
    }
}

}
