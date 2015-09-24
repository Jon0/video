#pragma once

#include "input.h"

namespace video {

class texture {
public:
	texture();

	void update();

private:
	float pixels[640 * 480 * 3];
	capture_source video_src;
};

}
