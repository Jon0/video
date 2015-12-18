#include <iostream>

#include "texture.h"

namespace video {

texture::texture() {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

texture::~texture() {
	glDeleteTextures(1, &tex);
}


void mvpix() {
	//    image<short> result = read_frame(video_src);
	// for (int x = 0; x < 640; ++x) {
	// 	for (int y = 0; y < 480; ++y) {
	// 		int index = 3 * (y * 640 + x);
	// 		short value = result.get(x, y);
	// 		float white = static_cast<float>(value & 0xff) / 256.0;
	// 		float a = static_cast<float>((value & 0xff00) >> 8) / 256.0;
	// 		pixels[index + 0] = white; //(value & 0x7) / 8.0;
	// 		pixels[index + 1] = white * a; //((value >> 3) & 0x7) / 8.0;
	// 		pixels[index + 2] = white; //((value >> 6) & 0x3) / 4.0;
	// 	}
	// }
}

void texture::update(AVFrame *frame) {
	//std::cout << frame->width << " x " << frame->height << "\n";
	//std::cout << av_frame_get_colorspace(frame) << "\n";
	//std::cout << AV_PIX_FMT_YUV410P << "\n";
	//std::cout << av_frame_get_color_range(frame) << "\n";
	if (frame) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame->data[0]);
	}
}


}
