#!/usr/bin/env python3
# example.py - Basic Python CGI Script

import datetime

# Output the HTTP header
print("Content-Type: text/html")
print()

# Output the HTML content
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI Example</title>
</head>
<body>
    <h1>Welcome to the Python CGI Example</h1>
    <p>This is a simple example of a CGI script written in Python.</p>
    <p>Current Date and Time: {datetime.datetime.now()}</p>
</body>
</html>
""")