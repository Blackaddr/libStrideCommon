#pragma once
#include <JuceHeader.h>

// This file is partially ported out of the Gin JUCE module located here:
// https://github.com/FigBug/Gin

/** A very fast blur. This is a compromise between Gaussian Blur and Box blur.
    It creates much better looking blurs than Box Blur, but is 7x faster than some Gaussian Blur
    implementations.
 *
 \param radius from 2 to 254
 */
void applyStackBlur (juce::Image& img, int radius);
