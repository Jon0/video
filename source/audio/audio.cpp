#include <iostream>
#include <cstring>

#include "../video/ffmpeg.h"
#include "audio.h"

namespace audio {

/* The audio function callback takes the following parameters:
 stream:  A pointer to the audio buffer to be filled
 len:     The length (in bytes) of the audio buffer
*/
void fill_audio(void *udata, Uint8 *stream, int len) {
	mixer *mix = reinterpret_cast<mixer *>(udata);
	float *st = reinterpret_cast<float *>(stream);
	mix->generate2(st, len);

	/* Mix as much data as possible */
	//len = ( len > audio_len ? audio_len : len );
	//std::cout << len << "\n";
	//SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	//std::memcpy(stream, audio_pos, len);
}



mixer::mixer(video::audio_stream &vs)
	:
	vs(vs) {
	channels = 2;
	sample = 0;
	freq = 44100;

	/* Set the audio format */
	SDL_AudioSpec spec;
	spec.freq = freq;
	spec.format = AUDIO_F32LSB;
	spec.channels = channels;    /* 1 = mono, 2 = stereo */
	spec.samples = 2048 * 2;  /* per channel samples */
	spec.silence = 0;
	spec.callback = fill_audio;
	spec.userdata = this;

	/* Open the audio device, forcing the desired format */
	if (SDL_OpenAudio(&spec, nullptr) < 0) {
		std::cout << "Couldn't open audio: " << SDL_GetError() << "\n";
	}

	/* Let the callback function play the audio chunk */
	SDL_PauseAudio(0);
	//SDL_Delay(1000);         /* Sleep 1/10 second */
}


mixer::~mixer() {
	SDL_CloseAudio();
}


void mixer::list_devices() const {
	int capture = 0;
	int number =  SDL_GetNumAudioDevices(capture);
	std::cout << "outputs " << number << "\n";
	for (int i = 0; i < number; ++i) {
		std::cout << SDL_GetAudioDeviceName(i, capture) << "\n";
	}
}


void mixer::generate(Sint16 *stream, int length) {
	double wave_freq = 300 * 2.0 * M_PI / static_cast<double>(freq);
	double amplitude = 5000.0;
	for (int i = 0; i < length; i += channels) {
		int index = static_cast<double>(sample + (i / 2));
		double sample = sin(index * wave_freq) * amplitude;
		for (int c = 0; c < channels; ++c) {
			stream[i + c] = static_cast<Sint16>(sample);
		}
	}
	sample += length / channels;
}

void mixer::generate2(float *stream, int length) {
	vs.get_audio(reinterpret_cast<uint8_t *>(stream), length);
}


}
