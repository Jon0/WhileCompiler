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

namespace std {

Pipe::Pipe() {
	// TODO Auto-generated constructor stub

}

Pipe::~Pipe() {
	// TODO Auto-generated destructor stub
}

string Pipe::exec(string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

} /* namespace std */
