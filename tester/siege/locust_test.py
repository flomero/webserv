from locust import HttpUser, task, between

class WebServerUser(HttpUser):
    wait_time = between(1, 3)  # Simulates realistic user wait times

    @task
    def get_homepage(self):
        self.client.get("/")  # Test simple GET request to homepage

    @task
    def get_cgi_script(self):
        self.client.get("/cgi-bin/test.py")  # Test GET request to CGI script

    @task
    def post_cgi_script(self):
        self.client.post("/cgi-bin/test.py", data={"key": "value"})  # Test POST request to CGI script

