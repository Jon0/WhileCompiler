/*
 * Constant.h
 *
 *  Created on: 23/06/2014
 *      Author: asdf
 */

#ifndef CONSTANT_H_
#define CONSTANT_H_

#include <string>

#include "Classfile.h"

namespace std {

enum const_type {
	utf8_type = 1,
	integer_type = 3,
	float_type = 4,
	class_type = 7,
	string_type = 8,
	fieldref_type = 9,
	methodref_type = 10,
	name_type = 12
};

typedef vector<shared_ptr<Constant>> const_list;


class Constant {
public:
	Constant(const_type);
	virtual ~Constant() {}

	const_type getType() const;

	const_list getChildren() const {
		 return children;
	}

	bool equalChildren( const_list ) const;

	virtual bool operator!=( const Constant &other ) const;

	virtual bool operator==( const Constant &other ) const;

	virtual bytecode writeByteCode() const;

	virtual string toString() const {
		return "Constant "+to_string(ind);
	}

	// no longer used
	virtual string lookupStr() = 0;
	virtual string typeStr() = 0;

	int getIndex() {
		return ind;
	}

	void setIndex(int i) {
		ind = i;
	}

	bool hasIndex() const {
		return (ind >= 0);
	}

protected:
	int ind; 	// TODO remove

	const_type type;
	const_list children;
};

class UTF8: public Constant {
public:
	UTF8(string c): Constant(utf8_type) {
		str = c;
	}

	virtual bool operator==( const Constant &other ) const;
	virtual bytecode writeByteCode() const;

	virtual string toString() const {
		return "UTF8 "+str;
	}

	virtual string lookupStr();
	virtual string typeStr();

private:
	string str;
};

class JClass: public Constant {
public:
	JClass(string c): Constant(class_type) {
		children.push_back( make_shared<UTF8>(c) );
	}

	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "Class ("+children[0]->toString()+", "+to_string(children[0]->getIndex())+")";
	}
};

class JNameType: public Constant {
public:
	JNameType(string n, string d): Constant(name_type) {
		children.push_back( make_shared<UTF8>(n) );
		children.push_back( make_shared<UTF8>(d) );
	}

	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "NameType ("+children[0]->toString()+", "+children[1]->toString()+")";
	}
};

class JFieldRef: public Constant {
public:
	JFieldRef(string c, string n, string t): Constant(fieldref_type) {
		children.push_back( make_shared<JClass>(c) );
		children.push_back( make_shared<JNameType>(n, t) );
	}

	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "FieldRef ("+children[0]->toString()+", "+children[1]->toString()+")";
	}
};

class JMethodRef: public Constant {
public:
	JMethodRef(string c, string n, string t): Constant(methodref_type) {
		children.push_back( make_shared<JClass>(c) );
		children.push_back( make_shared<JNameType>(n, t) );
	}

	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "MethodRef ("+children[0]->toString()+", "+children[1]->toString()+")";
	}
};

class JString: public Constant {
public:
	JString(string c): Constant(string_type) {
		children.push_back( make_shared<UTF8>(c) );
	}

	virtual bool operator==( const Constant &other ) const;
	virtual bytecode writeByteCode() const;
	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "String ("+children[0]->toString()+", "+to_string(children[0]->getIndex())+")";
	}
};

class JInteger: public Constant {
public:
	JInteger(int n): Constant(integer_type) {
		value = n;
	}

	virtual bool operator==( const Constant &other ) const;
	virtual bytecode writeByteCode() const;
	virtual string lookupStr();
	virtual string typeStr();
	int getValue();

	virtual string toString() const {
		return "Integer " + to_string(value);
	}

private:
	int value;
};

class JFloat: public Constant {
public:
	JFloat(float n): Constant(float_type) {
		value = n;
	}

	virtual bool operator==( const Constant &other ) const;
	virtual bytecode writeByteCode() const;
	virtual string lookupStr();
	virtual string typeStr();

	virtual string toString() const {
		return "Float " + to_string(value);
	}

private:
	float value;
};

} /* namespace std */

#endif /* CONSTANT_H_ */
