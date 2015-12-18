#include <iostream>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "socket.h"

namespace os {


int open_socket(const sockaddr_in &serv_addr) {
	// open socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	// bind and listen on socket
	if (connect(sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}
	listen(sockfd, 5);
	return sockfd;
}

int connect_sockfd(const char *hostname, int port) {
	struct hostent *hostinfo;

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) {
		fprintf (stderr, "Unknown host %s.\n", hostname);
		exit (EXIT_FAILURE);
	}
	serv_addr.sin_addr = *(struct in_addr *) hostinfo->h_addr;
	return open_socket(serv_addr);

}


int listen_sockfd(int port) {
	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	return open_socket(serv_addr);
}


tcp_acceptor::tcp_acceptor(int port)
	:
	sockfd(listen_sockfd(port)) {
}


tcp_acceptor::~tcp_acceptor() {
	close(sockfd);
}


int tcp_acceptor::acceptfd() const {
	sockaddr_in cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) {
		error("ERROR on accept");
	}
	return newsockfd;
}


tcp_socket::tcp_socket(const std::string &host, int port)
	:
	sockfd(connect_sockfd(host.c_str(), port)) {
}


tcp_socket::tcp_socket(const tcp_acceptor &a)
	:
	sockfd(a.acceptfd()) {
}


tcp_socket::~tcp_socket() {
	close(sockfd);
}


int tcp_socket::fd() const {
	return sockfd;
}


}
