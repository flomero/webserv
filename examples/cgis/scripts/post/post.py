import cgi
import json
import os

data_file = "data.json"

# Function to render HTML response
def render_html(message, link, link_text, is_error=False):
    return f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <script src="https://cdn.tailwindcss.com"></script>
        <title>Response</title>
    </head>
    <body class="bg-gray-900 text-white">
        <div class="container mx-auto p-6">
            <h1 class="text-3xl font-bold text-center { 'text-red-500' if is_error else 'text-green-500'}">{message}</h1>
            <div class="text-center mt-6">
                <a href="{link}" class="text-blue-500 hover:text-blue-600">{link_text}</a>
            </div>
        </div>
    </body>
    </html>
    """

# Parse form data
form = cgi.FieldStorage()
new_post = form.getvalue("new_post", "").strip()

if new_post:
    try:
        # Load existing posts
        if os.path.exists(data_file):
            with open(data_file, "r") as file:
                data = json.load(file)
                posts = data.get("posts", [])
        else:
            posts = []

        # Append the new post
        posts.append(new_post)

        # Save back to data.json
        with open(data_file, "w") as file:
            json.dump({"posts": posts}, file, indent=4)

        # Output success response
        print("Content-Type: text/html\n")
        print(render_html("Post added successfully!", "index.py", "Go back"))

    except Exception as e:
        print("Content-Type: text/html\n")
        print(render_html(f"Error: {e}", "index.py", "Go back", is_error=True))

else:
    print("Content-Type: text/html\n")
    print(render_html("No content provided!", "index.py", "Go back", is_error=True))
