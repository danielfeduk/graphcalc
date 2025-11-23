#include <stdint.h>
#include <stdbool.h>

enum builtin {
	BUILTIN_ADD,
	BUILTIN_SUB,
	BUILTIN_MUL,
	BUILTIN_DIV,
	BUILTIN_EXP,
	BUILTIN_SQRT,
	BUILTIN_ABS
};

enum exprtype {
	CONST,
	NAME,
	BUILTIN,
	FUNCALL,
//	LAMBDA
};

struct expr {
	enum exprtype type;
	union {
		// type.CONST
		double val;
		// type.NAME
		const char *name;
		// type.BUILTIN
		struct {
			enum builtin op;
			struct expr **args;
		};
		// type.FUNCALL
		struct {
			const char *fname;
			struct expr **fargs;
		};
	};
};

struct constdef {
	const char *name;
	struct expr bound;
};

#include <stdio.h>

#define FRAGMAXSZ	262144
#define LOGMAXSZ	1024

#define DEFWIDTH	800
#define DEFHEIGHT	600

void compilefn(const char*, const char*);
char *compilefrag(const char*);
struct expr parse_formula(const char *);
void printexpr(struct expr, int);