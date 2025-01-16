#!/usr/bin/python3
import os
import sys

# Read the environment variables
method = os.getenv('REQUEST_METHOD')
query_string = os.getenv('QUERY_STRING', '')
content_length = os.getenv('CONTENT_LENGTH', '0')
content_type = os.getenv('CONTENT_TYPE', 'text/plain')

# Read POST data if method is POST
if method == "POST" and content_length:
    content_length = int(content_length)
    post_data = sys.stdin.read(content_length)
else:
    post_data = None

# Output CGI response headers
print("HTTP/1.1 200 OK")
print("Content-Type: text/html")
print()  # Empty line between headers and body (required)

# Output the response body
print("<html><body>")
print(f"<p>Method: {method}</p>")

if method == "GET":
    print(f"<p>Query String: {query_string}</p>")
elif method == "POST" and post_data:
    print(f"<p>POST Data: {post_data}</p>")
    print(f"<p>Content Type: {content_type}</p>")
    print(f"<p>Content Length: {content_length}</p>")

print("</body></html>")