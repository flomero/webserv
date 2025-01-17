
#pragma once
#include <sys/poll.h>

#include <cstddef>
#include <vector>

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
		[[nodiscard]] std::vector<pollfd> getShuffledPolls() const;

	private:
		PollFdManager() = default;
		~PollFdManager() = default;

		std::vector<pollfd> _pollFds;
};
