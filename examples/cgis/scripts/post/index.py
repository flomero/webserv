import json

def render_html(posts):
    posts_html = "".join(f"<li class='bg-gray-800 text-white p-4 rounded-lg my-2'>{post}</li>" for post in posts)
    html = f"""
    <!DOCTYPE html>
    <html lang="en" class="dark">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Posts</title>
        <script src="https://cdn.tailwindcss.com"></script>
    </head>
    <body class="bg-gray-900 text-white">
        <div class="container mx-auto p-6">
            <h1 class="text-3xl font-bold mb-6">Posts</h1>
            <ul class="list-none">
                {posts_html}
            </ul>
            <form action="post.py" method="post" enctype="application/x-www-form-urlencoded" class="mt-6">
                <label for="new_post" class="block text-xl mb-2">New Post:</label>
                <textarea id="new_post" name="new_post" rows="4" class="w-full p-3 rounded-lg bg-gray-700 text-white" placeholder="Write your post here..."></textarea><br><br>
                <button type="submit" class="bg-blue-500 text-white px-6 py-3 rounded-lg hover:bg-blue-600 transition">Submit</button>
            </form>
        </div>
    </body>
    </html>
    """
    return html

# Load posts from data.json
try:
    with open("data.json", "r") as file:
        data = json.load(file)
        posts = data.get("posts", [])
except (FileNotFoundError, json.JSONDecodeError):
    posts = []

# Output the HTML
print("Content-Type: text/html\n")
print(render_html(posts))
