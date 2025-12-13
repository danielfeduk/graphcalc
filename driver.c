
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
#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include <err.h>
#include "common.h"
#include "sys.h"

// fullscreen quad
const char *vertex_shader_src =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"void main() {\n"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
	"}\n";

const char *fragment_main_src = 
	"#version 330 core\n"
	"uniform float u_time, u_szx, u_szy, u_sclx, u_scly;\n"
	"out vec4 fragColor;\n"
	"\n%s\n\n"
	"void main() {\n"
		"vec2 pos = gl_FragCoord.xy;\n"
		"float x = u_sclx * (pos.x / u_szx) - (u_sclx/2);\n"
		"float y = u_scly * (pos.y / u_szy) - (u_scly/2);\n"
		"float n = f(x, y, u_time);\n"
		"float npos = max(n, 0.0);\n"
		"float nneg = -min(n, 0.0);\n"
		"vec3 color = vec3(nneg, 0, npos);\n"
		"if(nneg > 1.0) color = vec3(1.0, 1.0, 0.0);\n"
		"if(npos > 1.0) color = vec3(0.0, 1.0, 1.0);\n"
		"fragColor = vec4(color, 1.0);\n"
	"}\n"
	;

// fullscreen quad vertices
float quad_vertices[] = {
	-1.0f, -1.0f,	// bottom left
	1.0f, -1.0f,	// bottom right
	-1.0f, 1.0f,	// top left
	1.0f, -1.0f,	// bottom right
	1.0f, 1.0f,	// top right
	-1.0f, 1.0f	// top left
};

static void
usage(char *progname)
{
	fprintf(stderr, "Usage: %s [-f file] [formula]\n", progname);
	exit(EX_USAGE);
}

static GLuint
compileshader(const char *src, GLenum shadertype)
{
	GLuint shader = glCreateShader(shadertype);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		printf("Shader compile error: %s\n", info_log);
		exit(1);
	}
	
	return shader;
}

static const char fragfmt[] = {
	"#version 330 core\n"
	"uniform " NUMBER " u_time;\n"
	"uniform vec2 u_sz;"
	"uniform mat2 u_win;"
	"out vec4 frag_color;\n"
	"\n%s\n\n"
	"void main() {\n"
		"vec2 pos = gl_FragCoord.xy;\n"
		NUMBER " x = u_win[0].x + (pos.x / u_sz.x) * (u_win[1].x - u_win[0].x);\n"
		NUMBER "y = u_win[0].y + (pos.y / u_sz.y) * (u_win[1].y - u_win[0].y);\n"
		NUMBER " n = f(x, y, u_time);\n"
		NUMBER " npos = max(n, 0.0);\n"
		NUMBER " nneg = -min(n, 0.0);\n"
		"vec3 color = vec3(nneg, 0, npos);\n"
		"if(nneg > 1.0) color = vec3(1.0, 1.0, 0.0);\n"
		"if(npos > 1.0) color = vec3(0.0, 1.0, 1.0);\n"
		"frag_color = vec4(color, 1.0);\n"
	"}\n"
};

static char
*composefrag(const char *formula)
{
	assert(strlen(fragfmt) < FRAGMAXSZ);
	char *frag;
	char *fun;
	fun = codegenfn((const char* []){"f", "x", "y","z", NULL}, formula);
	asprintf(&frag, fragfmt, fun);
	free(fun);
	return frag;
}

static GLuint
mkshaderprog(const char *formula)
{
	char *frag_src = composefrag(formula);
	//printf("%s", frag_src);

	GLuint vert = compileshader(vertex_shader_src, GL_VERTEX_SHADER);
	GLuint frag = compileshader(frag_src, GL_FRAGMENT_SHADER);
	free(frag_src);
	
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);
	
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char info_log[LOGMAXSZ];
		glGetProgramInfoLog(program, LOGMAXSZ, NULL, info_log);
		printf("Shader link error: %s\n", info_log);
		exit(1);
	}
	
	glDeleteShader(vert);
	glDeleteShader(frag);
	return program;
}

int
main(int argc, char *argv[])
{
	int opt;
	float win[4] = {-1.0, 1.0, -1.0, 1.0};
	const char *formula = NULL;
	
	while((opt = getopt(argc, argv, "f:x:X:y:Y:")) != -1) {
		switch(opt) {
		case 'f': {
			const char *filename = optarg;
			FILE *fp = fopen(filename, "r");
			if(!fp) err(EX_NOINPUT, "couldn't open formula file");
			formula = GS_slurp(fp);
		}
			break;

		// todo add getopt long, validate input is a number.
		case 'x':
			win[0] = strtof(optarg, NULL);
			break;
		case 'X':
			win[1] = strtof(optarg, NULL);
			break;
		case 'y':
			win[2] = strtof(optarg, NULL);
			break;
		case 'Y':
			win[3] = strtof(optarg, NULL);
			break;

		default: usage(argv[0]);

		}
	}
	
	if(!formula) {
		if(optind >= argc) usage(argv[0]);
		if(!strncmp(argv[optind], "-", 2)) {
			formula = GS_slurp(stdin);
		} else {
			formula = argv[optind];
		}
	}

	struct GS_ctx *ctx = GS_init();
	GLuint shader_program = mkshaderprog(formula);
	
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	GLint time_loc = glGetUniformLocation(shader_program, "u_time");
	GLint sz_loc = glGetUniformLocation(shader_program, "u_sz");
	GLint win_loc = glGetUniformLocation(shader_program, "u_win");

	while(!ctx->end) {
		GS_pollevents(ctx);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shader_program);
		float time = GS_gettime(ctx);
		glUniform1f(time_loc, time);
		glUniformMatrix2fv(win_loc, 1, GL_TRUE, win);
		glUniform2f(sz_loc, ctx->width, ctx->height);	
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		GS_swapframe(ctx);
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shader_program);
	
	GS_quit(ctx);
	return 0;
}