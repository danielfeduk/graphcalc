#include <epoxy/gl.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>

#include "common.h"

// fullscreen quad
const char *vertex_shader_src =
	"#version 330 core\n"
	"layout (location = 0) in vec2 aPos;\n"
	"void main() {\n"
	"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
	"}\n";

// fullscreen quad vertices
float quad_vertices[] = {
	-1.0f, -1.0f,	// bottom left
	1.0f, -1.0f,	// bottom right
	-1.0f, 1.0f,	// top left
	1.0f, -1.0f,	// bottom right
	1.0f, 1.0f,	// top right
	-1.0f, 1.0f	// top left
};

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

static GLuint
mkshaderprog(char *formula)
{
	char *frag_src = compilefrag(formula);
	//printf("%s", frag_src);

	GLuint vert = compileshader(vertex_shader_src, GL_VERTEX_SHADER);
	GLuint frag = compileshader(frag_src, GL_FRAGMENT_SHADER);
	
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
	if(argc != 2) {
		fprintf(stderr, "Usage: %s [formula]\n", argv[0]);
		exit(EX_USAGE);
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL init error: %s\n", SDL_GetError());
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
	
	if (!window) {
		printf("Window creation error: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EX_UNAVAILABLE);
	}
	
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		printf("OpenGL context creation error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		exit(1);
	}
	
	GLuint shader_program = mkshaderprog(argv[1]);
	
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	GLint time_loc = glGetUniformLocation(shader_program, "u_time");
	GLint sclx_loc = glGetUniformLocation(shader_program, "u_sclx");
	GLint scly_loc = glGetUniformLocation(shader_program, "u_scly");
	GLint szx_loc = glGetUniformLocation(shader_program, "u_szx");
	GLint szy_loc = glGetUniformLocation(shader_program, "u_szy");

	// Main loop
	int running = 1;
	SDL_Event event;
	
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				width = event.window.data1;
				height = event.window.data2;
				glViewport(0, 0, width, height);
			}
		}
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shader_program);
		float time = SDL_GetTicks() / 1000.0f;
		glUniform1f(time_loc, time);
		glUniform1f(sclx_loc, 1.0f);
		glUniform1f(scly_loc, 1.0f);
		glUniform1f(szx_loc, width);
		glUniform1f(szy_loc, height);
		
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		SDL_GL_SwapWindow(window);
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shader_program);
	
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}