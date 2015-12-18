#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <librtmp/log.h>
#include <librtmp/rtmp.h>

#include "stream.h"
#include "output.h"

namespace video {

void start_stream(const std::string &rtmp_url) {

	// create source
	encoder e(AV_CODEC_ID_FLV1);

	
	RTMP *rtmp = RTMP_Alloc();
	if (!rtmp) {
		std::cerr << "Unable to create rtmp object\n";
		return;
	}

	RTMP_Init(rtmp);
	RTMP_LogSetLevel(RTMP_LOGINFO);
	RTMP_LogSetOutput(stderr);
	RTMP_SetupURL(rtmp, const_cast<char*>(rtmp_url.c_str())); // librtmp is a mess
	RTMP_EnableWrite(rtmp);

	// connect to server
	if (!RTMP_Connect(rtmp, NULL)) {
		std::cerr << "Unable to connect to server\n";
		RTMP_Free(rtmp);
		return;
	}

	// connect to stream (this will be the stream specified in the RTMP URL)
	if (!RTMP_ConnectStream(rtmp, 0)) {
		std::cerr << "Unable to connect to stream\n";
		RTMP_Free(rtmp);
		return;
	}

	auto ms = std::chrono::milliseconds(1000 / 25);
	auto start = std::chrono::system_clock::now();
	int i = 0;
	while (true) {
		auto now = std::chrono::system_clock::now();
		auto delay = (start + ms * i) - now;
		if (delay > std::chrono::milliseconds(0)) {
			std::this_thread::sleep_for(delay);
		}

    	// write the packet
		std::string frame = e.get_frame();
    	if (RTMP_Write(rtmp, frame.c_str(), frame.length()) <= 0) {
        	std::cerr << "Unable to write to server\n";
        	break;
    	}
    	i++;
	}

}

}
