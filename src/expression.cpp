#include "expression.hpp"
#include <cmath>

ID Expr::type() const {
	if(defined())
		return node->type;
	else
		return ID();
}
Expr Expr::evaluate() const {
	if(defined())
		return node->evaluate();
	else
		return Expr();
}
set<ID> Expr::find_vars() const {
	if(defined())
		return node->find_vars();
	else
		return set<ID>();
}
Expr Expr::substitute(const map<ID,Expr>& context) const {
	if(defined())
		return node->substitute(context);
	else
		return Expr();
}
string Expr::to_string(bool force_parentheses) const {
	if(defined())
		return node->to_string(force_parentheses);
	else
		return "∅";
}
bool Expr::same_as(const Expr& b) const {
	if(defined())
		return node->same_as(b);
	else
		return false;
}

Expr::Expr(const Expr& b){
	if(b.defined())
		node=b.node->clone().node;
}
Expr::Expr(Expr&& b){
	if(b.defined())
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

Expr& Expr::operator=(const Expr& b){
	if(b.defined())
		node=b.node->clone().node;
	return *this;
}

Expr& Expr::operator=(Expr&& b){
	if(b.defined())
		node=std::move(b.node);
	return *this;
}

set<ID> ExprNode::find_vars() const {
	set<ID> ret;
	for(const Expr& child : subexprs){
		if(!child.defined()) continue;
		set<ID> cvars = child.find_vars();
		for(ID id : cvars){
			ret.emplace(id);
		}
	}
	return ret;
}

enum Associativity{
	//NON means association must be explicit
	//LEFT/RIGHT, means that associative direction is assumed, but otherwise non-associative
	//ASSOCIATIVE means any association works
	NON_ASSOCIATIVE, LEFT_ASSOCIATIVE, RIGHT_ASSOCIATIVE, ASSOCIATIVE
};

struct Operator{

	Associativity associativity=NON_ASSOCIATIVE;
	bool commutative=false;

	static constexpr uint8_t NOTHING=1<<1;
	static constexpr uint8_t NUMBER=1<<2;
	static constexpr uint8_t BOOLEAN=1<<3;
	static constexpr uint8_t ARRAY=1<<4;
	static constexpr uint8_t TUPLE=1<<5;
	static constexpr uint8_t FUNCTION=1<<6;

	static constexpr uint8_t SCALAR=NUMBER|BOOLEAN|FUNCTION;
	static constexpr uint8_t COLLECTION=ARRAY|TUPLE;
	static constexpr uint8_t SOMETHING=SCALAR|COLLECTION;
	static constexpr uint8_t ANYTHING=NOTHING|SOMETHING;

	uint8_t left_argt{},right_argt{};

	Expr (*do_op)(const Expr&,const Expr&){};

	const char* name;

	constexpr Operator(){}

};

bool etype_is_value(ID type){
	return type=="Boolean"_id || type=="Number"_id || type=="Function"_id ||
		type=="Array"_id || type=="Tuple"_id;
}

Expr binary_op(const Operator& op, const Expr& a, const Expr& b){

	if(a.type()=="Array"_id){
		if(!(op.left_argt&Operator::ARRAY)){

			if(b.type()=="Array"_id){
				if(!(op.right_argt&Operator::ARRAY)){
					Array* ret=new Array();
					for(const Expr& elem_a : a.node.get()->subexprs){
						for(const Expr& elem_b : b.node.get()->subexprs){
							ret->subexprs.push_back(binary_op(op,elem_a,elem_b));
						}
					}
					return ret;
				}
			}

			Array* ret=new Array();
			for(const Expr& elem : a.node.get()->subexprs){
				ret->subexprs.push_back(binary_op(op,elem,b));
			}
			return ret;
		}
	}

	if(b.type()=="Array"_id){
		if(!(op.right_argt&Operator::ARRAY)){
			Array* ret=new Array();
			for(const Expr& elem : b.node.get()->subexprs){
				ret->subexprs.push_back(binary_op(op,a,elem));
			}
			return ret;
		}
	}

	if(a.type()=="Tuple"_id){
		if(!(op.left_argt&Operator::TUPLE)){

			if(b.type()=="Tuple"_id){

				const Tuple* a_tup = dynamic_cast<const Tuple*>(a.node.get());
				const Tuple* b_tup = dynamic_cast<const Tuple*>(b.node.get());
				if(a_tup->subexprs.size()!=b_tup->subexprs.size()){
					throw ExprError("dimensionality mismatch: "+std::to_string(a_tup->subexprs.size())+" vs "+std::to_string(b_tup->subexprs.size()));
				}

				Tuple* ret=new Tuple();
				auto a_it = a_tup->subexprs.begin();
				auto b_it = b_tup->subexprs.begin();
				while(a_it!=a_tup->subexprs.end()){
					ret->subexprs.push_back(binary_op(op,*a_it,*b_it));
					a_it++; b_it++;
				}
				return ret;
			}else{
				throw ExprError("operator "+string(op.name)+" cannot take a tuple as left argument");
			}
		}
	}

	if(b.type()=="Tuple"_id){
		if(!(op.right_argt&Operator::TUPLE)){
			throw ExprError("operator "+string(op.name)+" cannot take a tuple as right argument");
		}
	}

	if(!a.defined()){
		if(!(op.left_argt&Operator::NOTHING)){
			throw ExprError("operator "+string(op.name)+" cannot have nothing as left operand");
		}
	}
	else if(a.type()=="Number"_id){
		if(!(op.left_argt&Operator::NUMBER)){
			throw ExprError("operator "+string(op.name)+" cannot have a number as left operand");
		}
	}
	else if(a.type()=="Boolean"_id){
		if(!(op.left_argt&Operator::BOOLEAN)){
			throw ExprError("operator "+string(op.name)+" cannot have a boolean as left operand");
		}
	}
	else if(a.type()=="Function"_id){
		if(!(op.left_argt&Operator::FUNCTION)){
			throw ExprError("operator "+string(op.name)+" cannot have a function as left operand");
		}
	}

	if(!b.defined()){
		if(!(op.right_argt&Operator::NOTHING)){
			throw ExprError("operator "+string(op.name)+" cannot have nothing as right operand");
		}
	}
	else if(b.type()=="Number"_id){
		if(!(op.right_argt&Operator::NUMBER)){
			throw ExprError("operator "+string(op.name)+" cannot have a number as right operand");
		}
	}
	else if(b.type()=="Boolean"_id){
		if(!(op.right_argt&Operator::BOOLEAN)){
			throw ExprError("operator "+string(op.name)+" cannot have a boolean as right operand");
		}
	}
	else if(b.type()=="Function"_id){
		if(!(op.right_argt&Operator::FUNCTION)){
			throw ExprError("operator "+string(op.name)+" cannot have a function as right operand");
		}
	}

	return op.do_op(a,b);
}

// assumes left associative binary op sequence, ie a+b+c == (a+b)+c
// does not assume associative or commutative properties
deque<Expr> nary_op(const Operator& op, deque<Expr>&& children){
	assert(op.associativity!=NON_ASSOCIATIVE);

	Expr&(deque<Expr>::*next)() = op.associativity==RIGHT_ASSOCIATIVE ?
		(Expr&(deque<Expr>::*)())&deque<Expr>::back :
		(Expr&(deque<Expr>::*)())&deque<Expr>::front;

	void (deque<Expr>::* pop)() = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(deque<Expr>::*)())&deque<Expr>::pop_back :
		(void(deque<Expr>::*)())&deque<Expr>::pop_front;

	void (deque<Expr>::* push)(Expr&&) = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(deque<Expr>::*)(Expr&&))&deque<Expr>::push_back :
		(void(deque<Expr>::*)(Expr&&))&deque<Expr>::push_front;

	void (deque<Expr>::* push_alt)(Expr&&) = op.associativity==RIGHT_ASSOCIATIVE ?
		(void(deque<Expr>::*)(Expr&&))&deque<Expr>::push_front :
		(void(deque<Expr>::*)(Expr&&))&deque<Expr>::push_back;


	deque<Expr> alt;
	while(!children.empty()){
		Expr a = std::move((children.*next)());
		(children.*pop)();
		if(children.empty()){
			(alt.*push_alt)(std::move(a));
		}
		else if(!a.defined()||etype_is_value(a.type())){
			Expr b = std::move((children.*next)());
			(children.*pop)();
			if(!b.defined()||etype_is_value(b.type())){
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
		Expr tmp = std::move((children.*next)());
		(children.*pop)();
		(alt.*push_alt)(std::move(tmp));
	}

	return alt;
}

deque<Expr> sub_eval(const deque<Expr>& subs){
	deque<Expr> ret;
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

#define NARY_OP_EXPR_EVAL(EXPRNODE,OPER)                        \
Expr EXPRNODE::evaluate() const {                               \
	EXPRNODE* ret=new EXPRNODE();                                 \
	ret->subexprs=nary_op(OPER,sub_eval(subexprs));               \
	if(ret->subexprs.size()==1){                                  \
		return ret->subexprs.front();                               \
	}                                                             \
	return ret;                                                   \
}

#define BIOP_EXPR_EVAL(EXPRNODE,OPER)                           \
Expr EXPRNODE::evaluate() const {                               \
	if(subexprs.size()!=2) \
		throw ExprError("operator "+string(OPER.name)+" is strictly binary"); \
	EXPRNODE* ret=new EXPRNODE();                                 \
	ret->subexprs=sub_eval(subexprs);  \
	if((!ret->subexprs.front().defined()||etype_is_value(ret->subexprs.front().type())) && (!ret->subexprs.back().defined()||etype_is_value(ret->subexprs.back().type())) ){ \
		return binary_op(OPER,ret->subexprs.front(),ret->subexprs.back());      \
	}else{ \
		return ret; \
	} \
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
	op.left_argt=Operator::NUMBER;
	op.right_argt=Operator::NUMBER;
	op.name="+";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* a_num = dynamic_cast<const Number*>(a.node.get());
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		Number* ret=new Number();
		ret->value = a_num->value + b_num->value;
		return ret;
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Add,op_add)
SUBSTITUTE_IMPL(Add)
OP_TOSTRING_IMPL(Add,+,(
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))
SAME_AS_IMPL(Add)


constexpr Operator op_sub=[](){
	Operator op;

	op.associativity=LEFT_ASSOCIATIVE;
	op.commutative=false;
	op.left_argt=Operator::NUMBER|Operator::NOTHING;
	op.right_argt=Operator::NUMBER;
	op.name="-";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		number_t a_num,b_num;
		if(a.defined())
			a_num = dynamic_cast<const Number*>(a.node.get())->value;
		else
			a_num=0;
		b_num = dynamic_cast<const Number*>(b.node.get())->value;
		Number* ret=new Number();
		ret->value = a_num - b_num;
		return ret;
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Sub,op_sub)
SUBSTITUTE_IMPL(Sub)
OP_TOSTRING_IMPL(Sub,-,(
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))
SAME_AS_IMPL(Sub)

constexpr Operator op_mul=[](){
	Operator op;

	op.associativity=ASSOCIATIVE;
	op.commutative=true;
	op.left_argt=Operator::NUMBER;
	op.right_argt=Operator::NUMBER;
	op.name="*";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* a_num = dynamic_cast<const Number*>(a.node.get());
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		Number* ret=new Number();
		ret->value = a_num->value * b_num->value;
		return ret;
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Mul,op_mul)
SUBSTITUTE_IMPL(Mul)
OP_TOSTRING_IMPL(Mul,*,(
	REQ_PAREN(Sub)
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))
SAME_AS_IMPL(Mul)

constexpr Operator op_div=[](){
	Operator op;

	op.associativity=LEFT_ASSOCIATIVE;
	op.commutative=false;
	op.left_argt=Operator::NUMBER;
	op.right_argt=Operator::NUMBER;
	op.name="/";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* a_num = dynamic_cast<const Number*>(a.node.get());
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		Number* ret=new Number();
		ret->value = a_num->value / b_num->value;
		return ret;
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Div,op_div)
SUBSTITUTE_IMPL(Div)
OP_TOSTRING_IMPL(Div,/,(
	REQ_PAREN(Mul)
	REQ_PAREN(Sub)
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))
SAME_AS_IMPL(Div)

constexpr Operator op_exp=[](){
	Operator op;

	op.associativity=RIGHT_ASSOCIATIVE;
	op.commutative=false;
	op.left_argt=Operator::NUMBER;
	op.right_argt=Operator::NUMBER;
	op.name="^";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* a_num = dynamic_cast<const Number*>(a.node.get());
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		Number* ret=new Number();
		ret->value = pow(a_num->value , b_num->value);
		return ret;
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Exponent,op_exp)
SUBSTITUTE_IMPL(Exponent)
OP_TOSTRING_IMPL(Exponent,^,(
	REQ_PAREN(Div)
	REQ_PAREN(Mul)
	REQ_PAREN(Sub)
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
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

	op.associativity=NON_ASSOCIATIVE;
	op.commutative=true;
	op.left_argt=Operator::NUMBER;
	op.right_argt=Operator::NUMBER;
	op.name="=";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* a_num = dynamic_cast<const Number*>(a.node.get());
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		Boolean* ret=new Boolean();
		ret->value = a_num->value == b_num->value;
		return ret;
	};

	return op;
}();

BIOP_EXPR_EVAL(Equal,op_eql)
SUBSTITUTE_IMPL(Equal)
SAME_AS_IMPL(Equal)
OP_TOSTRING_IMPL(Equal,=,(
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))


constexpr Operator op_idx=[](){
	Operator op;

	op.associativity=RIGHT_ASSOCIATIVE;
	op.commutative=false;
	op.left_argt=Operator::ARRAY|Operator::TUPLE;
	op.right_argt=Operator::NUMBER;
	op.name="@";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Number* b_num = dynamic_cast<const Number*>(b.node.get());
		long idx = b_num->value;
		long size = a.node->subexprs.size();
		idx=((idx%size)+size)%size;
		return a.node->subexprs[idx];
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Index,op_idx)
SUBSTITUTE_IMPL(Index)
SAME_AS_IMPL(Index)
OP_TOSTRING_IMPL(Index,@,(
	REQ_PAREN(Call)
	REQ_PAREN(Exponent)
	REQ_PAREN(Div)
	REQ_PAREN(Mul)
	REQ_PAREN(Sub)
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
))

constexpr Operator op_call=[](){
	Operator op;

	op.associativity=ASSOCIATIVE;
	op.commutative=false;
	op.left_argt=Operator::FUNCTION;
	op.right_argt=Operator::SOMETHING;
	op.name="#";
	op.do_op=[](const Expr& a,const Expr& b)->Expr{
		const Function* a_func = dynamic_cast<const Function*>(a.node.get());
		if(b.type()=="Function"_id){

			const Function* b_func = dynamic_cast<const Function*>(b.node.get());
			Call* call=new Call();
			call->subexprs.push_back(b);
			if(b_func->inputs.size()==1){
				Variable* var = new Variable();
				var->name=b_func->inputs.front();
				call->subexprs.push_back(var);
			}
			else{
				Tuple* args = new Tuple();
				for(ID arg : b_func->inputs){
					Variable* var=new Variable();
					var->name=arg;
					args->subexprs.push_back(var);
				}
				call->subexprs.push_back(args);
			}
			Expr gx(call);

			map<ID,Expr> replacements;
			if(a_func->inputs.size()==1){
				replacements.emplace(a_func->inputs.front(),std::move(gx));
			}
			else{
				for(int n=0;n<a_func->inputs.size();n++){
					Index* idx = new Index();
					idx->subexprs.push_back(gx);
					idx->subexprs.push_back(n);
					replacements.emplace(a_func->inputs[n],idx);
				}
			}

			Function* ret=new Function();
			ret->inputs=b_func->inputs;
			Expr ex=a_func->subexprs.front();
			ex=ex.substitute(replacements);
			ex=ex.evaluate();
			ret->subexprs.push_back(std::move(ex));
			return ret;

		}
		else if(b.type()=="Tuple"_id){
			const Tuple* b_tup = dynamic_cast<const Tuple*>(b.node.get());
			if(a_func->inputs.size()==1){
				Expr ret=a_func->subexprs.front();
				ret=ret.substitute(map<ID,Expr>{{a_func->inputs.front(),b}});
				ret=ret.evaluate();
				return ret;
			}
			else if(a_func->inputs.size()==b_tup->subexprs.size()){
				map<ID,Expr> replace;
				for(int n=0;n<a_func->inputs.size();n++){
					Index* idx = new Index();
					idx->subexprs.push_back(b);
					idx->subexprs.push_back(n);

					replace.emplace(a_func->inputs[n],idx);
				}
				Expr ret=a_func->subexprs.front();
				ret=ret.substitute(replace);
				ret=ret.evaluate();
				return ret;
			}
			else{
				throw ExprError("bad arg count");
			}
		}
		else{
			if(a_func->inputs.size()==1){
				Expr ret=a_func->subexprs.front();
				ret=ret.substitute(map<ID,Expr>{{a_func->inputs.front(),b}});
				ret=ret.evaluate();
				return ret;
			}
			else{
				throw ExprError("bad arg count");
			}
		}
	};

	return op;
}();

NARY_OP_EXPR_EVAL(Call,op_call)
SUBSTITUTE_IMPL(Call)
SAME_AS_IMPL(Call)
OP_TOSTRING_IMPL(Call,#,(
	REQ_PAREN(Exponent)
	REQ_PAREN(Div)
	REQ_PAREN(Mul)
	REQ_PAREN(Sub)
	REQ_PAREN(Add)
	REQ_PAREN(Equal)
	REQ_PAREN(Less)
	REQ_PAREN(LessEqual)
	REQ_PAREN(Greater)
	REQ_PAREN(GreaterEqual)
	REQ_PAREN(Not)
	REQ_PAREN(And)
	REQ_PAREN(Or)
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
	return value;
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


Expr Function::evaluate() const{
	return clone();
}
Expr Function::substitute(const map<ID,Expr>& context) const {
	return clone();
}
bool Function::same_as(const Expr& b) const{
	if(b.type()!=type){
		return false;
	}
	const Function* b_func=dynamic_cast<const Function*>(b.node.get());
	if(inputs.size()!=b_func->inputs.size()){
		return false;
	}
	for(int n=0;n<inputs.size();n++){
		if(inputs[n]!=b_func->inputs[n]){
			return false;
		}
	}
	return subexprs.front().same_as(b_func->subexprs.front());
}
set<ID> Function::find_vars() const{
	return set<ID>();
}
string Function::to_string(bool force_parentheses) const{
	return "$func";
}
