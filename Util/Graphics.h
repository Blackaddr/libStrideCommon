/*
 * AppLookAndFeel.h
 *
 *  Created on: Dec. 13, 2020
 *      Author: blackaddr
 */

#ifndef UTIL_GRAPHICS_H_
#define UTIL_GRAPHICS_H_

#include <string>
#include <JuceHeader.h>

namespace stride {

class DefaultControlImages {
public:
    DefaultControlImages() = default;
    virtual ~DefaultControlImages() { clearSingletonInstance(); }

    void loadImagesFromCache();

    juce::Image defaultPotImage;
    juce::Image defaultEncoderImage;
    juce::Image defaultIrSelectImage;
    juce::Image defaultButtonOffImage;
    juce::Image defaultButtonOnImage;

    bool isInitialized = false;

    JUCE_DECLARE_SINGLETON (DefaultControlImages, true)
};

class DefaultLibraryImages {
public:
    DefaultLibraryImages() = default;
    virtual ~DefaultLibraryImages() { clearSingletonInstance(); }

    void loadImagesFromCache();

    juce::Image defaultDeleteImage;
    juce::Image defaultMissingEfxImage;

    bool isInitialized = false;

    JUCE_DECLARE_SINGLETON (DefaultLibraryImages, true)
};

// Create an alpha blend of two images. Alpha is the opacity of the foreground image
juce::Image createCompositedImage(juce::Image background, juce::Image foreground, float alpha);

}

#endif /* UTIL_GRAPHICS_H_ */
