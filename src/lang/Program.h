/*
 * Program.h
 *
 *  Created on: 17/03/2014
 *      Author: remnanjona
 */

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <string>
#include <unordered_set>
#include <vector>

#include "Func.h"
#include "SyntaxElem.h"

namespace lang {
using namespace std;

typedef map<string, shared_ptr<Func>> FuncMap;

class Program: public SyntaxElem, public enable_shared_from_this<Program> {
public:
	Program( FuncMap, string );
	virtual ~Program();

	string getProgramName() const;

	FuncMap getFuncMap() const;

	void run();
	void typeCheck();

	virtual void visit(shared_ptr<SyntaxVisitor> v) {
		return v->accept( shared_from_this() );
	}

	void visitFunc(string, shared_ptr<SyntaxVisitor> v);

private:
	string program_name;
	FuncMap funcs;

};

} /* namespace lang */

#endif /* PROGRAM_H_ */
