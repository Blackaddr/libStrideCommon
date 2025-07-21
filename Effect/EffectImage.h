/*
 * EffectImage.h
 *
 *  Created on: Jan. 9, 2021
 *      Author: blackaddr
 */

#ifndef EFFECTIMAGE_H_
#define EFFECTIMAGE_H_

#include <JuceHeader.h>

namespace stride {

// EffectImage is forward declared in EffectFileData simply so it can have a smart pointer
// to the class. We will simply use the JUCE Image class in place here.

/// Effectimage is used to store image files associated with effects such as logs, etc.
class EffectImage {
public:
    EffectImage() = delete;
    EffectImage(juce::Image& imageIn) : image(imageIn) {}
    virtual ~EffectImage() = default;

    bool isValid() { return image.isValid(); }
    unsigned getHeight() { return image.getHeight(); }
    unsigned getWidth()  { return image.getWidth(); }
    juce::Image image;
};

}

#endif /* SOURCE_EFFECTIMAGE_H_ */
