#include <iostream>

#include "../lang/Type.h"

#include "../io/Pipe.h"

#include "../java/Classfile.h"
#include "../java/ClassfileWriter.h"
#include "../java/Constant.h"

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

	auto obj = make_shared<WhileList>(p, make_shared<lang::ListType>(lang::intType));
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
	io::Pipe pipe;
	string outStr = pipe.exec("./"+writer->filepath());

	cout << outStr << endl;
}

void InterfaceTest::test2() {
	shared_ptr<X86Program> p = make_shared<X86Program>();
	p->initialise("test");
	p->beginFunction( "main", false );



	auto obj = make_shared<WhileObject>(p, lang::realType);
	obj->putOnStack();
	obj->initialise( make_shared<X86RealRef>(0.5), true );


	auto obj2 = make_shared<WhileObject>(p, lang::realType);
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
	io::Pipe pipe;
	string outStr = pipe.exec("./"+writer->filepath());
	cout << "output:" << endl;
	cout << outStr << endl;
}

void InterfaceTest::test3() {
	shared_ptr<Classfile> program = make_shared<Classfile>();
	program->initialise("Test");
	program->beginFunction("main");
	program->call();

	cout << program->getConstPool()->debug() << endl;

	shared_ptr<ClassfileWriter> writer = make_shared<ClassfileWriter>(program, "bin/", "");
	writer->writeClassfile();

	io::Pipe pipe;
	string cmd = "javap -c "+writer->filepath();
	cout << cmd << endl;
	cout << "--------------" << endl;
	string outStr = pipe.exec( cmd );
	cout << outStr << endl;

	cmd = "java -cp "+writer->dirctoryname() + " " + writer->filename();
	cout << cmd << endl;
	cout << "--------------" << endl;
	outStr = pipe.exec( cmd );
	cout << outStr << endl;
}

void InterfaceTest::test4() {
	shared_ptr<ConstantPool> cp = make_shared<ConstantPool>();

	shared_ptr<JClass> c1 = make_shared<JClass>("rubbish1");
	shared_ptr<JClass> c2 = make_shared<JClass>("rubbish2");
	shared_ptr<JClass> c3 = make_shared<JClass>("rubbish2");

	cout << "1: " << (*c1 == *c2) << endl;
	cout << "2: " << (*c2 == *c3) << endl;

	short i = cp->use( make_shared<JClass>("rubbish") );
	short j = cp->use( make_shared<JClass>("rubbish") );

	cout << "index: " << i << ", " << j << endl;

	cout << cp->debug() << endl;
}

} /* namespace std */
