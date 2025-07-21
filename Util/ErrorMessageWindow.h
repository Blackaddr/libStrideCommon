/*
 * ErrorMessage.h
 *
 *  Created on: Jan. 17, 2021
 *      Author: blackaddr
 */

#ifndef UTIL_ERRORMESSAGEWINDOW_H_
#define UTIL_ERRORMESSAGEWINDOW_H_

#include <string>
#include <JuceHeader.h>

namespace stride {

void enableDisplayMessages();
void disableDisplayMessages();

void displayErrorMessage(const std::string& errorMessage);
void displayWarningMessage(const std::string& warningMessage);
void displayInfoMessage(const std::string& message);

}

#endif /* SOURCE_UTIL_ERRORMESSAGEWINDOW_H_ */
