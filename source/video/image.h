#pragma once 

#include <string>
#include <vector>

namespace video {

// shader textures
// input and output image formats

struct format {
	int width;
	int height;
	int color;
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


}
