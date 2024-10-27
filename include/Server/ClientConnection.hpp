#pragma once
#include <string>

class ClientConnection {
	public:
		explicit ClientConnection(int fd);
		~ClientConnection();

		void processRequest();
		[[nodiscard]] bool isDisconnected() const;

	private:
	int _clientFd;
	bool _disconnected;
	std::string _requestBuffer;

	bool receiveData();
	bool sendData(const std::string& data);
	void sendErrorResponse(int statusCode, const std::string& message);
	static bool isCompleteRequest(const std::string& buffer);
};
