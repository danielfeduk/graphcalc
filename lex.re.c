#include "lex.h"
#include "parse.gen.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>

#define YYCTYPE char

/* internal: dont use this in yylex */
static const char *YYCURSOR, *YYPREV, *YYMARKER;

int yylineno = 1;

/*!stags:re2c format = "char *@@;"; */

void
re2c_init(void)
{
	YYPREV = YYCURSOR;
}

void
yyerror(const char *err)
{
	printf("Error: line %d: %s\n", yylineno, err);
	exit(1);
}

/* this is bullshit */
#define RET(x) return x;

TOKTYPE
re2c_scan(void)
{
	YYPREV = YYCURSOR;
	/*!re2c
		re2c:yyfill:enable = 0;

		IDEN = [a-zA-Z_][a-zA-Z0-9_]?+ ;
		DEC = [0-9]+ ;
		NUM = (DEC?"."(DEC)?)|DEC ;
		
		NUM	{ RET(TOK_CONST)	}
		"="	{ RET(TOK_EQUALS)	}
		"("	{ RET(TOK_OPEN) 	}
		")"	{ RET(TOK_CLOSE)	}
		","	{ RET(TOK_COMMA)	}

		"+"	{ RET(TOK_PLUS)		}
		"-"	{ RET(TOK_MINUS)	}
		"*"	{ RET(TOK_TIMES)	}
		"/"	{ RET(TOK_SLASH)	}
		"^"	{ RET(TOK_POWER)	}
		"|"	{ RET(TOK_BAR)		}
		"sqrt"	{ RET(TOK_KW_SQRT)	}
		IDEN	{ RET(TOK_IDENTIFIER)	}

		[ \r\t]		{ RET(TOK_WHITESP)		}
		'\n'		{ ++yylineno; RET(TOK_WHITESP)	}
		*		{ RET(TOK_UNDEFINED)		}
		[\x00]		{ RET(TOK_END)			}
	*/
	RET(TOK_UNDEFINED);
}

static char
*genyytext()
{
	assert(YYCURSOR >= YYPREV);
	assert(YYCURSOR != NULL);
	assert(YYPREV != NULL);
	size_t text_size = YYCURSOR - YYPREV;
	char *yytext = malloc(text_size + 1);
	strncpy(yytext, YYPREV, text_size);
	yytext[text_size] = 0;
	return yytext;
}

struct token yylex(void)
{
	TOKTYPE type;
	struct token next_tok;
	do {
		type = re2c_scan();
	} while(type == TOK_WHITESP); /* go through all whitespace tokens, ignoring them. */
	next_tok.type = type;
	next_tok.lineno = yylineno;
	next_tok.content = genyytext();

	return next_tok;
}

void
yyinit(const char *src)
{
	YYCURSOR = src;
	re2c_init();
}
