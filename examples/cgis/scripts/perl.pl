#!/usr/bin/perl

use strict;
use warnings;

# Print the HTTP header
print "Content-type: text/html\n\n";

# Print the HTML content
print <<"HTML";
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Perl CGI Example</title>
</head>
<body>
    <h1>Welcome to the Perl CGI Example</h1>
    <p>This is a simple example of a CGI script written in Perl.</p>
    <p>Current Date and Time: @{[ scalar localtime ]}</p>
</body>
</html>
HTML