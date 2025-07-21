/*
 * EffectFileWrite.cpp
 *
 *  Created on: Dec. 28, 2021
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Util/ErrorMessage.h"
#include "Util/FileUtil.h"
#include "EffectFileSave.h"


using namespace juce;

namespace stride {

bool isFilenameMessageString(const std::string& filepath)
{
    String filepathStr = String(filepath);
    bool isMessage = filepathStr.contains(NOT_FOUND_STRING) ||
                      filepathStr.contains(NOT_SPECIFIED_STRING);
    return isMessage;
}


int writeEfxJson(std::shared_ptr<stride::EffectFileData> effectFileDataPtr, const  juce::String& filepath, bool stripGfxPaths)
{
    DynamicObject* jsonEffectDataObjPtr = new DynamicObject();

    std::string version = JUCEApplication::getInstance()->getApplicationVersion().toStdString();

    // Primary fields
    jsonEffectDataObjPtr->setProperty("efxFileVersion", String(version));
    jsonEffectDataObjPtr->setProperty("company",    String(effectFileDataPtr->company));
    jsonEffectDataObjPtr->setProperty("effectName", String(effectFileDataPtr->effectName));
    jsonEffectDataObjPtr->setProperty("effectVersion", String(effectFileDataPtr->effectVersion));
    jsonEffectDataObjPtr->setProperty("coreVersion", String(effectFileDataPtr->coreVersion));
    jsonEffectDataObjPtr->setProperty("effectShortName", String(effectFileDataPtr->effectShortName));
    jsonEffectDataObjPtr->setProperty("effectCategory", String(effectFileDataPtr->effectCategory));
    jsonEffectDataObjPtr->setProperty("effectDescription", String(effectFileDataPtr->effectDescription));
    jsonEffectDataObjPtr->setProperty("numInputs", static_cast<int>(effectFileDataPtr->numInputs));
    jsonEffectDataObjPtr->setProperty("numOutputs", static_cast<int>(effectFileDataPtr->numOutputs));
    jsonEffectDataObjPtr->setProperty("numControls", static_cast<int>(effectFileDataPtr->controlsVec.size()));
    jsonEffectDataObjPtr->setProperty("effectFilename", String(effectFileDataPtr->effectFilename));
    jsonEffectDataObjPtr->setProperty("libraryName", String(effectFileDataPtr->libraryName));
    jsonEffectDataObjPtr->setProperty("cppClass", String(effectFileDataPtr->cppClass));
    jsonEffectDataObjPtr->setProperty("cppInstBase", String(effectFileDataPtr->cppInstBase));
    jsonEffectDataObjPtr->setProperty("constructorParams", String(effectFileDataPtr->constructorParams));
    jsonEffectDataObjPtr->setProperty("effectCategory", String(getEffectCategoryString(effectFileDataPtr->effectCategoryEnum)));
    jsonEffectDataObjPtr->setProperty("isSingleton", effectFileDataPtr->isSingleton);
    jsonEffectDataObjPtr->setProperty("processMidi", effectFileDataPtr->processMidi);
    jsonEffectDataObjPtr->setProperty("isDataPak", effectFileDataPtr->isDataPak);
    jsonEffectDataObjPtr->setProperty("audioStreamType", static_cast<int>(effectFileDataPtr->audioStreamType));
    jsonEffectDataObjPtr->setProperty("cpuUsage",  String(effectFileDataPtr->cpuUsage));
    jsonEffectDataObjPtr->setProperty("ram0Usage", String(effectFileDataPtr->ram0Usage));
    jsonEffectDataObjPtr->setProperty("ram1Usage", String(effectFileDataPtr->ram1Usage));
    jsonEffectDataObjPtr->setProperty("writableBuffers", static_cast<int>(effectFileDataPtr->writableBuffers));

    var platformsArrayObj;
    std::string platforms;
    for (auto platform : effectFileDataPtr->platformsVec) {
        platformsArrayObj.append(var(platform));
    }

    // Type 1 is only inserted for Devel builds
    int type = effectFileDataPtr->isDevel ? 1 : 0;
    if (type == 1) { jsonEffectDataObjPtr->setProperty("type", static_cast<int>(type)); }

    // 'controls' array
    var controlArrayObj;

    // loop over each effect control
    for (auto &effectControlParam : effectFileDataPtr->controlsVec) {
        DynamicObject* jsonControlObjPtr = new DynamicObject();

        jsonControlObjPtr->setProperty("name", String(effectControlParam.name));
        jsonControlObjPtr->setProperty("shortName", String(effectControlParam.shortName));
        jsonControlObjPtr->setProperty("description", String(effectControlParam.description));

        var configArrayObj;
        configArrayObj.append(var(static_cast<int>(effectControlParam.config.type)));           // type
        configArrayObj.append(var(static_cast<float>(effectControlParam.config.minValue)));     // min
        configArrayObj.append(var(static_cast<float>(effectControlParam.config.maxValue)));     // max
        configArrayObj.append(var(static_cast<float>(effectControlParam.config.defaultValue))); // type
        configArrayObj.append(var(static_cast<float>(effectControlParam.config.stepValue)));    // step
        jsonControlObjPtr->setProperty("config", configArrayObj);

        switch (effectControlParam.config.type) {
        case EffectControl::Type::SWITCH_LATCHING  :
        case EffectControl::Type::SWITCH_MOMENTARY :
        case EffectControl::Type::LED_MONITOR :
            if (!effectControlParam.imageFilename.empty() && !isFilenameMessageString(effectControlParam.imageFilename)) {
                std::string filename = effectControlParam.imageFilename;
                filename = FileUtil::setPathSeparators(filename);
                // prepend a file separator (i.e. slash) to force a valid path, then strip the directory off the path
                if (stripGfxPaths) { filename = FileUtil::getFilenameFromRelativePath(filename); }
                jsonControlObjPtr->setProperty("iconOn", String(filename));
                jsonControlObjPtr->setProperty("iconOnHeight", var(static_cast<int>(effectControlParam.imageHeight)));
            }
            if (!effectControlParam.image2Filename.empty() && !isFilenameMessageString(effectControlParam.image2Filename)) {
                std::string filename = effectControlParam.image2Filename;
                filename = FileUtil::setPathSeparators(filename);
                // prepend a file separator (i.e. slash) to force a valid path, then strip the directory off the path
                if (stripGfxPaths) { filename = FileUtil::getFilenameFromRelativePath(filename); }
                jsonControlObjPtr->setProperty("iconOff", String(filename));
                jsonControlObjPtr->setProperty("iconOffHeight", var(static_cast<int>(effectControlParam.image2Height)));
            }

            break;
        case EffectControl::Type::EXPRESSION_POT :
        case EffectControl::Type::VALUE_MONITOR :
            if (!effectControlParam.imageFilename.empty() && !isFilenameMessageString(effectControlParam.imageFilename)) {
            //if (FileUtil::fileExists(effectControlParam.imageFilename)) {
            //if (!effectControlParam.imageFilename.empty()) {
                std::string filename = effectControlParam.imageFilename;
                filename = FileUtil::setPathSeparators(filename);
                // prepend a file separator (i.e. slash) to force a valid path, then strip the directory off the path
                if (stripGfxPaths) { filename = FileUtil::getFilenameFromRelativePath(filename); }
                jsonControlObjPtr->setProperty("iconPot", String(filename));
                jsonControlObjPtr->setProperty("iconPotHeight", var(static_cast<int>(effectControlParam.imageHeight)));
            }
            if (effectControlParam.config.fullRange)  {
                jsonControlObjPtr->setProperty("potFullRange", var(static_cast<int>(effectControlParam.config.fullRange)));
            }
            break;

        case EffectControl::Type::ENCODER :
        case EffectControl::Type::ENCODER_MONITOR :
        {
            if (!effectControlParam.imageFilename.empty() && !isFilenameMessageString(effectControlParam.imageFilename)) {
            //if (FileUtil::fileExists(effectControlParam.imageFilename)) {
            //if (!effectControlParam.imageFilename.empty()) {
                std::string filename = effectControlParam.imageFilename;
                filename = FileUtil::setPathSeparators(filename);
                // prepend a file separator (i.e. slash) to force a valid path, then strip the directory off the path
                if (stripGfxPaths) { filename = FileUtil::getFilenameFromRelativePath(filename); }
                jsonControlObjPtr->setProperty("iconEncoder", String(filename));
                jsonControlObjPtr->setProperty("iconEncoderHeight", var(static_cast<int>(effectControlParam.imageHeight)));
            }

            if (!effectControlParam.config.strings.empty()) {
                var enumArrayObj;
                for (auto & enumString : effectControlParam.config.strings) {
                    enumArrayObj.append(var(String(enumString)));
                }
                jsonControlObjPtr->setProperty("enums", enumArrayObj);
            }

            break;
        }

        case EffectControl::Type::IR_SELECT :
        {
            if (!effectControlParam.imageFilename.empty() && !isFilenameMessageString(effectControlParam.imageFilename)) {
                std::string filename = effectControlParam.imageFilename;
                filename = FileUtil::setPathSeparators(filename);
                // prepend a file separator (i.e. slash) to force a valid path, then strip the directory off the path
                if (stripGfxPaths) { filename = FileUtil::getFilenameFromRelativePath(filename); }
                jsonControlObjPtr->setProperty("iconIrSelect", String(filename));
                jsonControlObjPtr->setProperty("iconIrSelectHeight", var(static_cast<int>(effectControlParam.imageHeight)));
            }

            break;
        }

        default:
            break;
        }
        controlArrayObj.append(var(jsonControlObjPtr));

        // if the control has a position and scaling ratio
        if ((effectControlParam.config.position.first >= 0) && (effectControlParam.config.position.second >= 0) &&
            (effectControlParam.config.scalingRatio != 0.0f) ) {
            var positionArrayObj;
            positionArrayObj.append(var(effectControlParam.config.position.first));
            positionArrayObj.append(var(effectControlParam.config.position.second));
            jsonControlObjPtr->setProperty("position", positionArrayObj);

            jsonControlObjPtr->setProperty("scalingRatio", static_cast<float>(effectControlParam.config.scalingRatio));
        }

        if (effectControlParam.config.supressValueLabel) {
            jsonControlObjPtr->setProperty("supressValueLabel", var(static_cast<int>(1))); // set the suppress value label flag
        }

        jsonControlObjPtr->setProperty("userData", var(effectControlParam.config.userData)); // set the suppress value label flag

    }

    jsonEffectDataObjPtr->setProperty("platforms", platformsArrayObj); // Add to the parent JSON object
    jsonEffectDataObjPtr->setProperty("controls", controlArrayObj); // Add to the parent JSON object

    std::string msg = "writeEffectJson(): writing JSON effect config file to " + filepath.toStdString();
    noteMessage(msg);

    var jsonVar(jsonEffectDataObjPtr);
    String jsonJuceStr = JSON::toString(jsonVar);
    int result = FileUtil::writeStringToFile(jsonJuceStr.toStdString(), filepath.toStdString());

    return result;
}

}
