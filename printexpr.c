
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