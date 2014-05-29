/*
 * Directory.h
 *
 *  Created on: 29/05/2014
 *      Author: remnanjona
 */

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include <string>
#include <vector>

namespace std {

class Directory {
public:
	Directory(string);
	virtual ~Directory();

	vector<string> fileList(string);

private:
	string loc;
};

} /* namespace std */

#endif /* DIRECTORY_H_ */
