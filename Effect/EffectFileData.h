/*
  ==============================================================================

    EffectFileData.h
    Created: 19 Dec 2020 8:17:35am
    Author:  blackaddr
    Description:
    This file contains all class for storing all the information stored in an
    EFX datafile.

  ==============================================================================
*/
#ifndef EFFECT_EFFECTFILEDATA_H_
#define EFFECT_EFFECTFILEDATA_H_

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "Util/CommonDefs.h"

namespace stride {

extern const std::string COMPANY_PNG_FILENAME;
extern const std::string PEDAL_PNG_FILENAME;
extern const std::string PEDAL_BASE_PNG_FILENAME;
extern const std::string BYPASS_CONTROL_NAME;
extern const std::string BYPASS_CONTROL_SHORTNAME;
extern const std::string VOLUME_CONTROL_NAME;
extern const std::string VOLUME_CONTROL_SHORTNAME;

class EffectImage; // TODO: replace with include of EffectImage.h

enum class EffectCategory : unsigned {
    UNSPECIFIED = 0,
    DELAY,
    DISTORTION,
    DYNAMICS,
    FILTER_EQ,
    FUZZ,
    MODULATION,
    OVERDRIVE,
    PITCH,
    REVERB,
    SYNTH,
    WAH,
    DATAPAK,
    /* start Designer only categories. Entries below this line are not available to EffectCreator EFX */
    INPUT,
    OUTPUT,
    NUM_CATEGORIES
};
constexpr unsigned LAST_CATEGORY_INDEX_FOR_CREATOR = static_cast<unsigned>(EffectCategory::SYNTH);

enum class AudioStreamType : unsigned {
    INT16 = 0,
    FLOAT32
};

enum class StreamConvertMode : unsigned {
    NO_CONVERSION   = 0,
    INT16_TO_FLOAT  = 1,
    FLOAT_TO_INT16  = 2
};

EffectCategory getEffectCategoryEnum(std::string categoryStr);
std::string    getEffectCategoryString(EffectCategory);

struct EffectControl {
    enum class Type : unsigned {
        SWITCH_LATCHING  = 0,
        SWITCH_MOMENTARY = 1,
        ENCODER          = 2,
        EXPRESSION_POT   = 3,
        LED              = 4, // This LED indicates a control status only
        IR_SELECT        = 5,
        LED_MONITOR      = 6,
        VALUE_MONITOR    = 7, // This is only used for monitors
        ENCODER_MONITOR  = 8, // This is only used for monitors
        INVALID_TYPE     = ENCODER_MONITOR + 1,
        NUM_TYPES
    };

    enum class ConfigValuePosition : unsigned {
        TYPE    = 0,
        MIN     = 1,
        MAX     = 2,
        DEFAULT = 3,
        STEP    = 4
    };

    struct Config {
        Type type;
        float minValue;
        float maxValue;
        float defaultValue;
        float stepValue = 0.1f;
        bool  fullRange = false;
        std::vector<std::string> strings;
        std::pair<int,int> position;
        float scalingRatio    = 1.0f;
        int supressValueLabel = 0;
        int userData          = 0;
    };

    std::string name;
    std::string shortName;
    std::string effectName;      // parent effect name
    std::string effectShortName; // parent effect short name
    std::string libraryName;     // name of the static library
    std::string description;
    Config      config;
    int         index = INVALID_INDEX;
    std::shared_ptr<EffectImage> imagePtr  = nullptr;
    int imageHeight = 0;
    std::shared_ptr<EffectImage> imagePtr2 = nullptr;
    int image2Height = 0;

    std::string imageFilename;
    std::string image2Filename;

    bool isBypassControl();
    static std::string typeToString(Type type);
};

class EffectControlValue
{
public:
    EffectControlValue() = delete;
    EffectControlValue(EffectControl& effectControlIn);

    EffectControl& effectControl;
    float value;
    int   globalParamIndex      = INVALID_INDEX;
    bool  dirty                 = false;
    bool  isMidiUpdate          = false;
    bool  suppressDisplayUpdate = false;
};

class EffectFileData {
public:
    EffectFileData();
    bool        isDevel = false;
    std::string efxFileVersion;
    std::string company;
    std::string effectName;
    std::string effectShortName;
    std::string effectCategory;
    std::string effectDescription;
    unsigned numInputs   = 0;
    unsigned numOutputs  = 0;
    unsigned numControls = 0; // TODO: this should be replaced programatically with calls to controlsVec.size()?
    bool     isInputOutput = false;  /// TODO: thsi should be renamed to 'isBuiltIn'
    bool     isSingleton   = false;
    bool     processMidi   = false;
    bool     isDataPak     = false;
    AudioStreamType audioStreamType;
    std::string iconFilename;
    std::vector<EffectControl> controlsVec;
    std::vector<std::string> platformsVec;

