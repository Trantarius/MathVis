#pragma once
#include "expression.hpp"

struct ParseFail{
	string reason;
	ParseFail(){}
	ParseFail(string r):reason(r){}
};

Expr parse(string str);

