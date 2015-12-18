#include <iostream>

#include "stream.h"

namespace video {

/*
 * Video encoding example
 */
void video_encode_example(const char *filename, AVCodecID codec_id) {
	AVCodec *codec;
	AVCodecContext *c = NULL;
	int i, ret, x, y, got_output;
	FILE *f;
	AVFrame *frame;
	AVPacket pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	printf("Encode video file %s\n", filename);

	/* find the mpeg1 video encoder */
	avcodec_register_all();
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = 352;
	c->height = 288;
	/* frames per second */
	c->time_base = (AVRational){1,25};
	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 10;
	//c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec_id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	f = fopen(filename, "wb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width  = c->width;
	frame->height = c->height;

	/* the image can be allocated by any means and av_image_alloc() is
	 * just the most convenient way if av_malloc() is to be used */
	ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
						 c->pix_fmt, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}

	/* encode 1 second of video */
	for (i = 0; i < 100; i++) {
		av_init_packet(&pkt);
		pkt.data = NULL;    // packet data will be allocated by the encoder
		pkt.size = 0;

		fflush(stdout);
		/* prepare a dummy image */
		/* Y */
		for (y = 0; y < c->height; y++) {
			for (x = 0; x < c->width; x++) {
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for (y = 0; y < c->height/2; y++) {
			for (x = 0; x < c->width/2; x++) {
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		frame->pts = i;

		/* encode the image */
		ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			exit(1);
		}

		if (got_output) {
			printf("Write frame %3d (size=%5d)\n", i, pkt.size);
			fwrite(pkt.data, 1, pkt.size, f);
			av_packet_unref(&pkt);
		}
	}

	/* get the delayed frames */
	for (got_output = 1; got_output; i++) {
		fflush(stdout);

		ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			exit(1);
		}

		if (got_output) {
			printf("Write frame %3d (size=%5d)\n", i, pkt.size);
			fwrite(pkt.data, 1, pkt.size, f);
			av_packet_unref(&pkt);
		}
	}

	/* add sequence end code to have a real mpeg file */
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_close(c);
	av_free(c);
	av_freep(&frame->data[0]);
	av_frame_free(&frame);
	printf("\n");
}



encoder::encoder(AVCodecID codec_id) {
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	i = 0;

	/* find the mpeg1 video encoder */
	avcodec_register_all();
	codec = avcodec_find_encoder(codec_id);
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = 352;
	c->height = 288;
	/* frames per second */
	c->time_base = (AVRational){1,25};
	/* emit one intra frame every ten frames
	 * check frame pict_type before passing frame
	 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	 * then gop_size is ignored and the output of encoder
	 * will always be I frame irrespective to gop_size
	 */
	c->gop_size = 10;
	//c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec_id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = c->pix_fmt;
	frame->width  = c->width;
	frame->height = c->height;

	/* the image can be allocated by any means and av_image_alloc() is
	 * just the most convenient way if av_malloc() is to be used */
	ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
						 c->pix_fmt, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}
}


encoder::~encoder() {
	avcodec_close(c);
	av_free(c);
	av_freep(&frame->data[0]);
	av_frame_free(&frame);
}


std::string encoder::get_frame() {
	av_init_packet(&pkt);
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;

	int n = i % 256;

	/* prepare a dummy image */
	/* Y */
	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			frame->data[0][y * frame->linesize[0] + x] = x + y + n * 3;
		}
	}

	/* Cb and Cr */
	for (y = 0; y < c->height/2; y++) {
		for (x = 0; x < c->width/2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = 128 + y + n * 2;
			frame->data[2][y * frame->linesize[2] + x] = 64 + x + n * 5;
		}
	}

	frame->pts = i;

	/* encode the image */
	ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
	if (ret < 0) {
		fprintf(stderr, "Error encoding frame\n");
		exit(1);
	}

	std::string result;
	if (got_output) {
		printf("Write frame %3d (size=%5d)\n", i, pkt.size);
		//fwrite(pkt.data, 1, pkt.size, f);
		char *d = reinterpret_cast<char *>(pkt.data);
		result = std::string(d, pkt.size);
		av_packet_unref(&pkt);


	}
	
	// inc frame
	i++;
	return result;
}


