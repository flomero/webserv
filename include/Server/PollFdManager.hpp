
#pragma once
#include <vector>
#include <sys/poll.h>
#include <cstddef>

class PollFdManager {
	public:
		static PollFdManager& getInstance();
		PollFdManager(const PollFdManager&) = delete;
		PollFdManager& operator=(const PollFdManager&) = delete;

		void addFd(int fd);
		void removeFd(int fd);

		pollfd* data();
		[[nodiscard]] size_t size() const;
		[[nodiscard]] std::vector<pollfd> getPolls() const;

	private:
		PollFdManager() = default;
		~PollFdManager() = default;

		std::vector<pollfd> _pollFds;
};
