/*
 * Pipe.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef PIPE_H_
#define PIPE_H_

#include <string>

namespace io {

class Pipe {
public:
	Pipe();
	virtual ~Pipe();

	/**
	 * executes a command in the current working directory
	 * and returns the result of the command
	 */ 
	std::string exec(std::string);
};

} /* namespace io */

#endif /* PIPE_H_ */
