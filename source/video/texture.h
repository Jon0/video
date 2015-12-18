#pragma once

#include <epoxy/gl.h>

#include "../os/v4l.h"
#include "ffmpeg.h"

namespace video {

class texture {
public:
	texture();
	~texture();

	void update(AVFrame *frame);

private:
	GLuint tex;
	char pixels[1000 * 768 * 3];
};

}
