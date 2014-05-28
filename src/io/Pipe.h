/*
 * Pipe.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef PIPE_H_
#define PIPE_H_

#include <string>

namespace std {

class Pipe {
public:
	Pipe();
	virtual ~Pipe();

	string exec(string);
};

} /* namespace std */

#endif /* PIPE_H_ */
