/*
 * X86Program.cpp
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#include "X86Program.h"

namespace std {

X86Program::X86Program() {
	// TODO Auto-generated constructor stub

}

X86Program::~X86Program() {
	// TODO Auto-generated destructor stub
}

string X86Program::getName() {
	return name;
}

vector<string> X86Program::getSections() {
	vector<string> a;
	for (auto v: sections) {
		a.push_back(v.first);
	}
	return a;
}

vector<shared_ptr<X86Instruction>> X86Program::getInstructions(string s) {
	return sections[s];
}

void X86Program::setName(string n) {
	name = n;
}

void X86Program::addInstruction( string s, shared_ptr<X86Instruction> i ) {

	// add section if it doesnt exist
	if ( sections.count(s) == 0 ) {
		vector<shared_ptr<X86Instruction>> iv;
		sections.insert( map<string, vector<shared_ptr<X86Instruction>>>::value_type(s, iv) );
	}

	sections[s].push_back(i);
}

} /* namespace std */
