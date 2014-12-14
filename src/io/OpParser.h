#ifndef OPPARSER_H_
#define OPPARSER_H_

namespace io {
namespace parser {

typedef std::pair<std::shared_ptr<lang::Expr>, std::shared_ptr<lang::Expr>> ExprPair;

template<class I, class O> class ZeroParser {
public:
	static O func( std::shared_ptr<lang::Type> in ) {
		auto v = std::make_shared<lang::TypedValue<I>>( in, (I)0 );
		return std::make_shared<lang::ConstExpr>(v);
	}
};

template<class I, class O> class AddParser {
public:
	static O func( ExprPair in ) {
		return std::make_shared<lang::OpExpr<I, I, lang::AddOp<I>>>(in.first, in.second, true);
	}
};

template<class I, class O> class SubParser {
public:
	static O func( ExprPair in ) {
		return std::make_shared<lang::OpExpr<I, I, lang::SubOp<I>>>(in.first, in.second, true);
	}
};

template<class I, class O> class MulParser {
public:
	static O func( ExprPair in ) {
		return std::make_shared<lang::OpExpr<I, I, lang::MulOp<I>>>(in.first, in.second, true);
	}
};

template<class I, class O> class DivParser {
public:
	static O func( ExprPair in ) {
		return std::make_shared<lang::OpExpr<I, I, lang::DivOp<I>>>(in.first, in.second, true);
	}
};

template<class I, class O> class ModParser {
public:
	static O func( ExprPair in ) {
		return std::make_shared<lang::OpExpr<I, I, lang::ModOp<I>>>(in.first, in.second, true);
	}
};

template<class I, class O> class GreaterParser {
public:
	static O func(ExprPair in) {
		return std::make_shared<lang::OpExpr<bool, I, lang::GreaterOp<I>>>(
					std::make_shared<lang::AtomicType<bool>>("bool"),
					in.first, in.second);
	}
};

template<class I, class O> class GreaterEqualParser {
public:
	static O func(ExprPair in) {
		return std::make_shared<lang::OpExpr<bool, I, lang::GreaterEqualOp<I>>>(
					std::make_shared<lang::AtomicType<bool>>("bool"),
					in.first, in.second);
	}
};

template<class I, class O> class LessParser {
public:
	static O func(ExprPair in) {
		return std::make_shared<lang::OpExpr<bool, I, lang::LessOp<I>>>(
					std::make_shared<lang::AtomicType<bool>>("bool"),
					in.first, in.second);
	}
};

template<class I, class O> class LessEqualParser {
public:
	static O func(ExprPair in) {
		return std::make_shared<lang::OpExpr<bool, I, lang::LessEqualOp<I>>>(
					std::make_shared<lang::AtomicType<bool>>("bool"),
					in.first, in.second);
	}
};

} /* namespace parser */
} /* namespace io */

#endif /* OPPARSER_H_ */
