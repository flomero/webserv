#!/usr/bin/perl
use CGI;
my $query = CGI->new;
print $query->header("text/html");
my $name = $query->param('name') || "World";
print "<h1>Hello, $name!</h1>";
