#pragma once
#include <netinet/in.h>
#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RequestHandler.hpp"

class ClientConnection {
	public:
		enum class Status { HEADER, BODY, READY_TO_SEND, SENDING_RESPONSE };

		explicit ClientConnection(int clientFd, sockaddr_in clientAddr, std::vector<ServerConfig> configs);
		~ClientConnection();

		void handleClient();
		void sendResponse();
		[[nodiscard]] bool isDisconnected() const;

		[[nodiscard]] Status getStatus() const;

	private:
		int _clientFd;
		bool _disconnected;
		ServerConfig& _currentConfig;
		std::vector<ServerConfig> _configs;
		sockaddr_in _clientAddr;
		std::vector<char> _headerBuffer;
		std::vector<char> _bodyBuffer;
		RequestHandler _requestHandler;

		Status _status = Status::HEADER;

		HttpRequest _request = HttpRequest();
		HttpResponse _response = HttpResponse();

		bool _readingChunkSize = true;
		size_t _chunkSizeRemaining = 0;
		size_t _bytesSendToClient = 0;

		void _handleCompleteChunkedBodyRead();
		bool _readChunkData();
		bool _readChunkTerminator();
		void _readRequestBodyIfChunked();
		bool _parseChunkSize();
		void _receiveBody();
		bool _extractHeaderIfComplete(std::vector<char>& header);
		void _logHeader() const;
		bool _readData(int fd, std::vector<char>& buffer, size_t bytesToRead);
		bool _receiveHeader();
		void _readRequestBodyIfContentLength();
		void _handleCompleteBodyRead();
		bool _parseHttpRequestHeader(const std::string& header);
		bool _sendDataToClient(const std::string& data, size_t offset, size_t length);
		static std::optional<size_t> _findHeaderEnd(const std::vector<char>& buffer);
		[[nodiscard]] std::string _log(const std::string& msg) const;
};
