/*
 * InterfaceTest.cpp
 *
 *  Created on: 30/05/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../lang/Type.h"

#include "../io/Pipe.h"

#include "../x86/WhileObject.h"
#include "../x86/X86Function.h"
#include "../x86/X86Program.h"
#include "../x86/X86Reference.h"
#include "../x86/X86Writer.h"

#include "InterfaceTest.h"

namespace std {

InterfaceTest::InterfaceTest() {
	// define the external print function
	shared_ptr<X86Function> print = make_shared<X86Function>("print", true);


	shared_ptr<X86Program> p = make_shared<X86Program>();
	p->initialise("test");
	p->beginFunction( "main" );


	shared_ptr<X86Reference> number1 = make_shared<X86Reference>(16);
	shared_ptr<X86Register> reg = p->getFreeRegister();
	reg->assign( number1 );
	reg->add( make_shared<X86Reference>(4) );

	shared_ptr<WhileList> obj = make_shared<WhileList>(p);
	obj->putOnStack();
	obj->initialise( reg->ref(), true );
	reg->free();


	shared_ptr<WhileObject> obj2 = obj->get<WhileObject>( make_shared<X86Reference>(0) );
	obj2->initialise( make_shared<X86Reference>(42), true );
	obj2->free();

	shared_ptr<WhileList> obj3 = obj->get<WhileList>( make_shared<X86Reference>(1) );
	obj3->initialise( make_shared<X86Reference>(3),true );
	obj3->free();


	reg = p->getFreeRegister();
	reg->assign( number1 );
	reg->compare( number1 );	// sets flags

	// TODO branching control
	string tag1 = ".L0";
	p->addInstruction( "text", make_shared<InstrJ>( "e", tag1 ) );
	//out->addInstruction( "text", make_shared<InstrJ>( "mp", tag2 ) );
	p->addInstruction( "text", make_shared<InstrLabel>( tag1 ) );

	reg->free();

	shared_ptr<X86Reference> objAddr = obj->addrRef();
	p->callFunction(print, arg_list{objAddr});
	objAddr->free();


	p->endFunction();

	// save executable
	shared_ptr<X86Writer> writer = make_shared<X86Writer>(p, "bin/", "");
	writer->writeExecutable();

	// run program record output
	Pipe pipe;
	string outStr = pipe.exec("./"+writer->filepath());

	cout << outStr << endl;

}

InterfaceTest::~InterfaceTest() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
