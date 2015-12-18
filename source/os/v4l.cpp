#include <iostream>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <linux/videodev2.h>

#include "file.h"
#include "v4l.h"

namespace os {


void source_select(const capture_source &source) {
	fd_set fds;
	struct timeval tv;
	int r;

	FD_ZERO(&fds);
	FD_SET(source.fd, &fds);

	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	// returns negative for error, 0 for timeout
	r = select(source.fd + 1, &fds, NULL, NULL, &tv);
}


video::image<short> read_frame(const capture_source &source) {
	source_select(source);


	v4l2_buffer buf;
	memset(&(buf), 0, sizeof(buf));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if (-1 == xioctl(source.fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
		case EAGAIN:
			return video::image<short>(0, 0, 0);

		case EIO:
								/* Could ignore EIO, see spec. */
								/* fall through */
		default:
			errno_exit("VIDIOC_DQBUF");
		}
	}

	//assert(buf.index < buffers.size());
	if (-1 == xioctl(source.fd, VIDIOC_QBUF, &buf))
		errno_exit("VIDIOC_QBUF");

	short *buffer = reinterpret_cast<short *>(source.buffers[buf.index].start);
	video::image<short> img(buffer, 640, 480);
	return img;
}


void capture_source::start_capturing() {
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


void capture_source::init_mmap() {
	v4l2_requestbuffers req;
	memset(&(req), 0, sizeof(req));

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", dev_name.c_str());
			exit(EXIT_FAILURE);
		}
		else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name.c_str());
		exit(EXIT_FAILURE);
	}

	buffers.resize(req.count);
	for (int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		v4l2_buffer buf;
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

		if (MAP_FAILED == buffers[n_buffers].start) {
			errno_exit("mmap");
		}
	}
}


void capture_source::init_device() {
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;

	io_method io = IO_METHOD_MMAP;
	bool force_format = true;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", dev_name.c_str());
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", dev_name.c_str());
		exit(EXIT_FAILURE);
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n", dev_name.c_str());
			exit(EXIT_FAILURE);
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n", dev_name.c_str());
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
		fmt.fmt.pix.field       = V4L2_FIELD_NONE;

		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
			errno_exit("VIDIOC_S_FMT");

		/* Note VIDIOC_S_FMT may change width and height. */
	} else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

	/* Print format information */
	std::cout << "width " << fmt.fmt.pix.width << "\n";
	std::cout << "height " << fmt.fmt.pix.height << "\n";
	std::cout << "pixelformat " << std::string((char *)&fmt.fmt.pix.pixelformat, 4) << "\n";
	std::cout << "field " << fmt.fmt.pix.field << "\n";
	std::cout << "bytesperline " << fmt.fmt.pix.bytesperline << "\n";
	std::cout << "sizeimage " << fmt.fmt.pix.sizeimage << "\n";


	/* Buggy driver paranoia. */
	// unsigned int min = fmt.fmt.pix.width * 2;
	// if (fmt.fmt.pix.bytesperline < min)
	// 		fmt.fmt.pix.bytesperline = min;
	// min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	// if (fmt.fmt.pix.sizeimage < min)
	// 		fmt.fmt.pix.sizeimage = min;

	init_mmap();
}



capture_source::capture_source(const location &device)
	:
	dev_name(device.path()),
	fd(device.openfd()) {

	if (fd < 0) {
		return;
	}


	init_device();
	start_capturing();
}


capture_source::~capture_source() {
	close(fd);
}


}
