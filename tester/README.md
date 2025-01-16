# Webserv Tester

## Create and use Python venv:

In `webserv/tester`:

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

## Launch the webserver with the tester.conf file:

In webserv/:

```bash
make
./webserv tester/tester.conf
```

## Finally execute the tester.py script:

In webserv/tester:

```bash
python3 tester.py
```
