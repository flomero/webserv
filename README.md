<p align="center">
  <img src="https://github.com/ayogun/42-project-badges/blob/main/badges/webservm.png?raw=true" alt="miniRT"/>
</p>

<p align="center">
  Score: <i>125/100</i><br>
  Finished: <i>20.01.2025</i>
</p>
 
# webserv

_This project is about writing your our HTTP server._

## Features

- HTTP/1.1 compliant and non-blocking
- supports `GET`, `POST` and `DELETE` methods
- native File uploads using `multipart/form-data`
- `CGI` scripts
- `autoindex` feature
- HTTP redirections
- custom error pages
- chunked transfer encoding
- `keep-alive` connections
- fully configurable using a `nginx`-like configuration file. see [Configuration](#configuration)

The repository also includes some showcases/tests in the `examples` directory.

## Installation

```bash
git clone  ...
cd webserv
make
```

## Usage

```bash
./webserv [config_file]
```

## Configuration

### Simple Example

```nginx
http {
    server {
        listen 80;
        server_name localhost 127.0.0.1;

        root /www;
        index /index.html;

        client_max_body_size 1024m;
        client_body_buffer_size 1024k;
        client_header_buffer_size 1024k;

        request_timeout 10m;
        location / {
            allow_methods GET;
            autoindex on;
        }
    }
}
```

### Multiple Servers

> [!NOTE]
> If there are multiple servers with the same `host:port` pair, the first one will be used.

```nginx
http {
	server {
		...
	}
	server {
		...
	}
}
```

### Server Options

| directive                   | description                             | example            |
| --------------------------- | --------------------------------------- | ------------------ |
| `listen`                    | ip and/or port to listen on             | `0.0.0.0:80`       |
| `server_name`               | server name                             | `localhost`        |
| `root`                      | root directory                          | `/www`             |
| `index`                     | default index file                      | `/index.html`      |
| `client_max_body_size`      | maximum body size                       | `1024m`            |
| `client_body_buffer_size`   | body buffer size                        | `1024k`            |
| `client_header_buffer_size` | header buffer size                      | `1024k`            |
| `request_timeout`           | request timeout                         | `10m`              |
| `error_page`                | custom error page (`<code> <filepath>`) | `404 /404.html`    |
| `location`                  | location block                          | `location / {...}` |

### Location/Route Options

Specified within a `location` block. The server will use the closest matching location block.

| directive       | description                                            | example             |
| --------------- | ------------------------------------------------------ | ------------------- |
| `allow_methods` | allowed methods                                        | `GET POST DELETE`   |
| `autoindex`     | enable autoindex                                       | `on`                |
| `cgi`           | cgi script (`<ext> <path>`)                            | `.php /usr/bin/php` |
| `upload_dir`    | upload directory (by setting this uploads are enabled) | `/uploads`          |
| `root`          | root directory                                         | `/www`              |
| `index`         | default index file                                     | `/index.html`       |
| `return`        | only for redirect (`<status> <location>`)              | `301 /new`          |

#### Redirect Location Example

```nginx
location /youtube {
	return 301 https://www.youtube.com;
}
```

## Authors

This project was written as part of the 42 cursus at [42 Heilbronn](https://www.42heilbronn.de/) by:

- [Jonhannes Moritz](https://github.com/jojomo96)
- [Louen Gréau](https://github.com/GREAULouen)
- [Florian Fischer](https://github.com/flomero)
