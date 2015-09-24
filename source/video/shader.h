#pragma once

#include <string>

#include <epoxy/gl.h>

namespace video {

/**
 * draws input verticies
 * each shader uses a specific vertex format
 */
class shader {
public:
	std::string name;
	GLuint handle;
	GLenum type;

	shader(std::string filename, GLenum shader_type);
	~shader();

};

}
