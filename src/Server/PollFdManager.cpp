
#include "PollFdManager.hpp"

#include <algorithm>
#include <random>

#include "Logger.hpp"

PollFdManager& PollFdManager::getInstance() {
	static PollFdManager instance;
	return instance;
}

void PollFdManager::addFd(const int fd) { _pollFds.push_back({fd, static_cast<short>(POLLIN | POLLOUT), 0}); }

void PollFdManager::removeFd(int fd) {
	_pollFds.erase(std::remove_if(_pollFds.begin(), _pollFds.end(), [fd](const pollfd& pfd) { return pfd.fd == fd; }),
				   _pollFds.end());
}

pollfd* PollFdManager::data() { return _pollFds.data(); }

size_t PollFdManager::size() const { return _pollFds.size(); }

std::vector<pollfd> PollFdManager::getPolls() const { return _pollFds; }

std::vector<pollfd> PollFdManager::getShuffledPolls() const {
	std::vector<pollfd> shuffledFds = _pollFds;
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(shuffledFds.begin(), shuffledFds.end(), g);
	return shuffledFds;
}
