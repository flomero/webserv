#!/usr/bin/env python3
import os
import time
import cgitb
import json
from http.server import SimpleHTTPRequestHandler, HTTPServer

cgitb.enable()

UPLOAD_DIR = "./uploads"

def list_files():
    return os.listdir(UPLOAD_DIR)

def main():
    # time.sleep(50) # Uncomment to test timeout
    print("Content-Type: application/json\n")
    files = list_files()
    files = [f for f in files if f != ".gitignore"]
    print(json.dumps(files))

if __name__ == "__main__":
    main()
