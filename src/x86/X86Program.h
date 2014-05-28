/*
 * X86Program.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86PROGRAM_H_
#define X86PROGRAM_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace std {

class X86Instruction;

class X86Program {
public:
	X86Program();
	virtual ~X86Program();

	string getName();
	vector<string> getSections();
	vector<shared_ptr<X86Instruction>> getInstructions( string );

	void setName( string );
	void addInstruction( string, shared_ptr<X86Instruction> );

private:
	string name;

	map<string, vector<shared_ptr<X86Instruction>>> sections;
	map<string, shared_ptr<X86Instruction>> labels;

};

} /* namespace std */

#endif /* X86PROGRAM_H_ */
