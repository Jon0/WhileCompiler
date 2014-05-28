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

namespace std {

class Test {
public:
	Test();
	virtual ~Test();

	void testDirectory(string, vector<string>);
	void runTest(string, string);

private:
	int passed, total;
};

} /* namespace std */

#endif /* TEST_H_ */
