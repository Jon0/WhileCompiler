#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ClassfileWriter.h"
#include "ConstantPool.h"
#include "JavaFunction.h"

namespace std {

class SyntaxElem;

enum access_flag {
	access_public = 1,
	access_static = 8
};

/**
 * a java classfile contains compiled bytecode
 */
class Classfile: public enable_shared_from_this<Classfile> {
public:
	Classfile();
	virtual ~Classfile();

	/**
	 * called once to set program name and setup classfile
	 */
	void initialise( string class_name );

	/**
	 * create a function in the classfile
	 */
	void beginFunction( string func_name );
	void call();

	string classname();
	shared_ptr<ConstantPool> getConstPool();
	vector<unsigned char> getBytes();

private:
	string class_name_str;
	shared_ptr<lang::Program> program;
	shared_ptr<ConstantPool> cp;
	vector<unsigned char> bytes;

	short superclass, thisclass;

	map<string, shared_ptr<JavaFunction>> functions;

};

} /* namespace std */

#endif /* CLASSFILE_H_ */
