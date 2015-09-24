#include <fstream>
#include <iostream>
#include <sstream>

#include <epoxy/gl.h>

#include "window.h"

namespace video {

void sdl_init() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
}

std::string get_src_path() {
	std::ifstream text("src_path");
	std::stringstream ss;
	ss << text.rdbuf();
	std::string src_path = ss.str();
	return src_path.substr(0, src_path.length() - 1);
}

renderer::renderer()
	:
	src_path(get_src_path()),
	vert_shader(src_path + "/source/shaders/simple.vert", GL_VERTEX_SHADER),
	frag_shader(src_path + "/source/shaders/simple.frag", GL_FRAGMENT_SHADER),
	tx() {

	program = glCreateProgram();
	glAttachShader(program, vert_shader.handle);
	glAttachShader(program, frag_shader.handle);
	glLinkProgram(program);
	glUseProgram(program);

	float vertices[] = {
    -0.5f,  0.5f, 0.0f, 0.0f, // Top-left
     0.5f,  0.5f, 1.0f, 0.0f, // Top-right
     0.5f, -0.5f, 1.0f, 1.0f, // Bottom-right
    -0.5f, -0.5f, 0.0f, 1.0f  // Bottom-left
};

	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate 1 buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLint posAttrib = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);

	GLint texAttrib = glGetAttribLocation(program, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

	GLuint elements[] = {
    	0, 1, 2,
    	2, 3, 0
	};

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
}

void renderer::draw() {
	tx.update();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

window::window() {
	sdl_init();
	sdl_window = SDL_CreateWindow(
		"window",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800,
		600,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	gl_context = SDL_GL_CreateContext(sdl_window);
	std::cout << "OpenGL version " << epoxy_gl_version() << "\n";
	rd = std::make_unique<renderer>();
}

window::~window() {
	rd = nullptr;
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(sdl_window);
	SDL_Quit();
}

void window::update() {
	glClearColor(0.0, 0.2, 0.3, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	rd->draw();
	SDL_GL_SwapWindow(sdl_window);
}

}
