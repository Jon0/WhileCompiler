/*
 * X86Instruction.h
 *
 *  Created on: 28/05/2014
 *      Author: remnanjona
 */

#ifndef X86INSTRUCTION_H_
#define X86INSTRUCTION_H_

#include <memory>
#include <string>
#include <vector>

namespace std {

class X86Reference;

/* base class for instruction types */
class X86Instruction {
public:
	X86Instruction();
	virtual ~X86Instruction();

	virtual string str() = 0;

	virtual bool isLabel() {
		return false;
	}

	virtual string labelCode() {
		return "";
	}
};

class InstrComment: public X86Instruction {
public:
	InstrComment(string t) {
		s = t;
	}

	virtual ~InstrComment() {}

	virtual string str() {
		return "\n\t#\t"+s;
	}

private:
	string s;
};

class InstrDirective: public X86Instruction {
public:
	InstrDirective(string t) {
		type = t;
	}

	InstrDirective(string t, vector<string> a) {
		type = t;
		args = a;
	}
	virtual ~InstrDirective() {}

	virtual string str() {
		string result = "\t."+type+"";
		for (string a: args) { result += "\t\""+a+"\""; }
		return result;
	}

private:
	string type;
	vector<string> args;
};

class InstrGlobl: public X86Instruction {
public:
	InstrGlobl(string l) {labelStr = l;}
	virtual ~InstrGlobl() {}

	virtual string str() {
		return "\t.globl\t"+labelStr;
	}

private:
	string labelStr;
};

class InstrType: public X86Instruction {
public:
	InstrType(string l) {labelStr = l;}
	virtual ~InstrType() {}

	virtual string str() {
		return "\t.type\t"+labelStr+",\t@function";
	}

private:
	string labelStr;
};

/*
 * the label type base
 */
class InstrLabel: public X86Instruction {
public:
	InstrLabel(string l) {labelStr = l;}
	virtual ~InstrLabel() {}

	virtual string str() {
		return labelStr+":";
	}

	virtual bool isLabel() {
		return true;
	}

	virtual string labelCode() {
		return labelStr;
	}

private:
	string labelStr;
};

class InstrFuncLabel: public InstrLabel {
public:
	InstrFuncLabel(string l): InstrLabel(l) {}
	virtual ~InstrFuncLabel() {}

	virtual string str() {
		return labelCode()+":";
	}
};

/*
 * ==================================
 * base type for coding instructions
 * ==================================
 */
class InstrCode: public X86Instruction {
public:
	InstrCode() {}
	virtual ~InstrCode() {}

	string width(shared_ptr<X86Reference> r);
};

class InstrPush: public InstrCode {
public:
	InstrPush(shared_ptr<X86Reference>);
	virtual ~InstrPush();
	virtual string str();

private:
	string type, labelStr;
};

class InstrPop: public InstrCode {
public:
	InstrPop(shared_ptr<X86Reference>);
	virtual ~InstrPop() {}

	virtual string str() {
		return "\tpop"+type+"\t"+labelStr;
	}

private:
	string type, labelStr;
};

class InstrCdq: public InstrCode {
public:
	InstrCdq() {}
	virtual ~InstrCdq() {}

