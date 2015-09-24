#pragma once 

#include <string>
#include <vector>

namespace video {

enum io_method {
		IO_METHOD_READ,
		IO_METHOD_MMAP,
		IO_METHOD_USERPTR,
};

struct buffer {
		void   *start;
		size_t  length;
};

struct capture_source {
	int fd; 
	std::vector<buffer> buffers;
};

class device {
public:
	device(const std::string &dev_path)
		:
		path(dev_path) {

	}

	const std::string path;
};

template<typename T>
class image {
public:
	image(T *data, int width, int height)
		:
		data(data),
		width(width),
		height(height) {}


	T get(int x, int y) {
		return data[(y * width) + x];
	}


	int area(int x, int y, int size) {
		int result = 0;
		for (int dx = 0; dx < size; ++dx) {
			for (int dy = 0; dy < size; ++dy) {
				result += data[((y + dy) * width) + (x + dx)];
			}
		}
		return data[(y * width) + x] / (size * size);
	}

	const T *data;
	const int width, height;
};

capture_source open_device(const device &d);
image<short> read_frame(const capture_source &source);

}
