#include <cfloat>
#include <iostream>

#include "ffmpeg.h"

namespace video {


void log_error(int ret) {
	std::cout << "ret: " << ret << " (" << av_err2str(ret) << ")\n";
}


void init_codec(AVCodecContext *ctx) {

	// Find the decoder for the video stream
	AVCodec *codec = avcodec_find_decoder(ctx->codec_id);
	if (codec == nullptr) {
		std::cout << "Unsupported codec\n";
		return;
	}

	AVDictionary *optionsDict = nullptr;
	int ret = avcodec_open2(ctx, codec, &optionsDict);
	log_error(ret);
}


static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);
    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);
    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr  += buf_size;
    bd->size -= buf_size;
    return buf_size;
}


ffmpeg::ffmpeg() {
	av_register_all();
}


rgb_color::rgb_color()
	:
	pFrameRGB(nullptr),
	sws_ctx(nullptr) {}


rgb_color::rgb_color(AVCodecContext *codec_ctx) {

	// Allocate an AVFrame structure
	pFrameRGB = av_frame_alloc();
	if(pFrameRGB == nullptr) {
		std::cout << "allocation failed\n";
		return;
	}

	// Determine required buffer size and allocate buffer
	int numBytes = avpicture_get_size(PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height);
	uint8_t *buffer = reinterpret_cast<uint8_t *>(av_malloc(numBytes*sizeof(uint8_t)));
	sws_ctx = sws_getContext(
		codec_ctx->width,
		codec_ctx->height,
		codec_ctx->pix_fmt,
		codec_ctx->width,
		codec_ctx->height,
		PIX_FMT_RGB24,
		SWS_BILINEAR,
		nullptr,
		nullptr,
		nullptr
	);
	pFrameRGB->width = codec_ctx->width;
	pFrameRGB->height = codec_ctx->height;

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, codec_ctx->width, codec_ctx->height);
	std::cout << "size " << pFrameRGB->width << " x " << pFrameRGB->height << "\n";
}


rgb_color::~rgb_color() {
	//av_free(pFrameRGB);
}

AVFrame *rgb_color::convert(AVFrame *in) const {
	// Convert the image from its native format to RGB
	sws_scale(sws_ctx,
		(uint8_t const * const *)in->data,
		in->linesize,
		0,
		pFrameRGB->height,
		pFrameRGB->data,
		pFrameRGB->linesize
	);
	return pFrameRGB;
}


audio_stream::audio_stream() {}


audio_stream::audio_stream(AVCodecContext *ctx)
	:
	codec_ctx(ctx) {
	init_codec(codec_ctx);

	// Allocate audio frame
	audio_frame = av_frame_alloc();

	buffer.reserve(1024 * 1024 * 32);
	read_audio = 0;

	sample = 0;

	// require copy constructor
	//av_free(out_frame);
}


int audio_stream::get_audio(uint8_t *audio_buf, int buf_size) {
	if (buffer.size() >= buf_size) {
		memcpy(audio_buf, &buffer[read_audio], buf_size);
		read_audio += buf_size;
	}
	else {
		std::cout << "audio unavailable\n";
	}
}


void audio_stream::add_audio(AVPacket *pkt) {
	int got_frame = 0;

	int audio_pkt_size = pkt->size;
	while(audio_pkt_size > 0) {
		int length = avcodec_decode_audio4(codec_ctx, audio_frame, &got_frame, pkt);

		/* if error, skip frame */
		if(length < 0) {
			log_error(length);
			return;
		}

		if (got_frame) {
			add_audio(audio_frame);
		}
		audio_pkt_size -= length;
	}
}


void audio_stream::add_audio(AVFrame *frame) {
	int data_size = av_samples_get_buffer_size(
		NULL,
		codec_ctx->channels,
		audio_frame->nb_samples,
		codec_ctx->sample_fmt,
		1
	);

	int start = buffer.size();
	buffer.resize(start + data_size);
	float *fbuffer = reinterpret_cast<float *>(&buffer[start]);
	

	switch (codec_ctx->sample_fmt) {
	case AV_SAMPLE_FMT_FLTP: {
		int fsize = data_size / sizeof(float);
		for (int c = 0; c < codec_ctx->channels; ++c) {
			float *fdata = reinterpret_cast<float *>(audio_frame->extended_data[c]);
			for (int s = 0; s < audio_frame->nb_samples; ++s) {
				fbuffer[s * codec_ctx->channels + c] = fdata[s];
			}
		}
		break;
	}
	default:
		std::cout << "unknown format\n";
		//memcpy(&buffer[start], audio_frame->data[0], data_size);
		break;
	}
}

