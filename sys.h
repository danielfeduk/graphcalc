/* non-gl system abstractions */

#include <stdio.h>

struct GS_private;

struct GS_ctx {
	int width;
	int height;
	bool end;
	struct GS_private *data;
};

const char *GS_slurp(FILE *);
struct GS_ctx *GS_init(void);
void GS_quit(struct GS_ctx *);
void GS_pollevents(struct GS_ctx *);
void GS_swapframe(struct GS_ctx *);
float GS_gettime(struct GS_ctx *);
