
#pragma once
#include <sys/poll.h>

#include <set>
#include <vector>

class PollFdManager {
	public:
		static PollFdManager& getInstance();
		PollFdManager(const PollFdManager&) = delete;
		PollFdManager& operator=(const PollFdManager&) = delete;

		void addFd(int fd);
		void addServerFd(int fd);
		void removeFd(int fd);

		pollfd* data();
		[[nodiscard]] size_t size() const;
		[[nodiscard]] std::vector<pollfd> getPolls() const;
		[[nodiscard]] bool isServerFd(int fd) const;

	private:
		PollFdManager() = default;
		~PollFdManager() = default;

		std::vector<pollfd> _pollFds;
		std::set<int> _serverFds;
};