video_stream::video_stream(AVCodecContext *ctx)
	:
	codec_ctx(ctx) {
	init_codec(codec_ctx);

	// Allocate video frame
	out_frame = av_frame_alloc();
}


AVFrame *video_stream::get_image() {
	return rgb.convert(out_frame);
}

void video_stream::add_image(AVPacket *pkt) {
	int frameFinished;
	AVFrame *result = nullptr;

	// Decode video frame
	int bytes = avcodec_decode_video2(codec_ctx, out_frame, &frameFinished, pkt);
	if (bytes < 0) {
		log_error(bytes);
		return;
	}

	// Did we get a video frame?
	if (frameFinished) {
		result = rgb.convert(out_frame);
	}
}


std::pair<int, int> video_stream::size() const {
	return {codec_ctx->width, codec_ctx->height};
}

av_stream::av_stream() {}


av_stream::av_stream(AVFormatContext *fc)
	:
	fmt_ctx(fc) {
	std::cout << "nb_streams: " << fmt_ctx->nb_streams << "\n";
}


av_stream::~av_stream() {}


audio_stream av_stream::get_audio_stream() const {
	return audio_stream(fmt_ctx->streams[0]->codec); // todo
}


video_stream av_stream::get_video_stream() const {
	return video_stream(fmt_ctx->streams[0]->codec); // todo
}


void av_stream::read(audio_stream &as) {
	AVPacket packet;
	
	int ret = av_read_frame(fmt_ctx, &packet);
	if (ret < 0) {
		log_error(ret);
		return;
	}

	//std::cout << "found index " << packet.stream_index << "\n";
	as.add_audio(&packet);

	av_free_packet(&packet);
}


context::context(const std::string &filename)
	:
	fmt_ctx(nullptr) {
	int ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL);
	log_error(ret);
}


context::context() {
	fmt_ctx = avformat_alloc_context();
	if (!fmt_ctx) {
		std::cout << "error allocating context\n";
	}
}


context::~context() {
	avformat_close_input(&fmt_ctx);
}


void context::stream_info() const {
	int ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		std::cout << "Could not find stream information\n";
	}
	av_dump_format(fmt_ctx, 0, "stream_info", 0);
}


av_stream context::get_stream() {
	return av_stream(fmt_ctx);
}


void context::set_avio(AVIOContext *avio_ctx) {

	// read file
	fmt_ctx->pb = avio_ctx;
	int ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	log_error(ret);
}


avio::avio(const buffer_data &bdin)
	:
	bd(bdin) {

	avio_ctx_buffer_size = 4096;
	avio_ctx_buffer = reinterpret_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
	if (!avio_ctx_buffer) {
		std::cout << "error allocating buffer\n";
	}	

	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &bd, &read_packet, NULL, NULL);
	if (!avio_ctx_buffer) {
		std::cout << "error allocating buffer\n";
	}	

}


avio::~avio() {
	/* note: the internal buffer could have changed, and be != avio_ctx_buffer */
	if (avio_ctx) {
		av_freep(&avio_ctx->buffer);
		av_freep(&avio_ctx);
	}
	// TODO free buffer
}


AVIOContext *avio::context() const {
	return avio_ctx;
}


file_map::file_map(const std::string &filename)
	:
	name(filename) {
	int ret = av_file_map(name.c_str(), &buffer, &buffer_size, 0, NULL);
	log_error(ret);
	std::cout << "buffer_size: " << buffer_size << "\n";
}


file_map::~file_map() {
	av_file_unmap(buffer, buffer_size);
}


uint8_t *file_map::ptr() const {
	return buffer;
}


size_t file_map::size() const {
	return buffer_size;
}


buffer_data file_map::data() const {
	return {buffer, buffer_size};
}


}
