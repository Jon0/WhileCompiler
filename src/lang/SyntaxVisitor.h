#ifndef VISITOR_H_
#define VISITOR_H_

#include <memory>

namespace lang {

class Type;
class Value;

class Func;
class Program;

class BlockStmt;
class InitStmt;
class AssignStmt;
class IfStmt;
class WhileStmt;
class ForStmt;
class PrintStmt;
class EvalStmt;
class ReturnStmt;
class BreakStmt;
class SwitchStmt;

class ConstExpr;
class IsTypeExpr;
class VariableExpr;
class FuncCallExpr;

class ListExpr;
class ListLengthExpr;
class ConcatExpr;
class ListLookupExpr;
class RecordMemberExpr;
class BasicCastExpr;
class RecordExpr;
class AbstractOpExpr;
class EquivOp;
class NotEquivOp;
class AndExpr;
class OrExpr;
class NotExpr;

/**
 * visits elements in the abstract syntax tree
 */
class SyntaxVisitor {
public:
	virtual ~SyntaxVisitor() {}
	virtual void accept(std::shared_ptr<Type>) = 0;
	virtual void accept(std::shared_ptr<Value>) = 0;

	virtual void accept(std::shared_ptr<Func>) = 0;
	virtual void accept(std::shared_ptr<Program>) = 0;

	virtual void accept(std::shared_ptr<BlockStmt>) = 0;
	virtual void accept(std::shared_ptr<InitStmt>) = 0;
	virtual void accept(std::shared_ptr<AssignStmt>) = 0;
	virtual void accept(std::shared_ptr<IfStmt>) = 0;
	virtual void accept(std::shared_ptr<WhileStmt>) = 0;
	virtual void accept(std::shared_ptr<ForStmt>) = 0;
	virtual void accept(std::shared_ptr<PrintStmt>) = 0;
	virtual void accept(std::shared_ptr<EvalStmt>) = 0;
	virtual void accept(std::shared_ptr<ReturnStmt>) = 0;
	virtual void accept(std::shared_ptr<BreakStmt>) = 0;
	virtual void accept(std::shared_ptr<SwitchStmt>) = 0;

	virtual void accept(std::shared_ptr<ConstExpr>) = 0;
	virtual void accept(std::shared_ptr<IsTypeExpr>) = 0;
	virtual void accept(std::shared_ptr<VariableExpr>) = 0;
	virtual void accept(std::shared_ptr<FuncCallExpr>) = 0;

	virtual void accept(std::shared_ptr<ListExpr>) = 0;
	virtual void accept(std::shared_ptr<ListLengthExpr>) = 0;
	virtual void accept(std::shared_ptr<ConcatExpr>) = 0;
	virtual void accept(std::shared_ptr<ListLookupExpr>) = 0;
	virtual void accept(std::shared_ptr<RecordMemberExpr>) = 0;
	virtual void accept(std::shared_ptr<BasicCastExpr>) = 0;
	virtual void accept(std::shared_ptr<RecordExpr>) = 0;
	virtual void accept(std::shared_ptr<AbstractOpExpr>) = 0;
	virtual void accept(std::shared_ptr<EquivOp>) = 0;
	virtual void accept(std::shared_ptr<NotEquivOp>) = 0;
	virtual void accept(std::shared_ptr<AndExpr>) = 0;
	virtual void accept(std::shared_ptr<OrExpr>) = 0;
	virtual void accept(std::shared_ptr<NotExpr>) = 0;

};

} /* namespace lang */

#endif /* VISITOR_H_ */