AVStream *add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id) {
    AVCodecContext *c;
    AVStream *st;
    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    /* open it */
	if (avcodec_open2(c, *codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    st->codec = avcodec_alloc_context3(*codec);
	st->id = oc->nb_streams - 1;

	/* timebase: This is the fundamental unit of time (in seconds) in terms
	 * of which frame timestamps are represented. For fixed-fps content,
	 * timebase should be 1/framerate and timestamp increments should be
	 * identical to 1. */
	st->time_base.den = 30;
	st->time_base.num = 1;

    c = st->codec;
    switch ((*codec)->type) {
	case AVMEDIA_TYPE_VIDEO:
		//avcodec_get_context_defaults3(c, *codec);
		c->codec_id = codec_id;

		/* put sample parameters */
		c->bit_rate = 150000;
		/* resolution must be a multiple of two */
		c->width = 352;
		c->height = 288;
		/* frames per second */
		c->time_base = (AVRational){1,25};

		/* emit one intra frame every ten frames
		 * check frame pict_type before passing frame
		 * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
		 * then gop_size is ignored and the output of encoder
		 * will always be I frame irrespective to gop_size
		 */
		c->gop_size = 10;
		c->pix_fmt = AV_PIX_FMT_YUV420P;

		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
			/* just for testing, we also add B frames */
			c->max_b_frames = 2;
		}
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
			/* Needed to avoid using macroblocks in which some coeffs overflow.
			 * This does not happen with normal video, it just happens here as
			 * the motion of the chroma plane does not match the luma plane. */
			c->mb_decision = 2;
		}
		break;

	default:
		break;
	}
	/* Some formats want stream headers to be separate. */
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	return st;
}


out_stream::out_stream(const std::string &path) {
	AVFormatContext *pFormatCtx = NULL;
	int i, videoStream;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec;
	AVPacket packet;
	int frameFinished;

	const char *filename = path.c_str();
	AVOutputFormat *fmt;
	AVFormatContext *oc;
	AVStream *video_st;
	AVCodec *video_codec;
	int ret;
	int frame_count = 0;

	// Register all formats, codecs and network
	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	// setup mux
	fmt = av_guess_format("flv", filename, NULL);
	if (fmt == NULL) {
		printf("Could not guess format.\n");
		return;
	}

    // allocate the output media context
    oc = avformat_alloc_context();
    if (oc == NULL) {
        printf("could not allocate context.\n");
        return;
    }
    oc->oformat = fmt;

    // Add the video streams using the default format codecs
    // and initialize the codecs.
    video_st = NULL;
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
    	std::cout << "add stream\n";
        video_st = add_stream(oc, &video_codec, fmt->video_codec);
    }

    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename); //, av_err2str(ret)
            return;
        }
    }

	// dump output format
	av_dump_format(oc, 0, filename, 1);

	// Write the stream header, if any.
	ret = avformat_write_header(oc, NULL);
	if (ret < 0) {
		fprintf(stderr, "Error occurred when opening output file\n");
		return;
	}

	// Read frames, decode, and re-encode
	AVFrame *frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}
	frame->format = video_st->codec->pix_fmt;
	frame->width  = video_st->codec->width;
	frame->height = video_st->codec->height;

	/* the image can be allocated by any means and av_image_alloc() is
	 * just the most convenient way if av_malloc() is to be used */
	ret = av_image_alloc(frame->data, frame->linesize, video_st->codec->width, video_st->codec->height, video_st->codec->pix_fmt, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate raw picture buffer\n");
		exit(1);
	}

	std::cout << "start sending\n";
	while (true) {

		// encode the image
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = NULL; // packet data will be allocated by the encoder
		pkt.size = 0;
		int n = frame_count % 256;
		int got_output;

		/* prepare a dummy image */
		/* Y */
		for (int y = 0; y < video_st->codec->height; y++) {
			for (int x = 0; x < video_st->codec->width; x++) {
				frame->data[0][y * frame->linesize[0] + x] = x + y + n * 3;
			}
		}

		/* Cb and Cr */
		for (int y = 0; y < video_st->codec->height/2; y++) {
			for (int x = 0; x < video_st->codec->width/2; x++) {
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + n * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + n * 5;
			}
		}
		frame->pts = frame_count;

		/* encode the image */
		std::cout << "encoding\n";
		ret = avcodec_encode_video2(video_st->codec, &pkt, frame, &got_output);
		std::cout << "done\n";
		if (ret < 0) {
			fprintf(stderr, "Error encoding frame\n");
			exit(1);
		}

		if (got_output) {
			std::cout << "writing\n";
			if (video_st->codec->coded_frame->key_frame)
				pkt.flags |= AV_PKT_FLAG_KEY;
			
			pkt.stream_index = video_st->index;

			// Write the compressed frame to the media file.
			ret = av_interleaved_write_frame(oc, &pkt);
		}
		else {
			std::cout << "no output\n";
			ret = 0;
		}
		if (ret != 0) {
			fprintf(stderr, "Error while writing video frame\n");
			exit(1);
		}

		fprintf(stderr, "encoded frame #%d\n", frame_count);
		frame_count++;
		av_free_packet(&pkt);
		std::cout << "done\n";
    }

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(oc);

    // Free the streams.
    for (i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
    if (!(fmt->flags & AVFMT_NOFILE))
        /* Close the output file. */
        avio_close(oc->pb);

    /* free the stream */
    av_free(oc);

    // Free the YUV frame holding decoded frame
    av_free(frame);

    // Close the decoder (codec)
    avcodec_close(pCodecCtx);

    // Close the input video file
    avformat_close_input(&pFormatCtx);

}


}
