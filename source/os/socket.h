#pragma once

#include <string>

#include "file.h"

namespace os {

int connect_sockfd(const char *hostname, int port);
int listen_sockfd(int port);

class tcp_acceptor {
public:
	tcp_acceptor(int port);
	~tcp_acceptor();

	int acceptfd() const;

private:
	const int sockfd;

};

class tcp_socket {
public:
	tcp_socket(const std::string &host, int port);
	tcp_socket(const tcp_acceptor &a);
	~tcp_socket();

	int fd() const;

private:
	const int sockfd;


};

}
