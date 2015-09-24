#include <iostream>

#include <epoxy/gl.h>

#include "texture.h"

namespace video {

texture::texture() {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	video_src = open_device(device("/dev/video0"));
}

void texture::update() {
    image<short> result = read_frame(video_src);
	for (int x = 0; x < 640; ++x) {
		for (int y = 0; y < 480; ++y) {
			int index = 3 * (y * 640 + x);
			short value = result.get(x, y);
			pixels[index] = value >> 4; // (value << 0 | 0xf) / 16.0;
			pixels[index + 1] = 0.0; //(value >> 4 | 0xf) / 16.0;
			pixels[index + 2] = 0.0; // (value >> 8 | 0xf) / 16.0;
		}
	}

	// Black/white checkerboard
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_FLOAT, pixels);
}

}
