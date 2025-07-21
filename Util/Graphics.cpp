/*
 * AppLookAndFeel.cpp
 *
 *  Created on: Dec. 21, 2020
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Util/Graphics.h"
#include "Resources/BinaryIcons.h"

using namespace juce;

namespace stride {

JUCE_IMPLEMENT_SINGLETON (DefaultControlImages)
JUCE_IMPLEMENT_SINGLETON (DefaultLibraryImages)

void DefaultControlImages::loadImagesFromCache()
{
    if (!isInitialized) {
        defaultPotImage        = ImageCache::getFromMemory(BinaryIcons::defaultPot_png,       BinaryIcons::defaultPot_pngSize);
        defaultEncoderImage    = ImageCache::getFromMemory(BinaryIcons::defaultEncoder_png,   BinaryIcons::defaultEncoder_pngSize);
        defaultIrSelectImage   = ImageCache::getFromMemory(BinaryIcons::defaultEncoder_png,   BinaryIcons::defaultEncoder_pngSize);
        defaultButtonOffImage  = ImageCache::getFromMemory(BinaryIcons::defaultButtonOff_png, BinaryIcons::defaultButtonOff_pngSize);
        defaultButtonOnImage   = ImageCache::getFromMemory(BinaryIcons::defaultButtonOn_png,  BinaryIcons::defaultButtonOn_pngSize);

        isInitialized = true;
    }
}

void DefaultLibraryImages::loadImagesFromCache()
{
    if (!isInitialized) {
        defaultDeleteImage = ImageCache::getFromMemory(BinaryIcons::delete_png,         BinaryIcons::delete_pngSize);
        defaultMissingEfxImage = ImageCache::getFromMemory(BinaryIcons::MissingEfx_png, BinaryIcons::MissingEfx_pngSize);

        isInitialized = true;
    }
}

Image createCompositedImage (Image background, Image foreground, float alpha)
{
    Image result = background.createCopy();

    Graphics g(result);
    g.setOpacity(alpha);
    g.drawImageAt(foreground, 0, 0); // obviously this ignores the fact that they may be different sizes

    return result;
}

}
