/* implementation of sys.h for SDL */
#include "common.h"
#include "sys.h"

#include <stdio.h>
#include <sysexits.h>
#include <epoxy/gl.h>
#include <SDL2/SDL.h>

struct GS_private {
	SDL_Window *win;
	SDL_GLContext *gl;
};

const char
*GS_slurp(FILE *fp)
{
	int c;
	char *result;

	result = malloc(FRAGMAXSZ);

	for(int i = 0; (c = fgetc(fp)) != EOF; ++i) {
		if(i >= FRAGMAXSZ) break; // truncate or give up??
		result[i] = c;
	}

	return result;
}

struct GS_ctx
*GS_init(void)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
		exit(EX_UNAVAILABLE);
	}
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	int width = DEFWIDTH, height = DEFHEIGHT;
	SDL_Window* window = SDL_CreateWindow("graphcalc - math visual",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	
	if(!window) {
		fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EX_UNAVAILABLE);
	}
	
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if(!context) {
		fprintf(stderr, "OpenGL context creation error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}

	struct GS_private *data = malloc(sizeof(struct GS_private));
	*data = (struct GS_private){window, context};
	struct GS_ctx *ctx = malloc(sizeof(struct GS_ctx));
	*ctx = (struct GS_ctx){width, height, false, data};
	return ctx;
}

void
GS_quit(struct GS_ctx *ctx)
{
	SDL_GL_DeleteContext(ctx->data->gl);
	SDL_DestroyWindow(ctx->data->win);
	free(ctx->data);
	free(ctx);
	SDL_Quit();
}

void
GS_pollevents(struct GS_ctx *ctx)
{
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			ctx->end = true;
		}
		if(event.type == SDL_WINDOWEVENT
		    && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			ctx->width = event.window.data1;
			ctx->height = event.window.data2;
			glViewport(0, 0, ctx->width, ctx->height);
		}
	}
}

void
GS_swapframe(struct GS_ctx *ctx)
{
	SDL_GL_SwapWindow(ctx->data->win);
}

float
GS_gettime(struct GS_ctx *ctx)
{
	return SDL_GetTicks() / 1000.0f;
}

