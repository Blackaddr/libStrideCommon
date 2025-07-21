/*
 * BuildCommon.cpp
 *
 *  Created on: Feb. 4, 2022
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/ErrorMessage.h"
#include "Util/FileUtil.h"

#include "Build/BuildCommon.h"

using namespace juce;

namespace stride {

//const std::vector<std::string> BuildCommon::DEV_INCLUDE_LIBS_VEC = {"globalCompat", "sysPlatform", "Avalon", "SPI", "TeensyThreads", "cores"};
const std::vector<std::string> BuildCommon::DEV_INCLUDE_LIBS_VEC = {"globalCompat", "sysPlatform", "Avalon"};



std::string BuildCommon::getEfxInstallPath()
{
    File userAppDir = File::getSpecialLocation(File::userApplicationDataDirectory);
    std::string efxInstallPath = userAppDir.getFullPathName().toStdString() + FileUtil::fileSeparator() +
            APPLICATION_NAME_STR + FileUtil::fileSeparator() + PEDALBOARD_EFFECT_FOLDER;
    return efxInstallPath;
}

void BuildCommon::getCppClassSplit(const std::string& cppClass, std::string& classNamespace, std::string& className)
{
    String cppClassStr = String(cppClass);

    StringArray tokens;
    tokens.addTokens (cppClassStr, ":", "\"");

    String classNamespaceStr = tokens[0];
    String classNameStr         = tokens[tokens.size()-1];

//    std::cout << "cppClass: " << cppClass << std::endl;
//    std::cout << "classNamespace: " << classNamespaceStr.toStdString() << std::endl;
//    std::cout << "className: " << classNameStr.toStdString() << std::endl;

    classNamespace = classNamespaceStr.toStdString();
    className      = classNameStr.toStdString();
}

std::string BuildCommon::getEffectNameSplit(const std::string& cppClass)
{
    std::string effectNamespace;
    std::string effectName;
    getCppClassSplit(cppClass, effectNamespace, effectName);

    return effectName;
}

std::string BuildCommon::getEffectNamespaceSplit(const std::string& cppClass)
{
    std::string effectNamespace;
    std::string effectName;
    getCppClassSplit(cppClass, effectNamespace, effectName);

    return effectNamespace;
}

}
