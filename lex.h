#ifndef LEX_H
#define LEX_H

#include "parse.gen.h"

#define TOKTYPE int

struct token {
	TOKTYPE type;
	long long lineno;
	char *content;
};

struct token yylex(void);
void yyinit(const char* source);

#endif
