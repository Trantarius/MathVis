#include "expression.hpp"
#include <cmath>

ID Expr::type() const { return node->type; }
Expr Expr::evaluate() const { return node->evaluate(); }
set<ID> Expr::find_vars() const { return node->find_vars(); }
Expr Expr::substitute(const map<ID,Expr>& context) const { return node->substitute(context); }
string Expr::to_string(bool force_parentheses) const { return node->to_string(force_parentheses); }
bool Expr::same_as(const Expr& b) const { return node->same_as(b); }

Expr::Expr(const Expr& b){
	node=b.node->clone().node;
}
Expr::Expr(Expr&& b){
	node=std::move(b.node);
}
Expr::Expr(number_t num):node(new Number()){
	Number* p = new Number();
	p->value=num;
	node=unique<ExprNode>(p);
}

Expr::Expr(bool boo):node(new Boolean()){
	Boolean* p = new Boolean();
	p->value=boo;
	node=unique<ExprNode>(p);
}

set<ID> ExprNode::find_vars() const {
	set<ID> ret;
	for(const Expr& child : subexprs){
		set<ID> cvars = child.find_vars();
		for(ID id : cvars){
			ret.emplace(id);
		}
	}
	return ret;
}

enum Associativity{
	//NON means association must be explicit
	//LEFT/RIGHT, means that associative direction is assumed, but associative property is not
	//ASSOCIATIVE means any association works
	NON_ASSOCIATIVE, LEFT_ASSOCIATIVE, RIGHT_ASSOCIATIVE, ASSOCIATIVE
};

struct Operator{

	Associativity associativity;
	bool commutative;

	enum{
		NUMBER_TO_NUMBER, NUMBER_TO_BOOL, BOOL_TO_BOOL, UNARY_NUMBER, UNARY_BOOL
	} mode;

	number_t (*number_to_number)(number_t,number_t){};
	bool (*number_to_bool)(number_t,number_t){};
	bool (*bool_to_bool)(bool,bool){};
	number_t (*unary_number)(number_t){};
	bool (*unary_bool)(number_t){};

	const char* name;

	constexpr Operator(){}

};

Expr binary_op(const Operator& op, const Expr& a, const Expr& b){
	if(a.type()=="Number"_id){
		if(b.type()=="Number"_id){

			if(op.mode==Operator::NUMBER_TO_NUMBER){
				Number* ret=new Number;
				ret->value=op.number_to_number(
					dynamic_cast<const Number*>(a.node.get())->value,
					dynamic_cast<const Number*>(b.node.get())->value);
				return ret;
			}

			else if(op.mode==Operator::NUMBER_TO_BOOL){
				Boolean* ret=new Boolean;
				ret->value=op.number_to_bool(
					dynamic_cast<const Number*>(a.node.get())->value,
					dynamic_cast<const Number*>(b.node.get())->value);
				return ret;
			}

			else{
				throw ExprError("cannot call operator '"+string(op.name)+"' on two numbers");
			}
		}

		else if(b.type()=="Boolean"_id){
			throw ExprError("cannot call operator '"+string(op.name)+"' on a number and a boolean");
		}

		else if(b.type()=="Array"_id){
			Array* ret=new Array();
			for(Expr& elem : b.node->subexprs){
				ret->subexprs.push_back(binary_op(op,a,elem));
			}
			return ret;
		}

		else if(b.type()=="Tuple"_id){
			throw ExprError(string("dimensionality mismatch: 1 vs ")+std::to_string(dynamic_cast<const Tuple*>(b.node.get())->subexprs.size()));
		}
	}

	else if(a.type()=="Boolean"_id){
		if(b.type()=="Number"_id){
			throw ExprError("cannot call operator '"+string(op.name)+"' on a boolean and a number");
		}

		else if(b.type()=="Boolean"_id){

			if(op.mode==Operator::BOOL_TO_BOOL){
				Boolean* ret=new Boolean;
				ret->value=op.number_to_bool(
					dynamic_cast<const Boolean*>(a.node.get())->value,
					dynamic_cast<const Boolean*>(b.node.get())->value);
				return ret;
			}

			else{
				throw ExprError("cannot call operator '"+string(op.name)+"' on two booleans");
			}
		}

		if(b.type()=="Array"_id){
			Array* ret=new Array();
			for(Expr& elem : b.node->subexprs){
				ret->subexprs.push_back(binary_op(op,a,elem));
			}
			return ret;
		}

		if(b.type()=="Tuple"_id){
			throw ExprError(string("dimensionality mismatch: 1 vs ")+std::to_string(dynamic_cast<const Tuple*>(b.node.get())->subexprs.size()));
		}
	}

	else if(a.type()=="Array"_id){

		if(b.type()=="Array"_id){
			Array* ret=new Array();
			const Array* a_arr = dynamic_cast<const Array*>(a.node.get());
			const Array* b_arr = dynamic_cast<const Array*>(b.node.get());
			for(const Expr& elem_a : a_arr->subexprs){
				for(const Expr& elem_b : b_arr->subexprs){
					ret->subexprs.push_back(binary_op(op,elem_a,elem_b));
				}
			}
			return ret;
		}

		else{
			Array* ret=new Array();
			Array* a_arr = dynamic_cast<Array*>(a.node.get());
			for(Expr& elem : a_arr->subexprs){
				ret->subexprs.push_back(binary_op(op,elem,b));
			}
			return ret;
		}
	}

	else if(a.type()=="Tuple"_id){
		if(b.type()=="Number"_id || b.type()=="Boolean"_id){
			throw ExprError(string("dimensionality mismatch: 1 vs ")+std::to_string(dynamic_cast<Tuple*>(a.node.get())->subexprs.size()));
		}

		if(b.type()=="Array"_id){
			Array* ret=new Array();
			for(Expr& elem : b.node->subexprs){
				ret->subexprs.push_back(binary_op(op,a,elem));
			}
			return ret;
		}

		if(b.type()=="Tuple"_id){
			Tuple* a_tuple = dynamic_cast<Tuple*>(a.node.get());
			Tuple* b_tuple = dynamic_cast<Tuple*>(a.node.get());
			if(a_tuple->subexprs.size() != b_tuple->subexprs.size()){
				throw ExprError(string("dimensionality mismatch: ") +
					std::to_string(a_tuple->subexprs.size()) + " vs " +
					std::to_string(b_tuple->subexprs.size()));
			}
			Tuple* ret=new Tuple();
			auto it_a = a.node->subexprs.begin();
			auto it_b = b.node->subexprs.begin();
			while(it_a!=a.node->subexprs.end()){
				ret->subexprs.push_back(binary_op(op,*it_a,*it_b));
				it_a++;it_b++;
			}
			return ret;
		}
	}

	//should be unreachable
	return Expr();
}

