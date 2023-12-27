#pragma once
#include <map>
#include <set>
#include "string_id.hpp"
#include <vector>
#include <list>
#include <memory>
#include <limits>

template<typename T>
using unique = std::unique_ptr<T>;
using std::string;
using std::vector;
using std::list;
using std::map;
using std::set;

struct ExprError{
	string what;
	ExprError(string what):what(what){}
};

struct SafeFloat{
	static constexpr long double epsilon = std::numeric_limits<long double>::epsilon();
	long double value=0;

	operator long double() const {return value;}

	constexpr SafeFloat(){}
	constexpr SafeFloat(long double n):value(n){}

	bool operator==(const SafeFloat& b) const {
		return (value>b.value ? value-b.value : b.value-value) < epsilon * std::max(value<0?-value:value,b.value<0?-b.value:b.value);
	}
	bool operator!=(const SafeFloat& b) const{
		return !(*this==b);
	}

	bool operator>(const SafeFloat& b) const{
		return value > b && !(*this==b);
	}
	bool operator<(const SafeFloat& b) const{
		return value < b && !(*this==b);
	}
	bool operator>=(const SafeFloat& b) const{
		return value >= b || (*this==b);
	}
	bool operator<=(const SafeFloat& b) const{
		return value <= b || (*this==b);
	}

#define OPER(OP)\
SafeFloat operator OP (const SafeFloat& b) const{\
	return value OP b.value;\
}

	OPER(+) OPER(-) OPER(*) OPER(/)
#undef OPER
};

using number_t = SafeFloat;

struct ExprNode;

struct Expr{
	unique<ExprNode> node;

	ID type() const;

	Expr evaluate() const;
	set<ID> find_vars() const;
	Expr substitute(const map<ID,Expr>&) const;
	string to_string(bool force_parentheses=false) const;
	bool same_as(const Expr& b) const;

	//default value is 'undefined'
	Expr(){};
	Expr(const Expr& b);
	Expr(Expr&& b);
	Expr(ExprNode* ptr):node(ptr){}
	Expr(number_t num);
	Expr(bool boo);
};

struct ExprNode{
	const ID type;
	list<Expr> subexprs;

	//direct simplification, as far as possible (ie, 2*2 => 4, 2*2*x => 4*x)
	virtual Expr evaluate() const =0;
	//get all referenced names that don't have a built-in definition
	virtual set<ID> find_vars() const;
	virtual Expr substitute(const map<ID,Expr>&) const =0;
	virtual Expr clone() const =0;
	virtual string to_string(bool force_parentheses=false) const =0;
	virtual bool same_as(const Expr& b) const =0;

	ExprNode(ID type):type(type){}
};

#define SUBEXPR(EXPRTYPE) \
inline static const ID type = #EXPRTYPE;\
Expr evaluate() const override;\
Expr clone() const override{return new EXPRTYPE(*this);}\
string to_string(bool force_parentheses=false) const override;\
Expr substitute(const map<ID,Expr>&) const override;\
bool same_as(const Expr& b) const override;\
EXPRTYPE():ExprNode(#EXPRTYPE){}\

struct Add : public ExprNode{
	SUBEXPR(Add);
};
struct Sub : public ExprNode{
	SUBEXPR(Sub);
};
struct Mul : public ExprNode{
	SUBEXPR(Mul);
};
struct Div : public ExprNode{
	SUBEXPR(Div);
};
struct Exponent : public ExprNode{
	SUBEXPR(Exponent);
};

struct Parenthetical : public ExprNode{
	SUBEXPR(Parenthetical);
};

struct Equal : public ExprNode{
	SUBEXPR(Equal);
};

struct Number : public ExprNode{
	number_t value;

	SUBEXPR(Number);
};

struct Boolean : public ExprNode{
	bool value;
	SUBEXPR(Boolean);
};

struct Variable : public ExprNode{
	ID name;
	set<ID> find_vars() const override;
	SUBEXPR(Variable);
};

struct Function : public ExprNode{
	vector<ID> inputs;
	set<ID> find_vars() const override;

	SUBEXPR(Function);
};

struct Array : public ExprNode{
	SUBEXPR(Array);
};

struct Tuple : public ExprNode{
	SUBEXPR(Tuple);
};



struct And : public ExprNode{
	SUBEXPR(And);
};

struct Or : public ExprNode{
	SUBEXPR(Or);
};

struct Not : public ExprNode{
	SUBEXPR(Not);
};



//disambiguates f times (...)  versus f of (...)
struct Adjacent : public ExprNode{
	SUBEXPR(Adjacent);
};

// eg (x, where x > 3)
struct Restricted : public ExprNode{
	SUBEXPR(Restricted);
};

//a group of mutually exclusive 'Restricted's
struct Piecewise : public ExprNode{
	SUBEXPR(Piecewise);
};

struct Derivative : public ExprNode{
	ID variable;
	SUBEXPR(Derivative);
};

struct DefiniteIntegral : public ExprNode{
	ID variable;
	set<ID> find_vars() const override;
	SUBEXPR(DefiniteIntegral);
};





struct Cosine : public ExprNode{
	SUBEXPR(Cosine);
};
struct Sine : public ExprNode{
	SUBEXPR(Sine);
};
struct Tangent : public ExprNode{
	SUBEXPR(Tangent);
};
