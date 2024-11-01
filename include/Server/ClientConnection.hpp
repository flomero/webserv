#pragma once
#include <netinet/in.h>

#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RequestHandler.hpp"

class ClientConnection {
	public:
		enum class Status {
			HEADER,
			BODY,
			READY_TO_SEND,
		};

		explicit ClientConnection(int clientFd, sockaddr_in clientAddr, ServerConfig& config);
		~ClientConnection();

		void handleClient();
		void sendResponse();
		[[nodiscard]] bool isDisconnected() const;

	private:
		int _clientFd;
		bool _disconnected;
		sockaddr_in _clientAddr;
		std::string _headerBuffer;
		std::string _bodyBuffer;
		RequestHandler _requestHandler;

		Status _status = Status::HEADER;
		HttpRequest _request = HttpRequest();
		HttpResponse _response = HttpResponse();

		void _logHeader() const;
		bool _readData(int fd, std::string& buffer, size_t bytesToRead);
		bool receiveHeader();
		bool _processHttpRequest(const std::string& header);
		bool _sendDataToClient(const std::string& data);
		static bool _isHeaderComplete(const std::string& buffer);
};
