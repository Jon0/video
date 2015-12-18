#pragma once

#include "ffmpeg.h"

namespace video {

void video_encode_example(const char *filename, AVCodecID codec_id);


class encoder {
public:
	encoder(AVCodecID codec_id);
	~encoder();

	std::string get_frame();

private:
	AVCodec *codec;
	AVCodecContext *c = NULL;
	int i, ret, x, y, got_output;
	FILE *f;
	AVFrame *frame;
	AVPacket pkt;

};


class out_stream {
public:
	out_stream(const std::string &path);
};


}
