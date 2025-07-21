/*
  ==============================================================================

    EffectInfo.cpp
    Created: 19 Dec 2020 8:17:35am
    Author:  blackaddr

  ==============================================================================
*/

#include "EffectFileData.h"

namespace stride {

const std::string COMPANY_PNG_FILENAME    = "company.png";
const std::string PEDAL_PNG_FILENAME      = "pedal.png";
const std::string PEDAL_BASE_PNG_FILENAME = "baseImage.png";

const std::string BYPASS_CONTROL_NAME      = "Bypass";
const std::string BYPASS_CONTROL_SHORTNAME = "BYP";
const std::string VOLUME_CONTROL_NAME      = "Volume";
const std::string VOLUME_CONTROL_SHORTNAME = "VOL";

EffectControlValue::EffectControlValue(EffectControl& effectControlIn)
: effectControl(effectControlIn),
  value(effectControlIn.config.defaultValue)
{

}


EffectFileData::EffectFileData()
{
}

EffectCategory getEffectCategoryEnum(std::string categoryStr)
{
    if (categoryStr == "Input")       { return EffectCategory::INPUT; }
    if (categoryStr == "Output")      { return EffectCategory::OUTPUT; }
    if (categoryStr == "Unspecified") { return EffectCategory::UNSPECIFIED; }

    if (categoryStr == "Delay")       { return EffectCategory::DELAY; }
    if (categoryStr == "Distortion")  { return EffectCategory::DISTORTION; }
    if (categoryStr == "Dynamics")    { return EffectCategory::DYNAMICS; }
    if (categoryStr == "Fuzz")        { return EffectCategory::FUZZ; }
    if (categoryStr == "Modulation")  { return EffectCategory::MODULATION; }
    if (categoryStr == "Overdrive")   { return EffectCategory::OVERDRIVE; }
    if (categoryStr == "Pitch")       { return EffectCategory::PITCH; }
    if (categoryStr == "Reverb")      { return EffectCategory::REVERB; }
    if (categoryStr == "Wah")         { return EffectCategory::WAH; }
    if (categoryStr == "Filter/EQ")   { return EffectCategory::FILTER_EQ; }
    if (categoryStr == "Synth")       { return EffectCategory::SYNTH; }
    if (categoryStr == "DataPak")     { return EffectCategory::DATAPAK; }

    return EffectCategory::UNSPECIFIED;
}

std::string getEffectCategoryString(EffectCategory effectCategory)
{
    switch(effectCategory) {
    case EffectCategory::INPUT         : return std::string("Input");
    case EffectCategory::OUTPUT        : return std::string("Output");

    case EffectCategory::DELAY         : return std::string("Delay");
    case EffectCategory::DISTORTION    : return std::string("Distortion");
    case EffectCategory::DYNAMICS      : return std::string("Dynamics");
    case EffectCategory::FUZZ          : return std::string("Fuzz");
    case EffectCategory::MODULATION    : return std::string("Modulation");
    case EffectCategory::OVERDRIVE     : return std::string("Overdrive");
    case EffectCategory::PITCH         : return std::string("Pitch");
    case EffectCategory::REVERB        : return std::string("Reverb");
    case EffectCategory::WAH           : return std::string("Wah");
    case EffectCategory::FILTER_EQ     : return std::string("Filter/EQ");
    case EffectCategory::SYNTH         : return std::string("Synth");
    case EffectCategory::DATAPAK       : return std::string("DataPak");

    case EffectCategory::UNSPECIFIED   :
    default:
        return std::string("Unspecified");
    }
}

std::string EffectControl::typeToString(Type type) {
    switch(type) {
    case EffectControl::Type::SWITCH_LATCHING   : return "Switch, latching";
    case EffectControl::Type::SWITCH_MOMENTARY  : return "Switch, momentary";
    case EffectControl::Type::ENCODER           : return "Selection encoder";
    case EffectControl::Type::EXPRESSION_POT    : return "Pot or expression";
    case EffectControl::Type::LED               : return "Status LED";
    case EffectControl::Type::IR_SELECT         : return "IR Selector";
    case EffectControl::Type::LED_MONITOR       : return "LED Monitor";
    case EffectControl::Type::VALUE_MONITOR     : return "Value Monitor";
    case EffectControl::Type::ENCODER_MONITOR   : return "Encoder Monitor";
    default : return "INVALID TYPE";
    }
}

bool EffectControl::isBypassControl()
{
    if ( (name == BYPASS_CONTROL_NAME) && (shortName == BYPASS_CONTROL_SHORTNAME) ) {
        return true;
    } else { return false; }
}

void EffectFileData::debugPrint() {
    std::cout << "\ncompany: " << company << std::endl;
    std::cout << "effectName: " << effectName << std::endl;
    std::cout << "effectDescription: " << effectDescription << std::endl;
    std::cout << "numControls: " << numControls << std::endl;

    for (auto effectControl : controlsVec) {
        std::cout << "controls: name: " << effectControl.name << std::endl;
        std::cout << "controls: shortName: " << effectControl.shortName << std::endl;
        std::cout << "controls: description: " << effectControl.description << std::endl;
        std::cout << "controls: config: " << "\n\t\ttype: " << EffectControl::typeToString(effectControl.config.type) <<
                "\n\t\tminValue: " << effectControl.config.minValue <<
                "\n\t\tmaxValue: " << effectControl.config.maxValue <<
                "\n\t\tdefaultValue: " << effectControl.config.defaultValue <<
                std::endl;
    }
}

}
