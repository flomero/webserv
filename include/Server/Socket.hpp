
#pragma once

#include <netinet/in.h>

class Socket {
	public:
		explicit Socket(int port);
		~Socket();

		void bind();
		void listen() const;
		int accept();

	private:
		int _socketFd;
		int _port;
		sockaddr_in _addr;
};
