#ifndef INTERFACETEST_H_
#define INTERFACETEST_H_

namespace std {

class X86Function;

class InterfaceTest {
public:
	InterfaceTest();
	virtual ~InterfaceTest();

	void test1();
	void test2();
	void test3();
	void test4();

private:
	shared_ptr<X86Function> print;
};

} /* namespace std */

#endif /* INTERFACETEST_H_ */
