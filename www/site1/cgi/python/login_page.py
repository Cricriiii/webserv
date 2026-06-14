#!/usr/bin/env python

import os
import sys
import html


def read_file_content(filename):
    try:
        with open(filename, "r") as file:
            return file.read()
    except IOError:
        return """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Error</title>
</head>
<body>
    <h1>500 Internal Server Error</h1>
    <p>Could not open ./www/site1/cgi/python/login_request.html</p>
</body>
</html>
"""


def build_welcome_page(user_name):
    safe_user_name = html.escape(user_name)
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome</title>
    <link rel="stylesheet" href="/styles/index.css">
    <link rel="stylesheet" href="/styles/welcome.css">
</head>
<body>
    <div class="welcome-message">
        <h1>Hello {safe_user_name}</h1>

        <a href="/" class="button button-secondary back_home_button">
            ← Back to Home
        </a>
    </div>
</body>
</html>
"""


def send_response(content):
    content_length = len(content.encode("utf-8"))

    sys.stdout.write("Content-Type: text/html\r\n")
    sys.stdout.write(f"Content-Length: {content_length}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(content)


def main():
    user_name = os.environ.get("USER_NAME", "")

    if user_name.strip() == "":
        content = read_file_content("./www/site1/cgi/python/login_request.html")
    else:
        content = build_welcome_page(user_name)

    send_response(content)


if __name__ == "__main__":
    main()