#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>

namespace os {

void error(const char *msg);

void errno_exit(const char *s);

int xioctl(int fh, int request, void *arg);


class location {
public:
	location(const std::string &path);

	bool exists() const;

	bool isdir() const;

	bool ischr() const;

	bool isexec() const;

	int size() const;

	std::string sizestr() const;

	std::string modified() const;

	std::string mode() const;

	std::string path() const;

	location append(const std::string &path) const;

	int openfd() const;

private:
	const std::string filepath;
	const int location_exists;
	struct stat location_stat;

};


/**
 * a file desciptor allows reading and writing
 * to various io devices
 */
class fdbuf : public std::streambuf {
public:
	typedef std::streambuf::traits_type traits_type;

	fdbuf(int fd);
	~fdbuf();
	void open(int fd);
	void close();

protected:
	int overflow(int c);
	int underflow();
	int sync();

private:
	enum { bufsize = 1024 };
	char outbuf_[bufsize];
	char inbuf_[bufsize + 16 - sizeof(int)];
	int  fd_;
};


/**
 * A file directory
 */
class directory {
public:
	directory(const std::string &path);
	virtual ~directory();

	/**
	 * read all files with a given extension
	 */
	std::vector<std::string> file_list() const;

private:
	std::string path;

};


// use the kernel sendfile function
//void sendfile(const std::string &filename) const {}

}
