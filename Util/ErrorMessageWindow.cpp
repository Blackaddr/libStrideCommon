/*
 * ErrorMessageWindow.cpp
 *
 *  Created on: Jan. 17, 2021
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"

using namespace juce;

namespace stride {

static bool g_enableDisplayMessages = true;

void enableDisplayMessages()
{
    g_enableDisplayMessages = true;
}

void disableDisplayMessages()
{
    g_enableDisplayMessages = false;
}

void displayErrorMessage(const std::string& errMessage)
{
    if (g_enableDisplayMessages) { AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "ERROR!", String(errMessage), "OK"); }
    else { errorMessage(errMessage); }
}

void displayWarningMessage(const std::string& warnMessage)
{
    if (g_enableDisplayMessages) { AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Warning: ", String(warnMessage), "OK"); }
    else { warningMessage(warnMessage); }
}

void displayInfoMessage(const std::string& message)
{
    if (g_enableDisplayMessages) { AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "INFO", String(message), "OK"); }
    else { noteMessage(message); }
}

}
