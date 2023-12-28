#include "parser.hpp"


struct Token{
#define MODE(NAME,VALUE) inline static const ID NAME = #VALUE##_id
	MODE(PLUS,+);
	MODE(MINUS,-);
	MODE(TIMES,*);
	MODE(DIVIDE,/);
	MODE(POWER,^);
	MODE(PARENTHESES,());
	MODE(SQUARE_BRACKET,[]);
	MODE(CURLY_BRACKET,{});
	MODE(NOT,~);
	MODE(EQUAL,=);
	MODE(AND,&);
	MODE(OR,|);
	MODE(LESS,<);
	MODE(LESS_EQUAL,<=);
	MODE(GREATER,>);
	MODE(GREATER_EQUAL,>=);
	MODE(INDEX,@);
	MODE(CALL,#);
	inline static const ID COMMA = ","_id;
	MODE(IDENTIFIER,id);
	MODE(NUMBER,num);
#undef MODE

	//a type from above
	ID type;

	//only if type == id
	ID id;

	//only if type is PARENTHESES, SQUARE_BRACKET, or CURLY_BRACKET
	list<Token> subtokens;

	//only if type is NUMBER
	number_t num;
};

//may throw Parser::ParseFail if syntax is really bad
list<Token> tokenize(string);
using TokenIterator = list<Token>::const_iterator;
using CharIterator = string::const_iterator;


void seek_char(CharIterator& iter, const CharIterator& end, char target){
	while(iter!=end){
		if(*iter==target){
			return;
		}
		if(*iter=='('){
			seek_char(iter,end,')');
		}
		else if(*iter=='['){
			seek_char(iter,end,']');
		}
		else if(*iter=='{'){
			seek_char(iter,end,'}');
		}
		iter++;
	}
}

bool is_id_char(char c){
	return c>='a'&&c<='z' || c>='A'&&c<='Z' || c=='_';
}
bool is_num_char(char c){
	return c>='0'&&c<='9' || c=='.';
}

list<Token> tokenize(string str){
	list<Token> ret;
	bool is_making_id=false;
	bool is_making_number=false;
	string accum;
	CharIterator iter=str.begin();
	while(iter!=str.end()){

		if(is_making_id){
			if(is_id_char(*iter)){
				accum.push_back(*iter);
				iter++;
			}
			else{
				Token t;
				t.type=Token::IDENTIFIER;
				t.id=ID(accum);
				accum="";
				is_making_id=false;
				ret.push_back(t);
			}
			continue;
		}

		if(is_making_number){
			if(is_num_char(*iter)){
				accum.push_back(*iter);
				iter++;
			}
			else{
				Token t;
				t.type=Token::NUMBER;
				t.num=std::stold(accum);
				accum="";
				is_making_number=false;
				ret.push_back(t);
			}
			continue;
		}

		if(*iter=='('){
			auto icpy = iter;
			seek_char(icpy,str.end(),')');
			if(icpy==str.end()){
				throw ParseFail("unclosed parentheses");
			}
			iter++;
			if(iter==icpy){
				throw ParseFail("nothing in parentheses");
			}
			string sub (iter,icpy);
			Token t;
			t.type=Token::PARENTHESES;
			t.subtokens=tokenize(sub);
			ret.push_back(t);
			iter=icpy;
			iter++;
			continue;
		}

		if(*iter=='['){
			auto icpy = iter;
			seek_char(icpy,str.end(),']');
			if(icpy==str.end()){
				throw ParseFail("unclosed square brackets");
			}
			iter++;
			if(iter==icpy){
				throw ParseFail("nothing in square brackets");
			}
			string sub (iter,icpy);
			Token t;
			t.type=Token::SQUARE_BRACKET;
			t.subtokens=tokenize(sub);
			ret.push_back(t);
			iter=icpy;
			iter++;
			continue;
		}

		if(*iter=='{'){
			auto icpy = iter;
			seek_char(icpy,str.end(),'}');
			if(icpy==str.end()){
				throw ParseFail("unclosed curly brackets");
			}
			iter++;
			if(iter==icpy){
				throw ParseFail("nothing in curly brackets");
			}
			string sub (iter,icpy);
			Token t;
			t.type=Token::CURLY_BRACKET;
			t.subtokens=tokenize(sub);
			ret.push_back(t);
			iter=icpy;
			iter++;
			continue;
		}

#define CHAR_TOKEN(CHAR,TOKTYPE)     \
		if(*iter==CHAR){                 \
			Token t;                       \
			t.type = Token::TOKTYPE;       \
			ret.push_back(t);              \
			iter++;                        \
			continue;                      \
		}

		CHAR_TOKEN('@',INDEX)
		CHAR_TOKEN('#',CALL)
		CHAR_TOKEN('^',POWER)
		CHAR_TOKEN('/',DIVIDE)
		CHAR_TOKEN('*',TIMES)
		CHAR_TOKEN('-',MINUS)
		CHAR_TOKEN('+',PLUS)
		CHAR_TOKEN('=',EQUAL)
		CHAR_TOKEN('~',NOT)
		CHAR_TOKEN('&',AND)
		CHAR_TOKEN('|',OR)
		CHAR_TOKEN(',',COMMA)
#undef CHAR_TOKEN

		if(*iter=='<'){
			iter++;
			if(iter==str.end() || *iter!='='){
				Token t;
				t.type=Token::LESS;
				ret.push_back(t);
				continue;
			}
			Token t;
			t.type=Token::LESS_EQUAL;
			ret.push_back(t);
			iter++;
			continue;
		}

		if(*iter=='>'){
			iter++;
			if(iter==str.end() || *iter!='='){
				Token t;
				t.type=Token::GREATER;
				ret.push_back(t);
				continue;
			}
			Token t;
			t.type=Token::GREATER_EQUAL;
			ret.push_back(t);
			iter++;
			continue;
		}

		//ignore all other characters
		iter++;
		continue;
	}

	if(is_making_id){
		Token t;
		t.type=Token::IDENTIFIER;
		t.id=ID(accum);
		accum="";
		is_making_id=false;
		ret.push_back(t);
	}

	if(is_making_number){
		Token t;
		t.type=Token::NUMBER;
		t.num=std::stold(accum);
		accum="";
		is_making_number=false;
		ret.push_back(t);
	}

	return ret;
}

//gets a token of type 'what' at iter, assigns it to where (if where isn't NULL)
//if fails, throws ParseFail
//increments iter
void expect(TokenIterator& iter,ID what,Token* where){
	if(iter->type==what){
		if(where){
			*where=*iter;
		}
		iter++;
	}
	else{
		throw ParseFail("expected "+string(what));
	}
}

//increments iter until just past a token of type 'what'
//throws ParseFail if not found
//assigns content between [iter , found) to middle, if not NULL
void seek(TokenIterator& iter, const TokenIterator& end, ID what,list<Token>* middle, Token* found){
	if(iter==end){
		throw ParseFail("expected "+string(what));
	}
	while(iter!=end){
		if(iter->type==what){
			if(found){
				*found=*iter;
			}
			break;
		}else{
			if(middle){
				middle->push_back(*iter);
			}
			iter++;
		}
	}
	throw ParseFail("expected "+string(what));
}

//increments iter until just past a token of type 'what', or to end
//assigns content between iter and found (or remainder) (exclusive) to middle, if not NULL
void seek_opt(TokenIterator& iter, const TokenIterator& end, ID what,list<Token>* middle, Token* found) noexcept{
	while(iter!=end){
		if(iter->type==what){
			if(found){
				*found=*iter;
			}
			break;
		}else{
			if(middle){
				middle->push_back(*iter);
			}
			iter++;
		}
	}
}

template<typename NODE>
struct _expr_type;

#define ETYPE(AAA,BBB) template<> struct _expr_type<BBB>{ static constexpr const ID& op = Token::AAA; };

ETYPE(INDEX,Index)
ETYPE(CALL,Call)
ETYPE(POWER,Exponent)
ETYPE(DIVIDE,Div)
ETYPE(TIMES,Mul)
ETYPE(MINUS,Sub)
ETYPE(PLUS,Add)
ETYPE(EQUAL,Equal)
ETYPE(LESS,Less)
ETYPE(LESS_EQUAL,LessEqual)
ETYPE(GREATER,Greater)
ETYPE(GREATER_EQUAL,GreaterEqual)
ETYPE(NOT,Not)
ETYPE(AND,And)
ETYPE(OR,Or)

#undef ETYPE

Expr parse_non_op(const list<Token>&);

template<typename...Ts>
struct parse_op;

template<typename NODE,typename...OTHERS>
struct parse_op<NODE,OTHERS...>{
	Expr operator ()(const list<Token>& tokens) const {
		ID op = _expr_type<NODE>::op;
		TokenIterator iter=tokens.begin();
		deque<Expr> subexprs;
		list<Token> sub;
		while(iter!=tokens.end()){
			seek_opt(iter,tokens.end(),op,&sub,NULL);
			if(sub.empty()){
				subexprs.push_back(Expr());
			}else{
				subexprs.push_back(parse_op<OTHERS...>()(sub));
				sub.clear();
			}
			if(iter!=tokens.end()){
				iter++;
			}
		}

		if(subexprs.size()==1){
			return std::move(subexprs.front());
		}
		NODE* ex = new NODE();
		ex->subexprs=std::move(subexprs);
		return ex;
	}
};

template<> struct parse_op<>{
	Expr operator()(const list<Token>& tokens) const{
		return parse_non_op(tokens);
	}
};

Expr parse_tokens(const list<Token>& tokens){
	return parse_op<Equal,Add,Sub,Mul,Div,Exponent,Call,Index>()(tokens);
}

deque<Expr> parse_list(const list<Token>& tokens){
	TokenIterator iter=tokens.begin();
	deque<Expr> subexprs;
	list<Token> sub;
	while(iter!=tokens.end()){
		seek_opt(iter,tokens.end(),Token::COMMA,&sub,NULL);
		if(sub.empty()){
			subexprs.push_back(Expr());
		}else{
			subexprs.push_back(parse_tokens(sub));
			sub.clear();
		}
		if(iter!=tokens.end()){
			iter++;
		}
	}
	return subexprs;
}

Expr parse_one(const Token& token){
	if(token.type==Token::PARENTHESES){
		deque<Expr> sub = parse_list(token.subtokens);
		if(sub.empty()){
			throw ParseFail("empty ()");
		}
		if(sub.size()==1){
			return std::move(sub.front());
		}
		Tuple* tup = new Tuple();
		tup->subexprs=std::move(sub);
		return tup;
	}

	else if(token.type==Token::SQUARE_BRACKET){
		deque<Expr> sub = parse_list(token.subtokens);
		if(sub.empty()){
			throw ParseFail("empty []");
		}
		Array* tup = new Array();
		tup->subexprs=std::move(sub);
		return tup;
	}

	else if(token.type==Token::CURLY_BRACKET){
		throw ParseFail("{} not supported yet");
	}

	else if(token.type==Token::IDENTIFIER){
		Variable* var = new Variable();
		var->name=token.id;
		return var;
	}

	else if(token.type==Token::NUMBER){
		Number* num = new Number();
		num->value = token.num;
	}

	throw ParseFail("syntax error");
}

Expr parse_non_op(const list<Token>& tokens){
	if(tokens.empty()){
		throw ParseFail("expected expression");
	}
	if(tokens.size()==1){
		return parse_one(tokens.front());
	}
	throw ParseFail("invalid adjacent non-operator tokens");
}

Expr parse(string str){
	return parse_tokens(tokenize(str));
}
