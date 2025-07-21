/*
 * LaunchProcess.h
 *
 *  Created on: Jan. 25, 2022
 *      Author: blackaddr
 */

#ifndef LIBSTRIDE_COMMON_BUILD_LAUNCHPROCESS_H_
#define LIBSTRIDE_COMMON_BUILD_LAUNCHPROCESS_H_

#include <string>
#include <vector>

namespace stride {

int LaunchProcessCommand(const std::string& command, std::string& result, const std::string&launchDir = std::string(), unsigned timeoutSeconds = 0);

int LaunchProcessCommandList(const std::vector<std::string>& commandVec, std::string& result, const std::string&launchDir = std::string());

}

#endif /* LIBSTRIDE_COMMON_BUILD_LAUNCHPROCESS_H_ */
