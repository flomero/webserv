<?php
echo "Content-Type: text/html\n\n";

echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PHP CGI Example</title>
</head>
<body>
    <h1>Welcome to the PHP CGI Example</h1>
    <p>This is a simple example of a CGI script written in PHP.</p>
    <p>Current Date and Time: 
HTML;

echo date("Y-m-d H:i:s");

echo <<<HTML
	</p>
</body>
</html>
HTML;
?>