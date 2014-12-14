/*
 * Pipe.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>

#include "Pipe.h"

namespace io {

Pipe::Pipe() {}

Pipe::~Pipe() {}

std::string Pipe::exec(std::string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";

    // copy output to a std::string
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

} /* namespace io */
