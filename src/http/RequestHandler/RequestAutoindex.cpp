/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestAutoindex.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: flfische <flfische@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/22 14:25:06 by flfische          #+#    #+#             */
/*   Updated: 2024/12/10 17:05:53 by flfische         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <chrono>  // << NEU: sicherstellen, dass <chrono> inkludiert wird
#include <filesystem>
#include <iomanip>
#include <sstream>

#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "RequestHandler.hpp"

std::string humanReadableSize(const uintmax_t size) {
	constexpr uintmax_t KB = 1024;
	constexpr uintmax_t MB = KB * 1024;
	constexpr uintmax_t GB = MB * 1024;
	constexpr uintmax_t TB = GB * 1024;

	std::ostringstream result;
	if (size >= TB) {
		result << std::fixed << std::setprecision(2) << static_cast<double>(size) / TB << " TB";
	} else if (size >= GB) {
		result << std::fixed << std::setprecision(2) << static_cast<double>(size) / GB << " GB";
	} else if (size >= MB) {
		result << std::fixed << std::setprecision(2) << static_cast<double>(size) / MB << " MB";
	} else if (size >= KB) {
		result << std::fixed << std::setprecision(2) << static_cast<double>(size) / KB << " KB";
	} else {
		result << size << " B";
	}
	return result.str();
}

std::string formatTimestamp(const std::time_t time) {
	const std::tm* tm = std::localtime(&time);
	std::ostringstream ss;
	ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

std::string RequestHandler::buildDirectoryListingHTML(const std::string& path) const {
	std::ostringstream html;

	html << "<!DOCTYPE html>\n";
	html << "<html>\n";
	html << "<head>\n";
	html << "<title>Index of " << path << "</title>\n";
	html << "<meta charset=\"UTF-8\">\n";
	html << "<meta name=\"viewport\" content=\"width=device-width, "
			"initial-scale=1.0\">\n";
	html << "</head>\n";
	html << "<body>\n";
	html << "<h1>Index of " << path << "</h1>\n";
	html << "<hr>\n";
	html << "<table>\n";
	html << "<tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>\n";
	// TODO: check for server root
	if (path != "/") {
		html << "<tr><td><a href=\"..\">../</a></td><td>-</td><td>-</td></tr>\n";
	}

	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string entryPath = entry.path().string();
		std::string entryName = entry.path().filename().string();
		if (entry.is_directory() && !entryName.empty() && entryName.back() != '/') {
			entryName += "/";
		}

		// Größe (falls kein Verzeichnis)
		std::string entrySize =
			entry.is_directory() ? "-" : humanReadableSize(std::filesystem::file_size(entry.path()));

		// NEU: Portabler Zeit-Konvertierungscode
		auto ftime = entry.last_write_time();
		auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
			ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
		std::time_t entryTime = std::chrono::system_clock::to_time_t(sctp);

		std::string entryTimeStr = formatTimestamp(entryTime);

		std::string entryLink = _request.getLocation();
		if (!entryLink.empty() && entryLink.back() != '/' && entryLink != ".." && entryLink != "." &&
			entryLink != "/") {
			entryLink += "/";
		}
		entryLink += entryName;

		html << "<tr>"
			 << "<td><a href=\"" << entryLink << "\">" << entryName << "</a></td>"
			 << "<td>" << entrySize << "</td>"
			 << "<td>" << entryTimeStr << "</td>"
			 << "</tr>\n";
	}

	html << "</table>\n";
	html << "<hr>\n";
	html << "</body>\n";

	// Einfaches CSS:
	html << "<style>\n";
	html << "    * {\n";
	html << "        --light: #f2f2f2;\n";
	html << "        --dark: #ccc;\n";
	html << "        --background: #f2f2f2;\n";
	html << "        --color: #030303;\n";
	html << "        --link-color: #0000EE;\n";
	html << "    }\n";
	html << "    @media (prefers-color-scheme: dark) {\n";
	html << "        * {\n";
	html << "            --light: #222;\n";
	html << "            --dark: #3c3c3c;\n";
	html << "            --background: #000;\n";
	html << "            --color: #f2f2f2;\n";
	html << "            --link-color: #33bbff;\n";
	html << "        }\n";
	html << "    }\n";
	html << "    body {\n";
	html << "        font-family: Courier, monospace;\n";
	html << "        background-color: var(--background);\n";
	html << "        color: var(--color);\n";
	html << "    }\n";
	html << "    table {\n";
	html << "        width: 100%;\n";
	html << "        border-collapse: collapse;\n";
	html << "    }\n";
	html << "    th, td {\n";
	html << "        border: 1px solid var(--dark);\n";
	html << "        text-align: left;\n";
	html << "        padding: 8px;\n";
	html << "    }\n";
	html << "    tr:nth-child(even) {\n";
	html << "        background-color: var(--dark);\n";
	html << "    }\n";
	html << "    a {\n";
	html << "        color: var(--link-color);\n";
	html << "        text-decoration: none;\n";
	html << "    }\n";
	html << "    a:hover {\n";
	html << "        text-decoration: underline;\n";
	html << "    }\n";
	html << "    hr {\n";
	html << "        border: 0;\n";
	html << "        border-top: 1px solid var(--dark);\n";
	html << "    }\n";
	html << "</style>\n";

	html << "</html>\n";

	return html.str();
}

/**
 * @brief Handle a request for a directory listing.
 * @param path The path to the directory
 */
void RequestHandler::handleAutoindex(const std::string& path) {
	if (std::filesystem::is_directory(path)) {
		_response.setStatus(Http::OK);
		_response.setBody(buildDirectoryListingHTML(path));
		_response.addHeader("Content-Type", "text/html");
		_response.setDefaultHeaders();
	} else {
		_response.setStatus(Http::NOT_FOUND);
	}
}