// assumes left associative binary op sequence, ie a+b+c == (a+b)+c
// does not assume associative or commutative properties
list<Expr> nary_op(const Operator& op, list<Expr>&& children){
	assert(op.associativity!=NON_ASSOCIATIVE);

	Expr&(list<Expr>::*next)() = op.associativity==RIGHT_ASSOCIATIVE ?
		(Expr&(list<Expr>::*)())&list<Expr>::back :
		(Expr&(list<Expr>::*)())&list<Expr>::front;

	void (list<Expr>::* pop)() = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(list<Expr>::*)())&list<Expr>::pop_back :
		(void(list<Expr>::*)())&list<Expr>::pop_front;

	void (list<Expr>::* push)(Expr&&) = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(list<Expr>::*)(Expr&&))&list<Expr>::push_back :
		(void(list<Expr>::*)(Expr&&))&list<Expr>::push_front;

	void (list<Expr>::* push_alt)(Expr&&) = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(list<Expr>::*)(Expr&&))&list<Expr>::push_front :
		(void(list<Expr>::*)(Expr&&))&list<Expr>::push_back;


	list<Expr> alt;
	while(!children.empty()){
		Expr a = std::move((children.*next)());
		(children.*pop)();
		if(children.empty()){
			(alt.*push_alt)(std::move(a));
		}
		else if(a.type()=="Number"_id || a.type()=="Array"_id || a.type()=="Tuple"_id){
			Expr b = std::move((children.*next)());
			(children.*pop)();
			if(b.type()=="Number"_id || b.type()=="Array"_id || b.type()=="Tuple"_id){
				(children.*push)( op.associativity==LEFT_ASSOCIATIVE ? binary_op(op,a,b) : binary_op(op,b,a) );
			}
			else{
				(alt.*push_alt)(std::move(a));
				(alt.*push_alt)(std::move(b));
				if(op.associativity != ASSOCIATIVE){
					break;
				}
			}
		}
		else{
			(alt.*push_alt)(std::move(a));
			if(op.associativity != ASSOCIATIVE){
				break;
			}
		}
	}

	while(!children.empty()){

	}

	return alt;
}

list<Expr> sub_eval(const list<Expr>& subs){
	list<Expr> ret;
	for(const Expr& child : subs){
		ret.push_back(child.evaluate());
	}
	return ret;
}

#define PASS2(THING...) THING
#define PASS(THING...) PASS2(THING)

#define PASTE2(FIRST,SECOND) FIRST##SECOND
#define PASTE(FIRST,SECOND) PASTE2(FIRST,SECOND)

#define STRINGIFY2(THING) #THING
#define STRINGIFY(THING) STRINGIFY2(THING)

