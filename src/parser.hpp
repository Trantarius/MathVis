#pragma once
#include <string>
#include "expression.hpp"
using std::string;

struct ParseFail{
	string reason;
	ParseFail(){}
	ParseFail(string r):reason(r){}
};

Expr parse(string str);

