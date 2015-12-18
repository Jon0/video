#include <iostream>

#include <audio/audio.h>
#include <os/socket.h>
#include <video/ffmpeg.h>
#include <video/stream.h>
#include <video/window.h>

void test() {
	os::tcp_socket socket("live-syd.twitch.tv", 1935);
	os::fdbuf buffer(socket.fd());
	std::iostream stream(&buffer);

	stream << static_cast<char>(0x03);

	std::string s;
	for (int i = 0; i < 1536; ++i) {
		s += static_cast<char>(i);
	}
	stream << s;

	std::string str_part;
	while (std::getline(stream, str_part, '\n')) {
		std::cout << str_part << "\n";
	}
}

void make_window() {
	video::ffmpeg ff;
	video::context vc("chopin.ogg"); // "sample.m4v";
	vc.stream_info();

	// get streams
	video::av_stream vs = vc.get_stream();
	video::audio_stream as = vs.get_audio_stream();
	audio::mixer mix(as);
	mix.list_devices();

	video::window w;
	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				return;
			}
		}
		vs.read(as);
		w.update();
	}
}


void example1() {
	video::ffmpeg ff;
	video::file_map test("test.h264");
	video::context c1;
	video::buffer_data bd = test.data();
	video::avio buf(bd);
	c1.set_avio(buf.context());
	c1.stream_info();
}

int main() {
	make_window();
	return 0;
}
