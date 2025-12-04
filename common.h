#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

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

#define FRAGMAXSZ	262144
#define LOGMAXSZ	1024

#define DEFWIDTH	800
#define DEFHEIGHT	600

#define NUMBER		" float "

void compilefn(const char*, const char*);
char *codegenfn(const char **, const char *);
struct expr parse_formula(const char *);
void printexpr(struct expr, int);

static inline struct expr
**nargs(unsigned short n, ...)
{
	va_list ap; 
	struct expr **args = malloc((n+1) * sizeof(struct expr *));
	va_start(ap, n);
	for(int i = 0; i < n; ++i) {
		args[i] = malloc(sizeof(struct expr));
		*(args[i]) = va_arg(ap, struct expr);
	}
	args[n] = 0x0;
	va_end(ap);
	return args;
}