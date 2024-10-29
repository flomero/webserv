
#pragma once
#include <sys/poll.h>

#include <vector>

class PollFdManager {
	public:
		static PollFdManager& getInstance();

		void addFd(int fd);
		void removeFd(int fd);
		pollfd* data();
		[[nodiscard]] size_t size() const;
		PollFdManager(const PollFdManager&) = delete;
		PollFdManager& operator=(const PollFdManager&) = delete;
		[[nodiscard]] std::vector<pollfd> getPolls() const;

	private:
		PollFdManager() = default;
		~PollFdManager() = default;

		std::vector<pollfd> _pollFds;
};