	virtual string str() {
		return "\tcdq";
	}

private:
	string labelStr;
};

class InstrMov: public InstrCode {
public:
	InstrMov(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	InstrMov(string p, shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	virtual ~InstrMov() {}

	virtual string str() {
		return "\tmov"+ext+type+"\t"+from+", "+to;
	}

private:
	string from, to, type, ext;
};

class InstrLea: public InstrCode {
public:
	InstrLea(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	virtual ~InstrLea() {}

	virtual string str() {
		return "\tlea"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrAdd: public InstrCode {
public:
	InstrAdd(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	InstrAdd(int f, shared_ptr<X86Reference> t);
	InstrAdd(string f, string t) {
		from = f; to = t;
		type = "q";

		// TODO use register sizes
		if (f[1] == 'e' || t[1] == 'e') type = "l";
	}
	InstrAdd(string p, string f, string t) {
		from = f; to = t;
		type = p;
	}
	virtual ~InstrAdd() {}

	virtual string str() {
		return "\tadd"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrSub: public InstrCode {
public:
	InstrSub(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	InstrSub(int f, shared_ptr<X86Reference> t);
	InstrSub(string f, string t) {
		from = f; to = t;
		type = "q";

		// TODO use register sizes
		if (f[1] == 'e' || t[1] == 'e') type = "l";
	}
	InstrSub(string p, string f, string t) {
		from = f; to = t;
		type = p;
	}
	virtual ~InstrSub() {}

	virtual string str() {
		return "\tsub"+type+"\t"+from+", "+to;
	}

	void modifyLHS(string s) {
		from = s;
	}

private:
	string from, to, type;
};

class InstrMul: public InstrCode {
public:
	InstrMul(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	InstrMul(int f, shared_ptr<X86Reference> t);
	InstrMul(string f, string t) {
		from = f; to = t;
		type = "q";
		prefix = "i";

		// TODO use register sizes
		if (f[1] == 'e' || t[1] == 'e') type = "l";
	}
	InstrMul(string p, string f, string t) {
		from = f; to = t;
		type = p;
		prefix = "i";
	}
	virtual ~InstrMul() {}

	virtual string str() {
		return "\t"+prefix+"mul"+type+"\t"+from+", "+to;
	}

private:
	string from, to, prefix, type;
};

class InstrDiv: public InstrCode {
public:
	InstrDiv(shared_ptr<X86Reference> f);
	virtual ~InstrDiv() {}

	virtual string str() {
		return "\tidiv"+type+"\t"+from;
	}

private:
	string from, type;
};

class InstrAnd: public InstrCode {
public:
	InstrAnd(string f, string t) {
		from = f; to = t;
		type = "q";

		// TODO use register sizes
		if (f[0] == '$' || f[1] == 'e' || t[1] == 'e') type = "l";
	}
	InstrAnd(string p, string f, string t) {
		from = f; to = t;
		type = p;
	}
	virtual ~InstrAnd() {}

	virtual string str() {
		return "\tand"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrOr: public InstrCode {
public:
	InstrOr(string f, string t) {
		from = f; to = t;
		type = "q";

		// TODO use register sizes
		if (f[0] == '$' || f[1] == 'e' || t[1] == 'e') type = "l";
	}

	InstrOr(string p, string f, string t) {
		from = f; to = t;
		type = p;
	}

	virtual ~InstrOr() {}

	virtual string str() {
		return "\txor"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrXor: public InstrCode {
public:
	InstrXor(string f, string t) {
		from = f; to = t;
		type = "q";

		// TODO use register sizes
		if (f[0] == '$' || f[1] == 'e' || t[1] == 'e') type = "l";
	}
	InstrXor(string p, string f, string t) {
		from = f; to = t;
		type = p;
	}
	virtual ~InstrXor() {}

	virtual string str() {
		return "\txor"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrCmp: public InstrCode {
public:
	InstrCmp(string f, string t) {
		from = f; to = t;
		type = "q";
		if (f[1] == 'e' || t[1] == 'e') type = "l";
	}
	virtual ~InstrCmp() {}

	virtual string str() {
		return "\tcmp"+type+"\t"+from+", "+to;
	}

private:
	string from, to, type;
};

class InstrCast: public InstrCode {
public:
	InstrCast(shared_ptr<X86Reference> f, shared_ptr<X86Reference> t);
	virtual ~InstrCast() {}

	virtual string str() {
		return "\tcvtsi2sdq\t"+from+", "+to;
	}

private:
	string from, to, type, ext;
};

class InstrJ: public InstrCode {
public:
	InstrJ(string ty, string ta) {
		type = ty; target = ta;
	}
	virtual ~InstrJ() {}

	virtual string str() {
		return "\tj"+type+"\t"+target;
	}

private:
	string type, target;
};

class InstrRet: public InstrCode {
public:
	InstrRet() {}
	virtual ~InstrRet() {}

	virtual string str() {
		return "\tret";
	}
};


class InstrCall: public InstrCode {
public:
	InstrCall(string l) {labelStr = l;}
	virtual ~InstrCall() {}

	virtual string str() {
		return "\tcall\t"+labelStr;
	}

private:
	string labelStr;
};

class InstrSet: public InstrCode {
public:
	InstrSet(string l) {
		type = "e";
		labelStr = l;
	}
	InstrSet(string t, string l) {
		type = t;
		labelStr = l;
	}
	virtual ~InstrSet() {}

	virtual string str() {
		return "\tset"+type+"\t"+labelStr;
	}

private:
	string type, labelStr;
};


} /* namespace std */

#endif /* X86INSTRUCTION_H_ */
