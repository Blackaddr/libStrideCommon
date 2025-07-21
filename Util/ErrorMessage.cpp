/*
 * ErrorMessage.cpp
 *
 *  Created on: Jan. 31, 2021
 *      Author: blackaddr
 */
#include <iostream>
#include "Util/ErrorMessage.h"

namespace stride {

#define RED     "\u001b[31m"
#define YELLOW  "\u001b[33m"
#define CYAN    "\u001b[36m"
#define DEFAULT "\u001b[0m"

void errorMessage(const std::string& errorMsg)
{
    std::cout << RED << errorMsg << DEFAULT << std::endl;
}

void warningMessage(const std::string& warningMessage)
{
    std::cout << YELLOW << warningMessage << DEFAULT << std::endl;
}
void noteMessage(const std::string& noteMessage)
{
#if !defined(NDEBUG) || defined(VERBOSE)
    std::cout << CYAN << noteMessage << DEFAULT << std::endl;
#endif
}

void defaultMessage(const std::string& defaultMessage)
{
#if !defined(NDEBUG) || defined(VERBOSE)
    std::cout << DEFAULT << defaultMessage << std::endl;
#endif
}

}
