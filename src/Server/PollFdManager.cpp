
#include "PollFdManager.hpp"
PollFdManager& PollFdManager::getInstance() {
	static PollFdManager instance;
	return instance;
}
void PollFdManager::addFd(const int fd) { _pollFds.push_back({fd, POLLIN, 0}); }
void PollFdManager::addServerFd(int fd) {
	_pollFds.push_back({fd, POLLIN, 0});
	_serverFds.insert(fd);
}

void PollFdManager::removeFd(int fd) {
	_pollFds.erase(std::remove_if(_pollFds.begin(), _pollFds.end(), [fd](const pollfd& pfd) { return pfd.fd == fd; }),
				   _pollFds.end());
	_serverFds.erase(fd);
}
pollfd* PollFdManager::data() { return _pollFds.data(); }
size_t PollFdManager::size() const { return _pollFds.size(); }
std::vector<pollfd> PollFdManager::getPolls() const { return _pollFds; }
bool PollFdManager::isServerFd(const int fd) const { return _serverFds.find(fd) != _serverFds.end(); }
