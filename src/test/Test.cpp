/*
 * Test.cpp
 *
 *  Created on: 9/04/2014
 *      Author: remnanjona
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>

#include "Test.h"

std::string exec(char* cmd) {
    FILE* pipe = popen(cmd, "r");
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




namespace std {

Test::Test() {
	// give list of files in test folder
	vector<string> v = {"a", "b"};

}

Test::~Test() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
