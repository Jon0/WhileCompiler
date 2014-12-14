#include <iostream>

#include "io/Directory.h"
#include "io/Lexer.h"
#include "io/Parser.h"

#include "java/Classfile.h"
#include "java/ClassfileWriter.h"
#include "java/WhileToJava.h"

#include "lang/Common.h"

#include "x86/WhileToX86.h"
#include "x86/X86Program.h"
#include "x86/X86Writer.h"

#include "test/Test.h"
#include "test/InterfaceTest.h"

using namespace std;

bool debug = true;

enum class compile_target {
	java,
	x86
};

int main(int argc, char *argv[]) {

	// parse args for testing directories and target type
	compile_target target = compile_target::x86;
	bool test = false, testI = false, debug = false;
	string testDir = "";
	string filename = "";
	for (int i = 1; i < argc; ++i) {
		string s = argv[i];
		if (s == "--test") {
			test = true;

			// read extra directory param
			if (i < argc - 1) {
				testDir = argv[++i];
			}
			
		}
		else if (s == "--testinterface") {
			testI = true;
		}
		else if (s == "--debug") {
			debug = true;
		}
		else if (s == "--java") {
			target = compile_target::java;
		}
		else if (s == "--x86") {
			target = compile_target::x86;	
		}
		else  {
			filename = s;
		}
	}

	if (test) {
		cout << "using test directory " << testDir << endl;

		// get list of files in test folder
		io::Directory dir(testDir);
		vector<string> wfiles = dir.fileList("while");

		shared_ptr<Test> t = make_shared<Test>();
		t->testDirectory(testDir, wfiles);
		return 0;
	}
	else if (testI) {
		shared_ptr<InterfaceTest> t = make_shared<InterfaceTest>();
		return 0;
	}

	try {
		// read while program
		io::parser::Lexer lex(filename.c_str());
		io::parser::WhileParser parser(lex);
		auto input_program = parser.read();

		// either java or x86 output
		if (target == compile_target::x86) {

			// convert to a x86 assembly program
			shared_ptr<X86Program> x86prog = make_shared<X86Program>();
			shared_ptr<WhileToX86> converter = make_shared<WhileToX86>(x86prog, debug);
			input_program->visit(converter);

			// save executable
			shared_ptr<X86Writer> writer = make_shared<X86Writer>(x86prog, "bin/", "");
			writer->writeExecutable();
		}
		else {
			auto program = make_shared<Classfile>();
			auto converter = make_shared<WhileToJava>(program, debug);
			input_program->visit(converter);

			// write classfile
			auto writer = make_shared<ClassfileWriter>(program, "bin/", "");
			writer->writeClassfile();
		}

	}
	catch (exception &e) {
		cout << "error " << e.what() << endl;
	}

}
