#!/usr/bin/env python3
import sys
import os
import cgi
import cgitb
import json
from http.server import SimpleHTTPRequestHandler, HTTPServer

# Enable error logging
cgitb.enable()

# Directory to manage
# TODO: Change when CGI is fixed
UPLOAD_DIR = "./default/uploadthing/uploads"

def list_files():
    return os.listdir(UPLOAD_DIR)

def main():
    print("Content-Type: application/json\n")
    
    # Handle HTTP requests
    form = cgi.FieldStorage()
    request_method = os.environ.get('REQUEST_METHOD', 'GET')
  
    files = list_files()
    # remove .gitignore
    files = [f for f in files if f != ".gitignore"]
    print(json.dumps(files))

if __name__ == "__main__":
    main()