    // Compilation info
    std::vector<std::string> cppHeaderFilenameVec; // the name of the headerfile
    std::string libraryName; // the name of the static library file passed to the linker
    std::string effectFilename;  // the filename for the EFX file
    std::string effectVersion; // the version of the effect. must be x, or x.y or x.y.z
    std::string coreVersion; // the version of the effect. must be x, or x.y or x.y.z
    std::string cppClass; // The name of the AudioProcessor class to be instantiated in target code
    std::string cppInstBase;  // the basename of the instances for the instantiated effect
    std::string constructorParams; // a string of params to pass to the contructor
    unsigned    effectIndexId = 0; // The index in an array of EffectFileData info
    EffectCategory effectCategoryEnum = EffectCategory::UNSPECIFIED;
    float cpuUsage  = 0.0f;
    float ram0Usage = 0.0f;
    float ram1Usage = 0.0f;
    int writableBuffers = 0;

    void debugPrint();

    void setImagePtr        (std::shared_ptr<EffectImage> imagePtr) { m_pedalImagePtr     = imagePtr; }
    void setBaseImagePtr    (std::shared_ptr<EffectImage> imagePtr) { m_pedalBaseImagePtr = imagePtr; }
    void setCompanyLogoPtr  (std::shared_ptr<EffectImage> imagePtr) { m_companyLogoPtr    = imagePtr; }
    void setKnobImagePtr    (std::shared_ptr<EffectImage> imagePtr) { m_knobImagePtr      = imagePtr; }
    void setEncoderImagePtr (std::shared_ptr<EffectImage> imagePtr) { m_encoderImagePtr   = imagePtr; }
    void setIrSelectImagePtr(std::shared_ptr<EffectImage> imagePtr) { m_irSelectImagePtr  = imagePtr; }
    void setButtonImagePtr  (std::shared_ptr<EffectImage> imageOffPtr,
                             std::shared_ptr<EffectImage> imageOnPtr) { m_buttonOffImagePtr = imageOffPtr; m_buttonOnImagePtr = imageOnPtr; }

    std::shared_ptr<EffectImage> getPedalImagePtr()     { return m_pedalImagePtr;   }
    std::shared_ptr<EffectImage> getBasePedalImagePtr() { return m_pedalBaseImagePtr;   }
    std::shared_ptr<EffectImage> getCompanyLogoPtr()    { return m_companyLogoPtr;  }
    std::shared_ptr<EffectImage> getKnobImagePtr()      { return m_knobImagePtr;    }
    std::shared_ptr<EffectImage> getEncoderImagePtr()   { return m_encoderImagePtr; }
    std::shared_ptr<EffectImage> getIrSelectImagePtr()   { return m_irSelectImagePtr; }

    std::shared_ptr<EffectImage> getButtonOnImagePtr()  { return m_buttonOnImagePtr; }
    std::shared_ptr<EffectImage> getButtonOffImagePtr() { return m_buttonOffImagePtr; }

    void setEffectFilename(const std::string& effectFilename) { m_effectFilename = effectFilename; }
    std::string getEffectFilename() { return m_effectFilename; }

protected:
    std::shared_ptr<EffectImage> m_pedalImagePtr     = nullptr;
    std::shared_ptr<EffectImage> m_pedalBaseImagePtr = nullptr;
    std::shared_ptr<EffectImage> m_companyLogoPtr    = nullptr;
    std::shared_ptr<EffectImage> m_knobImagePtr      = nullptr;
    std::shared_ptr<EffectImage> m_encoderImagePtr   = nullptr;
    std::shared_ptr<EffectImage> m_irSelectImagePtr  = nullptr;
    std::shared_ptr<EffectImage> m_buttonOnImagePtr  = nullptr;
    std::shared_ptr<EffectImage> m_buttonOffImagePtr = nullptr;
    std::string                  m_headerFile;
    std::string                  m_effectFilename;
};

}

#endif // EFFECT_EFFECTFILEDATA_H_
