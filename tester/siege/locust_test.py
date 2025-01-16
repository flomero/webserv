from locust import HttpUser, task, between

class MyUser(HttpUser):
    wait_time = between(1, 3)  # Users wait between 1 to 3 seconds between tasks

    @task
    def index_page(self):
        self.client.get("/")  # Simulate users visiting the homepage