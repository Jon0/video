#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

enum io_method {
		IO_METHOD_READ,
		IO_METHOD_MMAP,
		IO_METHOD_USERPTR,
};

struct buffer {
		void   *start;
		size_t  length;
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


void errno_exit(const char *s) {
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}


int xioctl(int fh, int request, void *arg) {
	int r;
	do {
		r = ioctl(fh, request, arg);
	} while (-1 == r && EINTR == errno);

	return r;
}




void draw(short *data, int size) {
	int in_width = 640;
	int in_height = 480;
	int fmt_size = sizeof(short);
	image<short> img(data, 640, 480);


	std::string pix = " .,:;i1tfLCG08@";
	int dval = 0x0fff / pix.length();
	int r_size = 16;
	for (int y = 0; y < in_height; y += r_size) {
		for (int x = 0; x < in_width; x += r_size) {
			int val = img.area(x, y, r_size);
			int ind = pix.length() - (val % pix.length()) - 1;
			std::cout << pix[ind] << pix[ind];
		}
		std::cout << "\n";
	}
	for (int y = 0; y < 20; y += r_size) {
		std::cout << "\n";
	}
}

int read_frame(int fd, const std::vector<buffer> &buffers) {
	struct v4l2_buffer buf;
	memset(&(buf), 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return 0;

		case EIO:
								/* Could ignore EIO, see spec. */
								/* fall through */
		default:
			errno_exit("VIDIOC_DQBUF");
		}
	}

	//assert(buf.index < buffers.size());
	draw((short *)buffers[buf.index].start, buf.bytesused);
	//std::string text((char *)buffers[buf.index].start, buf.bytesused);
	//std::cout << text;
	//std::cout << "recv: " << buf.bytesused << "\n";
	//std::cout << "expt: " << (640*480*2) << "\n"; 

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	return 1;
}


void start_capturing(int fd, const std::vector<buffer> &buffers) {
	v4l2_buf_type type;

	for (int i = 0; i < buffers.size(); ++i) {
		struct v4l2_buffer buf;
		memset(&(buf), 0, sizeof(buf));

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
		errno_exit("VIDIOC_STREAMON");
}

std::vector<buffer> init_mmap(int fd, const char *dev_name) {
		v4l2_requestbuffers req;
		memset(&(req), 0, sizeof(req));

		req.count = 4;
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		req.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
				if (EINVAL == errno) {
						fprintf(stderr, "%s does not support "
								 "memory mapping\n", dev_name);
						exit(EXIT_FAILURE);
				} else {
						errno_exit("VIDIOC_REQBUFS");
				}
		}

		if (req.count < 2) {
				fprintf(stderr, "Insufficient buffer memory on %s\n",
						 dev_name);
				exit(EXIT_FAILURE);
		}

		std::vector<buffer> buffers;
		buffers.resize(req.count);

		for (int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
				struct v4l2_buffer buf;
				memset(&(buf), 0, sizeof(buf));

				buf.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory	  = V4L2_MEMORY_MMAP;
				buf.index	   = n_buffers;

				if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
						errno_exit("VIDIOC_QUERYBUF");

				buffers[n_buffers].length = buf.length;
				buffers[n_buffers].start =
						mmap(NULL /* start anywhere */,
							  buf.length,
							  PROT_READ | PROT_WRITE /* required */,
							  MAP_SHARED /* recommended */,
							  fd, buf.m.offset);

				if (MAP_FAILED == buffers[n_buffers].start)
						errno_exit("mmap");
		}
	return buffers;
}


std::vector<buffer> init_device(int fd, const char *dev_name) {
		struct v4l2_capability cap;
		struct v4l2_cropcap cropcap;
		struct v4l2_crop crop;
		struct v4l2_format fmt;
		unsigned int min;
		io_method   io = IO_METHOD_MMAP;
		bool force_format = false;

		if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
				if (EINVAL == errno) {
						fprintf(stderr, "%s is no V4L2 device\n",
								 dev_name);
						exit(EXIT_FAILURE);
				} else {
						errno_exit("VIDIOC_QUERYCAP");
				}
		}

		if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
				fprintf(stderr, "%s is no video capture device\n",
						 dev_name);
				exit(EXIT_FAILURE);
		}



		switch (io) {
		case IO_METHOD_READ:
				if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
						fprintf(stderr, "%s does not support read i/o\n",
								 dev_name);
						exit(EXIT_FAILURE);
				}
				break;

		case IO_METHOD_MMAP:
		case IO_METHOD_USERPTR:
				if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
						fprintf(stderr, "%s does not support streaming i/o\n",
								 dev_name);
						exit(EXIT_FAILURE);
				}
				break;
		}


		/* Select video input, video standard and tune here. */

		memset(&(cropcap), 0, sizeof(cropcap));
		cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
				crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				crop.c = cropcap.defrect; /* reset to default */

				if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
						switch (errno) {
						case EINVAL:
								/* Cropping not supported. */
								break;
						default:
								/* Errors ignored. */
								break;
						}
				}
		} else {
				/* Errors ignored. */
		}

		memset(&(fmt), 0, sizeof(fmt));

		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (force_format) {
				fmt.fmt.pix.width       = 640;
				fmt.fmt.pix.height      = 480;
				fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
				fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

				if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
						errno_exit("VIDIOC_S_FMT");

				/* Note VIDIOC_S_FMT may change width and height. */
		} else {
				/* Preserve original settings as set by v4l2-ctl for example */
				if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
						errno_exit("VIDIOC_G_FMT");
		}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
			fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;

	return init_mmap(fd, dev_name);
}


bool valid_path(const char *dev_name) {
	struct stat st;

	if (-1 == stat(dev_name, &st)) {
		//fprintf(stderr, "Cannot identify '%s': %d, %s\n",
		//		 dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		//fprintf(stderr, "%s is no device\n", dev_name);
		exit(EXIT_FAILURE);
	}
	return true;
}

void open_device(const device &d) {
	const char *dev_name = d.path.c_str();

	if (valid_path(dev_name)) {
		int fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
		auto buffers = init_device(fd, dev_name);
		start_capturing(fd, buffers);

		while (1) {
			read_frame(fd, buffers);
		}
		

		if (-1 == fd) {
			//fprintf(stderr, "Cannot open '%s': %d, %s\n",
			//		 dev_name, errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

int run() {
	device d1("/dev/video0");

	open_device(d1);
	return 0;
}
