import requests
import json
from colorama import init, Fore, Style

# Initialize colorama for colored output
init(autoreset=True)

# Base URL for the server
BASE_URL = "http://localhost:8080"

# Utility function to display test results
def print_result(title, success, method=None, endpoint=None):
	if success:
		print(f"{Fore.GREEN}✅ {title}")
	else:
		print(f"{Fore.RED}❌ {title}")
		if method and endpoint:
			print(f"{Fore.RED}   → {method} {endpoint}")

# Utility function to make requests and handle exceptions
def make_request(title, method, endpoint, headers=None, data=None, files=None, expected_status=None):
	try:
		response = requests.request(method, BASE_URL + endpoint, headers=headers, data=data, files=files)
		success = response.status_code == expected_status
		print_result(title, success, method, endpoint)
		if not success:
			print(f"{Fore.RED}   Expected: {expected_status}, Got: {response.status_code}\nResponse: {response.text}\n")
	except requests.exceptions.RequestException as e:
		print_result(title, False, method, endpoint)
		print(f"{Fore.RED}   Error: {e}")

# Testing GET requests
def test_get_requests():
	print("\nGET Requests")
	make_request("GET request with valid headers.", "GET", "/", expected_status=200)
	make_request("GET request without headers.", "GET", "/", headers={}, expected_status=200)
	make_request("GET request with missing required headers.", "GET", "/", expected_status=200)  # Assuming your server doesn't require specific headers
	make_request("GET request with extra unnecessary headers.", "GET", "/", headers={"X-Unnecessary-Header": "test"}, expected_status=200)
	make_request("GET request with malformed URL.", "GET", "/%wrong%url", expected_status=400)
	make_request("GET request to a non-existent endpoint.", "GET", "/nonexistent", expected_status=404)
	make_request("GET request with body.", "GET", "/", data="This should be ignored", expected_status=200)

	make_request("GET request HTTP/1.1 without keep-alive.", "GET", "/", headers={"Connection": "close"}, expected_status=200)

# Testing POST requests
def test_post_requests():
	print("\nPOST Requests")

	# Create a file for upload testing
	with open("test_file.txt", "w") as f:
		f.write("This is a test file.")

	# Valid multipart/form-data file upload
	with open("test_file.txt", "rb") as f:
		make_request("POST request with valid multipart form-data.", "POST", "/upload", files={"file": f}, expected_status=200)

	# Invalid content type
	make_request("POST request with invalid content type.", "POST", "/upload", headers={"Content-Type": "application/json"}, data=json.dumps({"file": "test"}), expected_status=415)

	# Empty body
	make_request("POST request with empty body.", "POST", "/upload", expected_status=400)

	# Non-existent endpoint
	make_request("POST request to a non-existent endpoint.", "POST", "/nonexistent", expected_status=405)

# Testing DELETE requests
def test_delete_requests():
	print("\nDELETE Requests")
	make_request("DELETE request with valid headers.", "DELETE", "/uploads/test_delete.txt", expected_status=200)
	make_request("DELETE request to a non-existent endpoint.", "DELETE", "/nonexistent", expected_status=404)

# Testing CGI requests
def test_cgi_requests():
	print("\nCGI Requests")
	make_request("CGI GET request with valid headers and query parameters.", "GET", "/cgi-bin/hello.py?name=Test", expected_status=200)
	make_request("CGI POST request with valid headers and valid JSON body.", "POST", "/cgi-bin/hello.py", headers={"Content-Type": "application/json"}, data=json.dumps({"name": "Test"}), expected_status=200)
	make_request("CGI request to a non-existent CGI script.", "GET", "/cgi-bin/nonexistent.py", expected_status=404)
	make_request("CGI request with malformed URL targeting CGI scripts.", "GET", "/cgi-bin/%invalid-url%", expected_status=400)

# General invalid tests
def test_invalid_requests():
	print("\nGeneral Invalid Tests")
	make_request("Request with unsupported HTTP method (PATCH).", "PATCH", "/", expected_status=501)
	make_request("Request with extremely large payload.", "POST", "/upload", data="A" * 10000000, expected_status=413)

# Main function to run all tests
if __name__ == "__main__":
	print("Starting tests for webserv project...\n")
	# test_get_requests()
	# test_post_requests()
	# test_delete_requests()
	# test_cgi_requests()
	# test_invalid_requests()
	make_request("GET request HTTP/1.1 without keep-alive.", "GET", "/", headers={"Connection": "close"}, expected_status=200)
	print("\nAll tests completed.")
