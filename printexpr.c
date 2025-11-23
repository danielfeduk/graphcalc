// unused but nice to have
#include <stdio.h>
#include "common.h"

static void
tab(int n)
{
	for(int i = 0; i < n ; ++ i) printf("\t");
}

void
printexpr(struct expr e, int nt)
{
	switch(e.type) {
	case CONST:
		tab(nt); printf("const\n");
		break;
	case NAME:
		tab(nt); printf("name\n");
		break;
	case BUILTIN:
		tab(nt); printf("builtin %d\n", e.op);
		for(int i = 0; e.args[i]; ++i) {
			printexpr(*(e.args[i]), nt + 1);
		}
		break;
	case FUNCALL:
		tab(nt); printf("function call: %s\n", e.fname);
		for(int i = 0; e.fargs[i]; ++i) {
			printexpr(*(e.fargs[i]), nt + 1);
		}
		break;
	default:
		tab(nt); printf("unknown expr type\n");
	}
}