/*
 * InterfaceTest.cpp
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Type.h"

#include "../io/Pipe.h"

#include "../java/Classfile.h"
#include "../java/ClassfileWriter.h"

#include "../x86/WhileObject.h"
#include "../x86/X86Function.h"
#include "../x86/X86Program.h"
#include "../x86/X86Reference.h"
#include "../x86/X86Writer.h"

#include "InterfaceTest.h"

namespace std {

InterfaceTest::InterfaceTest() {
	// define the external print function
	print = make_shared<X86Function>("print", false, true);

	//test2();

	test3();
}

InterfaceTest::~InterfaceTest() {
	// TODO Auto-generated destructor stub
}

void InterfaceTest::test1() {
	shared_ptr<X86Program> p = make_shared<X86Program>();
	p->initialise("test");
	p->beginFunction( "main", false );


	shared_ptr<X86ConstRef> number1 = make_shared<X86ConstRef>(16);
	shared_ptr<X86Register> reg = p->getFreeRegister();
	reg->assign( number1 );
	reg->add( make_shared<X86ConstRef>(4) );

	shared_ptr<WhileList> obj = make_shared<WhileList>(p, make_shared<ListType>(intType));
	obj->putOnStack();
	obj->initialise( reg->ref(), true );


	shared_ptr<WhileObject> obj2 = obj->get<WhileObject>( make_shared<X86ConstRef>(0) );
	obj2->initialise( make_shared<X86ConstRef>(42), true );

	shared_ptr<WhileList> obj3 = obj->get<WhileList>( make_shared<X86ConstRef>(1) );
	obj3->initialise( make_shared<X86ConstRef>(3),true );

	shared_ptr<X86Reference> objAddr = obj->addrRef();
	p->callFunction(print, arg_list{objAddr});

	p->endFunction();

	// save executable
	shared_ptr<X86Writer> writer = make_shared<X86Writer>(p, "bin/", "");
	writer->writeExecutable();

	// run program record output
	Pipe pipe;
	string outStr = pipe.exec("./"+writer->filepath());

	cout << outStr << endl;
}

void InterfaceTest::test2() {
	shared_ptr<X86Program> p = make_shared<X86Program>();
	p->initialise("test");
	p->beginFunction( "main", false );



	shared_ptr<WhileObject> obj = make_shared<WhileObject>(p, realType);
	obj->putOnStack();
	obj->initialise( make_shared<X86RealRef>(0.5), true );


	shared_ptr<WhileObject> obj2 = make_shared<WhileObject>(p, realType);
	obj2->putOnStack();
	obj2->initialise( make_shared<X86RealRef>(0.5634973452), true );



	//shared_ptr<X86Register> r2 = make_shared<X86Register>(p, "xmm1");
	//r2->assign( make_shared<X86RealRef>(0.555) );

	shared_ptr<X86Register> r = make_shared<X86Register>(p, "xmm0");
	r->assign( obj->valueDirect() );
	r->add( obj->valueDirect() );
	r->add( obj2->valueDirect() );
	//obj->assign(r->ref(), true);
	//p->callFunction(print, arg_list{obj->addrRef()});

	p->endFunction();

	// save executable
	shared_ptr<X86Writer> writer = make_shared<X86Writer>(p, "bin/", "");
	writer->writeExecutable();

	// run program record output
	Pipe pipe;
	string outStr = pipe.exec("./"+writer->filepath());
	cout << "output:" << endl;
	cout << outStr << endl;
}

void InterfaceTest::test3() {
	cout << "java test" << endl;

	shared_ptr<Classfile> program = make_shared<Classfile>();
	//program->beginFunction("main");

	shared_ptr<ClassfileWriter> writer = make_shared<ClassfileWriter>(program, "bin/", "");
	writer->writeClassfile();

	Pipe pipe;
	//string cmd = "java -cp "+writer->dirctoryname() + " " + writer->filename();
	string cmd = "javap -c "+writer->filepath();
	cout << cmd << endl;
	string outStr = pipe.exec( cmd );
	cout << "output:" << endl;
	cout << outStr << endl;
}

} /* namespace std */
