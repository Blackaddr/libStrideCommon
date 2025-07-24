/*
 * AppLookAndFeel.h
 *
 *  Created on: Dec. 13, 2020
 *      Author: blackaddr
 */

#ifndef SOURCE_APPLOOKANDFEEL_H_
#define SOURCE_APPLOOKANDFEEL_H_

#include <string>
#include <JuceHeader.h>

namespace stride {

#ifndef GLOBAL_STUFF
#define GLOBAL_STUFF
const juce::Colour BABlue      (22, 58, 62); // #37939e
const juce::Colour BAWhite (87,87,87); // #dddddd
const juce::Colour BABlack (0,0,0); // #000000
const juce::Colour JuceGrey    (0x32, 0x3e, 0x44);

const juce::Colour BALightGrey (0xA8, 0xA9, 0xAD); // #A8A9AD
const juce::Colour BADarkGrey      (0x30, 0x30, 0x30);
const juce::Colour BAMediumGrey(0x75, 0x75, 0x75); // #757575
const juce::Colour BADarkGreyAlpha ((uint8_t)0x30, (uint8_t)0x30, (uint8_t)0x30, 0.9f);

const juce::Colour AppBackground(JuceGrey);
const juce::Colour AreaBackground(BABlack);
const juce::Colour PopoutWindowBackground(BABlack);
#endif

constexpr float BORDER_THICKNESS   = 2.0f;
constexpr float BORDER_MARGIN      = 5.0f;
constexpr float BORDER_CORNER_SIZE = 5.0f;

constexpr float TEXT_EDIT_WINDOW_WIDTH_RATIO  = 0.35f;
constexpr float TEXT_EDIT_WINDOW_HEIGHT_RATIO = 0.2f;

constexpr float POPUP_WINDOW_DEFAULT_WIDTH  = 400;
constexpr float POPUP_WINDOW_DEFAULT_HEIGHT = 125;

void paintBasicWindow (juce::Graphics& g, juce::Component& component, bool isTransparentBackground = false);
void paintAreaWindow  (juce::Graphics& g, juce::Component& component, bool isTransparentBackground = false);
void paintPopoutWindow(juce::Graphics& g, juce::Component& component, bool isTransparentBackground = false);

void paintBasicWindow(juce::Graphics& g, juce::Component& component, juce::Colour backgroundColour, juce::Colour borderColour);
void paintBasicWindow(juce::Graphics& g, juce::Component& component, juce::Colour backgroundColour, juce::Colour borderColour, float borderThicknessRatio);
void paintBasicWindow(juce::Graphics& g, juce::Component& component, juce::Colour backgroundColour, juce::Colour borderColour, float cornerRatio, float borderThicknessRatio);

void paintBasicWindowRounded(juce::Graphics& g, juce::Component& component, juce::Colour backgroundColour, juce::Colour borderColour);
void paintBasicWindowRounded(juce::Graphics& g, juce::Component& component, juce::Colour backgroundColour, juce::Colour borderColour, float cornerRatio, float borderThicknessRatio);

class BlackaddrAudioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BlackaddrAudioLookAndFeel();
    virtual ~BlackaddrAudioLookAndFeel() = default;
};

class effectControlLookAndFeelKnob : public juce::LookAndFeel_V4 {
public:
    enum class KnobRangeType {
        STOPS = 0,
        FULL_RANGE
    };

    effectControlLookAndFeelKnob() = delete;
    effectControlLookAndFeelKnob(juce::Image& image, KnobRangeType knobRangeType, int knobFrameHeightIn = 0)
      : knobImage(image), knobRangeType(knobRangeType), knobFrameHeight(knobFrameHeightIn) {}
    virtual ~effectControlLookAndFeelKnob() = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
            float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    juce::Image getFrame(juce::Slider& slider);

    juce::Image   knobImage;
    KnobRangeType knobRangeType;
    int           knobFrameHeight;
};

class LedMeterLookAndFeelKnob : public juce::Slider, public juce::LookAndFeel_V4
{
public:
    LedMeterLookAndFeelKnob() = delete;
    LedMeterLookAndFeelKnob(juce::Image imageIn, unsigned imageHeightIn = 0) : image(imageIn), imageHeight(imageHeightIn) {}
    virtual ~LedMeterLookAndFeelKnob() { setLookAndFeel(nullptr); }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
            float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    juce::Image image;
    unsigned    imageHeight = 0;
};

class effectControlLookAndFeelExp : public juce::LookAndFeel_V4
{
public:

    effectControlLookAndFeelExp() = delete;
    effectControlLookAndFeelExp(juce::Image& image, int knobFrameHeightIn = 0) : knobImage(image), knobFrameHeight(knobFrameHeightIn) {}
    virtual ~effectControlLookAndFeelExp() = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
            float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    juce::Image& knobImage;
    int          knobFrameHeight;
};

}

#endif /* SOURCE_APPLOOKANDFEEL_H_ */
