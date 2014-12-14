/*
 * Test.h
 *
 *  Created on: 9/04/2014
 *      Author: remnanjona
 */

#ifndef TEST_H_
#define TEST_H_

#include <string>
#include <vector>

#include "../io/Pipe.h"

namespace std {

class Test {
public:
	Test();
	virtual ~Test();

	void testDirectory(string, vector<string>);
	void runTest(string, string);

private:
	io::Pipe p;
	int passed, completed, total;

	string runX86(shared_ptr<lang::Program>);
	string runJava(shared_ptr<lang::Program>);
};

} /* namespace std */

#endif /* TEST_H_ */
