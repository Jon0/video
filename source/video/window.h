#pragma once

#include <memory>

#include <SDL2/SDL.h>

#include "shader.h"
#include "texture.h"

namespace video {

void sdl_init();

class renderer {
public:
	renderer();

	void draw(AVFrame *frame);

	std::pair<int, int> size() const;

private:
	std::string src_path;
	shader vert_shader, frag_shader;
	GLuint program;
	texture tx;
};

class window {
public:
	window();
	~window();

	void update();

private:
	SDL_Window *sdl_window;
	SDL_GLContext gl_context;
	std::unique_ptr<renderer> rd;

};

}
