/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/06 13:58:51 by lgreau            #+#    #+#             */
/*   Updated: 2024/10/15 15:03:59 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Route.hpp"

// Constructor
Route::Route() : _autoindex(false), _code(0) {}

// Getters
const std::string& Route::getPath() const { return _path; }

const std::string& Route::getAlias() const { return _alias; }

const std::vector<std::string>& Route::getMethods() const { return _methods; }

const std::string& Route::getRoot() const { return _root; }

const std::string& Route::getIndex() const { return _index; }

bool Route::isAutoindex() const { return _autoindex; }

const std::string& Route::getUploadDir() const { return _uploadDir; }

const std::map<std::string, std::string>& Route::getCgiHandlers() const { return _cgiHandlers; }

int Route::getCode() const { return _code; }

const std::string& Route::getRedirect() const { return _redirect; }

// Setters
void Route::setPath(const std::string& path) { _path = path; }

void Route::setAlias(const std::string& alias) { _alias = alias; }

void Route::setMethods(const std::vector<std::string>& methods) { _methods = methods; }

void Route::setRoot(const std::string& root) { _root = root; }

void Route::setIndex(const std::string& index) { _index = index; }

void Route::setAutoindex(bool autoindex) { _autoindex = autoindex; }

void Route::setUploadDir(const std::string& dir) { _uploadDir = dir; }

void Route::setCgiHandlers(const std::map<std::string, std::string>& handlers) { _cgiHandlers = handlers; }

void Route::setCode(int code) { _code = code; }

void Route::setRedirect(const std::string& redirect) { _redirect = redirect; }

// Overload "<<" operator
std::ostream& operator<<(std::ostream& os, const Route& route) {
	os << "path: " << COLOR(BLUE, route.getPath()) << "\n";

	if (route.getAlias() != "") {
		os << std::left << std::setw(24) << "      |- alias: " << route.getAlias() << "\n";
	}
	if (route.getMethods().size() != 0) {
		os << std::left << std::setw(24) << "      |- methods: ";
		for (const auto& method : route.getMethods()) os << method << " ";
		os << "\n";
	}

	if (route.getRoot() != "") {
		os << std::left << std::setw(24) << "      |- root: " << route.getRoot() << "\n";
	}
	if (route.getIndex() != "") {
		os << std::left << std::setw(24) << "      |- index: " << route.getIndex() << "\n";
	}

	os << std::left << std::setw(24) << "      |- autoindex: " << (route.isAutoindex() ? "on" : "off") << "\n";

	if (route.getUploadDir() != "") {
		os << std::left << std::setw(24) << "      |- upload dir: " << route.getUploadDir() << "\n";
	}

	if (route.getCgiHandlers().size() != 0) {
		os << "      |- cgi handlers: \n";
		for (const auto& handler : route.getCgiHandlers())
			os << "        |- " << std::left << std::setw(6) << (handler.first + ": ") << handler.second << "\n";
	}

	if (route.getCode() != 0) {
		os << std::left << std::setw(24) << "      |- code: " << RED << route.getCode() << RESET_COLOR << "\n";
	}
	if (route.getRedirect() != "") {
		os << std::left << std::setw(24) << "      |- redirect/content: " << route.getRedirect() << "\n";
	}
	return os;
}