#define OP_EXPR_EVAL(EXPRNODE,OPER)                             \
Expr EXPRNODE::evaluate() const {                               \
	EXPRNODE* ret=new EXPRNODE();                                 \
	ret->subexprs=nary_op(OPER,sub_eval(subexprs));               \
	if(ret->subexprs.size()==1){                                  \
		return ret->subexprs.front();                               \
	}                                                             \
	return ret;                                                   \
}

#define SUBSTITUTE_IMPL(EXPRNODE)                                 \
Expr EXPRNODE::substitute(const map<ID,Expr>& context) const {    \
	EXPRNODE* ret=new EXPRNODE();                                   \
	for(const Expr& ex : subexprs){                                 \
		ret->subexprs.push_back(ex.substitute(context));              \
	}                                                               \
	return ret;                                                     \
}

#define REQ_PAREN(TYPE)             \
else if(elem.type()==TYPE::type){   \
	ret+="(";                         \
	ret+=elem.to_string();            \
	ret+=")";                         \
}

#define OP_TOSTRING_IMPL(EXPRNODE,OP,REQ_PAREN_TYPES)          \
string EXPRNODE::to_string(bool force_parentheses) const {     \
	string ret;                                                  \
	if(force_parentheses){                                       \
		ret+="(";                                                  \
	}                                                            \
	for(const Expr& elem : subexprs){                            \
		if(force_parentheses){                                     \
			ret+=elem.to_string(force_parentheses);                  \
		}                                                          \
		REQ_PAREN(EXPRNODE)                                        \
		PASS REQ_PAREN_TYPES                                       \
		else{                                                      \
			ret+=elem.to_string();                                   \
		}                                                          \
		if(&elem!=&subexprs.back()){                               \
			ret+=" " #OP " ";                                        \
		}                                                          \
	}                                                            \
	if(force_parentheses){                                       \
		ret+=")";                                                  \
	}                                                            \
	return ret;                                                  \
}

#define SAME_AS_IMPL(EXPRNODE)                                                 \
bool EXPRNODE::same_as(const Expr& b) const {                                  \
	if(b.type() != type){                                                        \
		return false;                                                              \
	}                                                                            \
	const EXPRNODE* node = dynamic_cast<const EXPRNODE*>(b.node.get());          \
	if(subexprs.size() != node->subexprs.size()){                                \
		return false;                                                              \
	}                                                                            \
	auto a_it = subexprs.begin();                                                \
	auto b_it = node->subexprs.begin();                                          \
	while(a_it!=subexprs.end()){                                                 \
		if(!a_it->same_as(*b_it)){                                                 \
			return false;                                                            \
		}                                                                          \
		a_it++; b_it++;                                                            \
	}                                                                            \
	return true;                                                                 \
}

constexpr Operator op_add=[](){
	Operator op;

	op.associativity=ASSOCIATIVE;
	op.commutative=true;
	op.mode=Operator::NUMBER_TO_NUMBER;
	op.name="+";
	op.number_to_number=[](number_t a,number_t b){return a+b;};

	return op;
}();

OP_EXPR_EVAL(Add,op_add)
SUBSTITUTE_IMPL(Add)
OP_TOSTRING_IMPL(Add,+,())
SAME_AS_IMPL(Add)

constexpr Operator op_sub=[](){
	Operator op;

	op.associativity=LEFT_ASSOCIATIVE;
	op.commutative=false;
	op.mode=Operator::NUMBER_TO_NUMBER;
	op.number_to_number=[](number_t a,number_t b){return a-b;};
	op.name="-";

	return op;
}();

OP_EXPR_EVAL(Sub,op_sub)
SUBSTITUTE_IMPL(Sub)
OP_TOSTRING_IMPL(Sub,-,(REQ_PAREN(Add)))
SAME_AS_IMPL(Sub)

constexpr Operator op_mul=[](){
	Operator op;

	op.associativity=ASSOCIATIVE;
	op.commutative=true;
	op.mode=Operator::NUMBER_TO_NUMBER;
	op.number_to_number=[](number_t a,number_t b){return a*b;};
	op.name="*";

	return op;
}();

OP_EXPR_EVAL(Mul,op_mul)
SUBSTITUTE_IMPL(Mul)
OP_TOSTRING_IMPL(Mul,*,(
	REQ_PAREN(Add)
	REQ_PAREN(Sub)
))
SAME_AS_IMPL(Mul)

constexpr Operator op_div=[](){
	Operator op;

	op.associativity=LEFT_ASSOCIATIVE;
	op.commutative=false;
	op.mode=Operator::NUMBER_TO_NUMBER;
	op.number_to_number=[](number_t a,number_t b){return a/b;};
	op.name="/";

	return op;
}();

