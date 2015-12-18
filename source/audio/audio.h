#pragma once

#include <SDL2/SDL.h>

namespace video {
	class audio_stream;
}

namespace audio {

class mixer {
public:
	mixer(video::audio_stream &vs);
	~mixer();

	void list_devices() const;

	void generate(Sint16 *stream, int length);
	void generate2(float *stream, int length);

private:
	video::audio_stream &vs;
	int channels;
	int sample;
	int freq;
};

}
