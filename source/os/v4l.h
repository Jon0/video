#pragma once

#include <string>
#include <vector>

#include "../video/image.h"
#include "file.h"

namespace os {

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR
};

struct buffer {
		void   *start;
		size_t  length;
};

class capture_source {
public:
	capture_source(const location &device);
	~capture_source();

	const std::string dev_name;
	const int fd;
	std::vector<buffer> buffers;

private:
	void start_capturing();
	void init_mmap();
	void init_device();

};

video::image<short> read_frame(const capture_source &source);

}
