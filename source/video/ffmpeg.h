#pragma once

#include <cmath>
#include <string>
#include <vector>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/error.h>	
#include <libavutil/file.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#undef av_err2str
#define av_err2str(errnum) \
	av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

namespace video {

void log_error(int ret);

void init_codec(AVCodecContext *ctx);

struct buffer_data {
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

class ffmpeg {
public:
	ffmpeg();
};


class rgb_color {
public:
	rgb_color();
	rgb_color(AVCodecContext *codec_ctx);
	~rgb_color();

	AVFrame *convert(AVFrame *in) const;

private:
	AVFrame *pFrameRGB;
	SwsContext *sws_ctx;

};


class audio_stream {
public:
	audio_stream();
	audio_stream(AVCodecContext *ctx);

	int get_audio(uint8_t *audio_buf, int buf_size);
	void add_audio(AVPacket *pkt);
	void add_audio(AVFrame *frame);

private:
	AVCodecContext *codec_ctx;
	AVFrame *audio_frame;
	int read_audio;
	std::vector<char> buffer;

	int sample;

};

class video_stream {
public:
	video_stream(AVCodecContext *ctx);

	AVFrame *get_image();
	void add_image(AVPacket *pkt);

	std::pair<int, int> size() const;

private:
	AVCodecContext *codec_ctx;

	AVFrame *out_frame;

	rgb_color rgb;

};



/**
 * a set of streams
 */
class av_stream {
public:
	av_stream();
	av_stream(AVFormatContext *fc);
	~av_stream();
	
	audio_stream get_audio_stream() const;
	video_stream get_video_stream() const;

	void read(audio_stream &as);

private:
	AVFormatContext *fmt_ctx;

};


class context {
public:
	context(const std::string &filename);
	context();
	~context();

	void stream_info() const;

	void set_avio(AVIOContext *avio_ctx);

	av_stream get_stream();

private:
	AVFormatContext *fmt_ctx;

};


class avio {
public:
	avio(const buffer_data &bdin);
	~avio();

	AVIOContext *context() const;

private:
	buffer_data bd;
	AVIOContext *avio_ctx;
	uint8_t *avio_ctx_buffer;
	size_t avio_ctx_buffer_size;

};


class file_map {
public:
	file_map(const std::string &filename);
	~file_map();

	uint8_t *ptr() const;
	size_t size() const;
	buffer_data data() const;

private:
	const std::string name;
	uint8_t *buffer;
    size_t buffer_size;

};


}
