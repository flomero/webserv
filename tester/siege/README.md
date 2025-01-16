# Locust Web Server Stress Test

## 1. Start the Web Server

Navigate to the webserv/ directory and run:

```
make
./webserv config/siege.conf
```

This will start your web server using the specified configuration file.

## 2. Start Locust Tester

Navigate to the tester/siege/ directory and run:

```
locust -f locust_test.py

This starts the Locust test script.
```

## 3. Configure and Start the Test

Open a web browser and go to:

http://localhost:8089
