/*
 * EffectFileWrite.h
 *
 *  Created on: Dec. 28, 2021
 *      Author: blackaddr
 */

#ifndef LIBSTRIDE_COMMON_EFFECT_EFFECTFILEWRITE_H_
#define LIBSTRIDE_COMMON_EFFECT_EFFECTFILEWRITE_H_

#include <string>
#include <vector>
#include <memory>
#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Effect/EffectFileData.h"

namespace stride {

const juce::String NOT_FOUND_STRING     = "<not found>";
const juce::String NOT_SPECIFIED_STRING = "<not specified>";

int writeEfxJson(std::shared_ptr<stride::EffectFileData> effectFileDataPtr, const juce::String& filepath, bool stripGfxPaths = false);

bool isFilenameMessageString(const std::string& filepath);

}

#endif /* LIBSTRIDE_COMMON_EFFECT_EFFECTFILEWRITE_H_ */
