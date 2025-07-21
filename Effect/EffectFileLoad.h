/*
 * EffectFileLoad.h
 *
 *  Created on: Jan. 9, 2021
 *      Author: blackaddr
 */

#ifndef SOURCE_EFFECT_EFFECTFILELOAD_H_
#define SOURCE_EFFECT_EFFECTFILELOAD_H_

#include <string>
#include <vector>
#include <mutex>
#include <JuceHeader.h>

#include "Util/StringUtil.h"
#include "Util/GuiUtil.h"
#include "Effect/EffectImage.h"
#include "Effect/EffectFileData.h"

namespace stride {

const std::string EFFECT_FILE_EXTENSION = "efx";
const std::string EFFECT_FILE_WILDCARD  = "*." +  EFFECT_FILE_EXTENSION;

const std::string EFFECT_BINARY_FILE_EXTENSION   = "dat";
const std::string EFFECT_JSON_FILE_EXTENSION     = "jsn";
const std::string EFFECT_JSON_WILDCARD           = "*." + EFFECT_JSON_FILE_EXTENSION;
const std::string EFFECT_HEADER_FILE_EXTENSION   = "h";
const std::string EFFECT_GRAPHICS_FILE_EXTENSION = "png";

void loadInputsOutputs(std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVec);

std::string getEfxJson(const std::string& efxFilePath);

//void loadEffect(juce::String filePath, std::shared_ptr<EffectFileData> &effectFileDataPtr);

// This function takes advantage of ZIP and PNG support in order to make processing the effect distributuable
// files easier.
/// TODO: change juce::String to std::string
void loadEffects(juce::String filePath, std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVec,
    SemanticVersion coreVersion,stride::BackgroundTask* taskPtr=nullptr);

}

#endif /* SOURCE_EFFECT_EFFECTFILELOAD_H_ */
