
#pragma once

#include <netinet/in.h>

class Socket {
	public:
		explicit Socket(int port);
		~Socket();

		void bind();
		void listen() const;
		[[nodiscard]] int accept() const;
		[[nodiscard]] int getSocketFd() const;

	private:
		void setupAddress();
		void setSocketOpt() const;
		int _socketFd;
		int _port;
		sockaddr_in _addr;
};
