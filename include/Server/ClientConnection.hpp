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
			COMPLETE,
		};

		explicit ClientConnection(int clientFd, sockaddr_in clientAddr, ServerConfig& config);
		~ClientConnection();

		void handleClient();

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

		void processRequest();
		bool receiveHeader();
		bool sendData(const std::string& data);

		void sendErrorResponse(int statusCode, const std::string& message);
		static bool isCompleteHeader(const std::string& buffer);
};
