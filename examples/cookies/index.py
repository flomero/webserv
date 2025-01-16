#!/usr/bin/env python3
import os
import http.cookies
import uuid
import json

session_db = {}

db_file = "session_db.json"
if os.path.exists(db_file):
    with open(db_file, "r") as f:
        session_db = json.load(f)

def save_session_db():
    with open(db_file, "w") as f:
        json.dump(session_db, f)

cookies = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE", ""))
session_id_cookie = cookies.get("session_id")
session_id = session_id_cookie.value if session_id_cookie else None

if session_id is None or session_id not in session_db:
    session_id = str(uuid.uuid4())
    session_db[session_id] = {"visits": 0}
    save_session_db()

session = session_db[session_id]
session["visits"] += 1
save_session_db()

new_cookie = http.cookies.SimpleCookie()
new_cookie["session_id"] = session_id
new_cookie["session_id"]["path"] = "/"
new_cookie["session_id"]["httponly"] = False # JavaScript can access the cookie
new_cookie["session_id"]["max-age"] = 3600

print("Content-Type: text/html")
print(new_cookie.output())
print()
print(f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🍪 cookies</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <script>
        function deleteCookie() {{
            document.cookie = 'session_id=; Max-Age=0; path=/; domain=' + location.hostname;
            location.reload();
        }}
    </script>
</head>
<body class="bg-gray-900 text-white min-h-screen flex flex-col justify-center items-center">
    <div class="bg-gray-800 shadow-lg rounded-lg p-8 text-center max-w-md">
        <h1 class="text-3xl font-bold mb-4 text-indigo-400">Welcome...</h1>
        <h2 class="text-2xl mb-4 text-indigo-400">have some cookies 🍪</h2>
        <p class="text-lg mb-2">Your session ID is:</p>
        <code class="bg-gray-700 text-indigo-200 rounded p-2 block mb-4">{session_id}</code>
        <p class="text-lg">Times you have visited this page:</p>
        <p class="text-4xl font-bold mb-4 text-indigo-400">{session["visits"]}</p>
        <div class="mt-6 flex justify-center gap-4">
            <button onclick="location.reload()" class="bg-indigo-500 hover:bg-indigo-600 text-white font-semibold py-2 px-4 rounded-lg shadow">
                Reload Page
            </button>
            <button onclick="deleteCookie()" class="bg-red-500 hover:bg-red-600 text-white font-semibold py-2 px-4 rounded-lg shadow">
                Delete Cookie
            </button>
        </div>
    </div>
</body>
</html>
""")
