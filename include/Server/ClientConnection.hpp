#pragma once
#include <netinet/in.h>

#include <string>

class ClientConnection {
	public:
		explicit ClientConnection(int clientFd, sockaddr_in clientAddr);
		~ClientConnection();

		void processRequest();
		[[nodiscard]] bool isDisconnected() const;
		bool receiveData();
		bool sendData(const std::string& data);

	private:
		int _clientFd;
		bool _disconnected;
		sockaddr_in _clientAddr;
		std::string _requestBuffer;

		void sendErrorResponse(int statusCode, const std::string& message);
		static bool isCompleteRequest(const std::string& buffer);
};
