#ifndef WHILETOJAVA_H_
#define WHILETOJAVA_H_

#include <memory>
#include <vector>

#include "../lang/SyntaxVisitor.h"

#include "Classfile.h"
#include "ClassfileWriter.h"

namespace std {

class Instruction {
public:
	vector<unsigned char> bytes;

	void modifyArg1(unsigned char c) {
		bytes[1] = c;
	}

	void modifyArg2(unsigned short c) {
		bytes[1] = (unsigned char) (c >> 8);
		bytes[2] = (unsigned char) (c >> 0);
	}

	void modifyArg4(unsigned int c) {
		bytes[1] = (unsigned char) (c >> 24);
		bytes[2] = (unsigned char) (c >> 16);
		bytes[3] = (unsigned char) (c >> 8);
		bytes[4] = (unsigned char) (c >> 0);
	}
};

class WhileToJava: public lang::SyntaxVisitor, public enable_shared_from_this<WhileToJava> {
public:
	WhileToJava(shared_ptr<Classfile> prog, bool debug);
	virtual ~WhileToJava();

	virtual void accept(shared_ptr<lang::Type>);
	virtual void accept(shared_ptr<lang::Value>);
	virtual void accept(shared_ptr<lang::Func>);
	virtual void accept(shared_ptr<lang::Program>);

	virtual void accept(shared_ptr<lang::BlockStmt>);
	virtual void accept(shared_ptr<lang::InitStmt>);
	virtual void accept(shared_ptr<lang::AssignStmt>);
	virtual void accept(shared_ptr<lang::IfStmt>);
	virtual void accept(shared_ptr<lang::WhileStmt>);
	virtual void accept(shared_ptr<lang::ForStmt>);
	virtual void accept(shared_ptr<lang::PrintStmt>);
	virtual void accept(shared_ptr<lang::EvalStmt>);
	virtual void accept(shared_ptr<lang::ReturnStmt>);
	virtual void accept(shared_ptr<lang::BreakStmt>);
	virtual void accept(shared_ptr<lang::SwitchStmt>);

	virtual void accept(shared_ptr<lang::ConstExpr>);
	virtual void accept(shared_ptr<lang::IsTypeExpr>);
	virtual void accept(shared_ptr<lang::VariableExpr>);
	virtual void accept(shared_ptr<lang::FuncCallExpr>);
	virtual void accept(shared_ptr<lang::RecordExpr>);

	virtual void accept(shared_ptr<lang::ListExpr>);
	virtual void accept(shared_ptr<lang::ListLengthExpr>);
	virtual void accept(shared_ptr<lang::ConcatExpr>);
	virtual void accept(shared_ptr<lang::ListLookupExpr>);
	virtual void accept(shared_ptr<lang::RecordMemberExpr>);
	virtual void accept(shared_ptr<lang::BasicCastExpr>);
	virtual void accept(shared_ptr<lang::AbstractOpExpr>);
	virtual void accept(shared_ptr<lang::EquivOp>);
	virtual void accept(shared_ptr<lang::NotEquivOp>);
	virtual void accept(shared_ptr<lang::AndExpr>);
	virtual void accept(shared_ptr<lang::OrExpr>);
	virtual void accept(shared_ptr<lang::NotExpr>);

private:
	shared_ptr<Classfile> classfile;
	ConstantPool &constant_pool;

	// TODO remove
	vector<Instruction> istack;
	int num_locals;
	map<string, int> local_map;
	vector<shared_ptr<lang::Type>> local_type;
	shared_ptr<lang::Type> t_const; // type of the last constant visited;


	int stackSize();

	void addInstruction(unsigned char code);
	void addInstruction1(unsigned char code, unsigned char arg);
	void addInstruction2(unsigned char code, unsigned short arg);
	void addInstruction4(unsigned char code, unsigned int arg);
};

} /* namespace std */

#endif /* WHILETOJAVA_H_ */