OP_EXPR_EVAL(Div,op_div)
SUBSTITUTE_IMPL(Div)
OP_TOSTRING_IMPL(Div,/,(
	REQ_PAREN(Add)
	REQ_PAREN(Sub)
	REQ_PAREN(Mul)
))
SAME_AS_IMPL(Div)

constexpr Operator op_exp=[](){
	Operator op;

	op.associativity=RIGHT_ASSOCIATIVE;
	op.commutative=false;
	op.mode=Operator::NUMBER_TO_NUMBER;
	op.number_to_number=[](number_t a,number_t b)->number_t{return pow(a,b);};
	op.name="^";

	return op;
}();

OP_EXPR_EVAL(Exponent,op_exp)
SUBSTITUTE_IMPL(Exponent)
OP_TOSTRING_IMPL(Exponent,^,(
	REQ_PAREN(Add)
	REQ_PAREN(Sub)
	REQ_PAREN(Mul)
	REQ_PAREN(Div)
))
SAME_AS_IMPL(Exponent)

SUBSTITUTE_IMPL(Parenthetical)
SAME_AS_IMPL(Parenthetical)
Expr Parenthetical::evaluate() const{
	if(subexprs.empty()){
		return Expr();
	}
	return subexprs.front().evaluate();
}
string Parenthetical::to_string(bool force_parentheses) const {
	if(subexprs.empty()){
		return "()";
	}
	if(force_parentheses){
		return subexprs.front().to_string(force_parentheses);
	}
	return "("+subexprs.front().to_string(force_parentheses)+")";
}


constexpr Operator op_eql=[](){
	Operator op;

	op.associativity=ASSOCIATIVE;
	op.mode=Operator::NUMBER_TO_BOOL;
	op.commutative=true;
	op.name="=";
	constexpr number_t epsilon = std::numeric_limits<number_t>::epsilon();
	op.number_to_bool=[](number_t a,number_t b){return a==b;};

	return op;
}();

OP_EXPR_EVAL(Equal,op_eql)
SUBSTITUTE_IMPL(Equal)
SAME_AS_IMPL(Equal)
OP_TOSTRING_IMPL(Equal,=,(
	REQ_PAREN(And)
	REQ_PAREN(Or)
	REQ_PAREN(Not)
))


Expr Number::evaluate() const {
	return clone();
}
Expr Number::substitute(const map<ID,Expr>& context) const{
	return clone();
}
bool Number::same_as(const Expr& b) const {
	if(b.type()!=type){
		return false;
	}
	return dynamic_cast<const Number*>(b.node.get())->value==value;
}
string Number::to_string(bool force_parentheses) const {
	return std::to_string(value);
}


Expr Boolean::evaluate() const {
	return clone();
}
Expr Boolean::substitute(const map<ID,Expr>& context) const{
	return clone();
}
bool Boolean::same_as(const Expr& b) const {
	if(b.type()!=type){
		return false;
	}
	return dynamic_cast<const Boolean*>(b.node.get())->value==value;
}
string Boolean::to_string(bool force_parentheses) const {
	return value ? "true" : "false";
}

Expr Variable::evaluate() const {
	return clone();
}
Expr Variable::substitute(const map<ID,Expr>& context) const {
	if(context.contains(name)){
		return context.at(name);
	}
	else{
		return clone();
	}
}
bool Variable::same_as(const Expr& b) const {
	if(b.type()!=type){
		return false;
	}
	return name==dynamic_cast<const Variable*>(b.node.get())->name;
}
string Variable::to_string(bool force_parentheses) const {
	return (const char*)name;
}
set<ID>Variable::find_vars() const {
	return set<ID>{name};
}

Expr Array::evaluate() const {
	Array* ret=new Array();
	for(const Expr& child : subexprs){
		ret->subexprs.push_back(child.evaluate());
	}
	return ret;
}
SUBSTITUTE_IMPL(Array);
SAME_AS_IMPL(Array);
string Array::to_string(bool force_parentheses) const {
	string ret="[";
	for(const Expr& child : subexprs){
		ret+=child.to_string(force_parentheses);
		if(&child!=&subexprs.back()){
			ret+=", ";
		}
	}
	ret+="]";
	return ret;
}

Expr Tuple::evaluate() const {
	Array* ret=new Array();
	for(const Expr& child : subexprs){
		ret->subexprs.push_back(child.evaluate());
	}
	return ret;
}
SUBSTITUTE_IMPL(Tuple);
SAME_AS_IMPL(Tuple);
string Tuple::to_string(bool force_parentheses) const {
	string ret="(";
	for(const Expr& child : subexprs){
		ret+=child.to_string(force_parentheses);
		if(&child!=&subexprs.back()){
			ret+=", ";
		}
	}
	ret+=")";
	return ret;
}
