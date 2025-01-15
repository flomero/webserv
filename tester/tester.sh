#!/bin/bash

# Define server URL
SERVER_URL="http://localhost:8080"

# Utility function to make requests and display results
make_request() {
  echo "==== $1 ===="
  curl -v $2 $SERVER_URL$3
  echo -e "\n\n"
}

# GET requests
make_request "GET request with valid headers" "-H 'Accept: application/json'" "/"
make_request "GET request without headers" "" "/"
make_request "GET request with missing required headers" "" "/"
make_request "GET request with extra unnecessary headers" "-H 'X-Custom-Header: CustomValue'" "/"
make_request "GET request with malformed URL" "" "/%wrong%url"
make_request "GET request to a non-existent endpoint" "" "/nonexistent"
make_request "GET request with body" "-d 'This should be ignored'" "/"

# POST requests
make_request "POST request with valid headers and valid JSON body" "-X POST -H 'Content-Type: application/json' -d '{\"name\": \"test\"}'" "/"
make_request "POST request with valid headers and valid form-encoded body" "-X POST -H 'Content-Type: application/x-www-form-urlencoded' -d 'name=test'" "/"
make_request "POST request with valid headers but empty body" "-X POST -H 'Content-Type: application/json'" "/"
make_request "POST request with valid headers and invalid JSON body" "-X POST -H 'Content-Type: application/json' -d 'invalid-json'" "/"
make_request "POST request with valid headers and binary body" "-X POST -H 'Content-Type: application/octet-stream' --data-binary @/bin/bash" "/"
make_request "POST request without headers and valid JSON body" "-X POST -d '{\"name\": \"test\"}'" "/"
make_request "POST request to a non-existent endpoint" "-X POST -d 'name=test'" "/nonexistent"

# DELETE requests
make_request "DELETE request with valid headers" "-X DELETE" "/"
make_request "DELETE request to a non-existent endpoint" "-X DELETE" "/nonexistent"

# CGI requests
make_request "CGI GET request with valid query parameters" "" "/cgi-bin/hello?name=test"
make_request "CGI POST request with valid JSON body" "-X POST -H 'Content-Type: application/json' -d '{\"name\": \"test\"}'" "/cgi-bin/hello"
