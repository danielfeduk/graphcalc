
/******************************************************************************
 *                                                                            *
 * graphcalc: math visualisation                                              *
 * Copyright (C) 2025  Daniel Feduk                                           *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify       *
 * it under the terms of the GNU General Public License as published by       *
 * the Free Software Foundation, either version 3 of the License, or          *
 * (at your option) any later version.                                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.     *
 *                                                                            *
 ******************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

typedef uint32_t Temporary;

static Temporary compile(struct expr, char **, long *);

/* look into bsd sbuf? */
static inline void
memfmt(char **bufloc, long *n, const char *fmt, ...)
{
	va_list ap;
	size_t advance = 0;
	va_start(ap, fmt);
	assert(*n > 0);
	assert(fmt);
	assert(bufloc);
	assert(*bufloc);
	advance = vsnprintf(*bufloc, *n, fmt, ap);
	va_end(ap);
	*bufloc += advance;
	*n -= advance;
	assert(*n > 0);
}

#define PRNT(...) memfmt(bufloc, n, __VA_ARGS__)

static Temporary
newtmp(void)
{
	static Temporary t = 1;
	return t++;
}

#define SET	"\t" NUMBER " tmp%d = "
#define END	";\n"
#define T	"tmp%d"

static Temporary
compbi(struct expr expr, char **bufloc, long *n)
{
	int nargs;
	Temporary args[16];
	for(nargs = 0; expr.args[nargs]; ++nargs) {
		args[nargs] = compile(*(expr.args[nargs]), bufloc, n);
		free(expr.args[nargs]);
	}
	
	Temporary t = newtmp();

	switch(expr.op) {
	case BUILTIN_ADD:
		PRNT(SET T "+" T END, t, args[0], args[1]);
		break;
	case BUILTIN_SUB:
		PRNT(SET T "-" T END, t, args[0], args[1]);
		break;
	case BUILTIN_MUL:
		PRNT(SET T "*" T END, t, args[0], args[1]);
		break;
	case BUILTIN_DIV:
		PRNT(SET T "/" T END, t, args[0], args[1]);
		break;
	case BUILTIN_EXP:
		PRNT(SET "pow(" T ", " T ")" END, t, args[0], args[1]);
		break;
	case BUILTIN_SQRT:
		PRNT(SET "sqrt(" T ")" END, t, args[0]);
		break;
	case BUILTIN_ABS:
		PRNT(SET "abs(" T ")" END, t, args[0]);
		break;
	}
	return t;
}

static Temporary
compile(struct expr e, char **bufloc, long *n)
{
	switch(e.type) {
	case CONST: {
		Temporary t = newtmp();
		PRNT(SET "%f" END, t, e.val);
		return t;
		}
		break;
	case NAME: {
		Temporary t = newtmp();
		PRNT(SET "%s" END, t, e.name);
		return t;
		}
		break;
	case BUILTIN:
		return compbi(e, bufloc, n);
	case FUNCALL: {
		Temporary t = newtmp();
		int nargs;
		Temporary args[16];
		for(nargs = 0; e.fargs[nargs]; ++nargs) {
			args[nargs] = compile(*(e.fargs[nargs]), bufloc, n);
			free(e.fargs[nargs]);
		}
		assert(nargs > 0);
		PRNT(SET "%s(" T, t, e.fname, args[0]);
		for(int i = 1; i < nargs; ++i) {
			PRNT(", " T, args[i]);
		}
		PRNT(")" END);
		return t;
		}
		break;
	}
}

char
*codegenfn(const char **fsig, const char *formula)
{
	long m = FRAGMAXSZ;
	long *n = &m;
	char *buf = malloc(m);
	char *orig = buf;
	char **bufloc = &buf;
	struct expr e = parse_formula(formula);
//	printexpr(e, 0);
	PRNT(NUMBER " %s(", fsig[0]);
	if(fsig[1]) {
		PRNT(NUMBER " %s", fsig[1]);
		for(int i = 2; fsig[i]; ++i) {
			PRNT(", float %s", fsig[i]);
		}
	}
	PRNT(")\n {\n");
	Temporary t = compile(e, bufloc, n);
	PRNT("\treturn tmp%d;\n}\n\n", t);
	*buf = '\0';
	return orig;
}
