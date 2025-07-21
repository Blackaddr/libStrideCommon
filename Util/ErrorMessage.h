/*
 * ErrorMessage.h
 *
 *  Created on: Jan. 31, 2021
 *      Author: blackaddr
 */

#ifndef UTIL_ERRORMESSAGE_H_
#define UTIL_ERRORMESSAGE_H_

#include <string>

namespace stride {

void errorMessage(const std::string& errorMsg);
void warningMessage(const std::string& warningMessage);
void noteMessage(const std::string& noteMessage);
void defaultMessage(const std::string& defaultMessage);

}

#endif /* SOURCE_UTIL_ERRORMESSAGE_H_ */
