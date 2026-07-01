#include "handlers/AutoIndexHandler.hpp"

#include <dirent.h>

AutoIndexHandler::AutoIndexHandler() {
}

AutoIndexHandler::~AutoIndexHandler() {
}

static std::string get_time() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    std::string time_str;
    static int old_sec = 0;
    static unsigned long long n_call = 0;
    t->tm_year += 1900;
    t->tm_mon += 1;

    StringHandler str_hdlr(time_str);
    str_hdlr += t->tm_year;
    if (t->tm_mon < 10)
        str_hdlr += "0";
    str_hdlr += t->tm_mon;
    if (t->tm_mday < 10)
        str_hdlr += "0";
    str_hdlr += t->tm_mday;
    str_hdlr += "-";
    if (t->tm_hour < 10)
        str_hdlr += "0";
    str_hdlr += t->tm_hour;
    str_hdlr += "-";
    if (t->tm_min < 10)
        str_hdlr += "0";
    str_hdlr += t->tm_min;
    str_hdlr += "-";
    if (t->tm_sec < 10)
        str_hdlr += "0";
    str_hdlr += t->tm_sec;

    if (old_sec != t->tm_sec) {
        n_call = 0;
        old_sec = t->tm_sec;
    }

    str_hdlr += "_";
    n_call++;
    if (n_call < 10)
        str_hdlr += "0";
    str_hdlr += n_call;

    return (time_str);
}

resource_context_t AutoIndexHandler::execute(request_context_t& context) {
    resource_context_t resource;
    std::string body_auto;
    DIR* dir = opendir(context.resolved_path.c_str());
    struct dirent* entry;
    std::string uri = context.resolved_path;
    uri.erase(0, context.root.size());

    if (dir == NULL) {
        throw std::runtime_error("Failed to open directory for autoindex");
    }

    resource.mime_type = "text/html";

    body_auto +=
        "<!DOCTYPE html>\n"
        "<html lang=\"fr\">\n"
        "<head>\n"
        "  <meta charset=\"UTF-8\">\n"
        "  <title>Index of " +
        uri +
        "</title>\n"
        "  <link rel=\"stylesheet\" href=\"/styles/autoindex.css\">\n"
        "</head>\n"
        "<body>\n"
        "  <div class=\"container\">\n"
        "    <h1>Index of " +
        uri +
        "</h1>\n"
        "    <p class=\"path\">Directory requested : " +
        uri + "</p>\n";

    if (context.limit_except.size() == 0 ||
        context.limit_except.find("PUT") == context.limit_except.end())
        body_auto +=
            "    <input type=\"file\" id=\"upload-file\">\n"
            "    <button id=\"upload-btn\">Upload</button>\n";

    body_auto += "    <ul class=\"listing\">\n";

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        std::string name = entry->d_name;
        std::string href = uri + name;

        if (std::string(entry->d_name) == "." ||
            std::string(entry->d_name) == "..")
            continue;

        if (entry->d_type == DT_DIR) {
            href += "/";
            body_auto += "      <li class=\"directory\"><a href=\"" + href +
                         "\">" + name + "/</a></li>\n";
        } else {
            body_auto += "      <li class=\"file\"><a href=\"" + href + "\">" +
                         name + "</a>\n";

            if (context.limit_except.size() == 0 ||
                context.limit_except.find("DELETE") ==
                    context.limit_except.end())
                body_auto += "      <button class=\"delete-btn\" data-path=\"" +
                             href + "\">Delete</button>\n";

            body_auto += "      </li>\n";
        }
    }

    body_auto +=
        "    </ul>\n"
        "  </div>\n"
        "  <script>\n"
        "    const buttons = document.querySelectorAll('.delete-btn');\n"
        "    const uploadBtn = document.getElementById('upload-btn');\n"
        "    const uploadInput = document.getElementById('upload-file');\n"
        "\n"
        "    for (let i = 0; i < buttons.length; i++) {\n"
        "      buttons[i].addEventListener('click', function (event) {\n"
        "        event.preventDefault();\n"
        "\n"
        "        const path = this.getAttribute('data-path');\n"
        "\n"
        "        fetch(path, { method: 'DELETE' })\n"
        "          .then(function (response) {\n"
        "            if (!response.ok)\n"
        "              throw new Error('Delete failed');\n"
        "\n"
        "            window.location.reload();\n"
        "          })\n"
        "          .catch(function () {\n"
        "            alert('Error during file deletion');\n"
        "          });\n"
        "      });\n"
        "    }\n"
        "\n"
        "    if (uploadBtn && uploadInput) {\n"
        "      uploadBtn.addEventListener('click', function (event) {\n"
        "        event.preventDefault();\n"
        "\n"
        "        if (!uploadInput.files || uploadInput.files.length  0) {\n"
        "          alert('No file selected');\n"
        "          return;\n"
        "        }\n"
        "\n"
        "        const file = uploadInput.files[0];\n"
        "        let base = window.location.pathname;\n"
        "\n"
        "        if (base[base.length - 1] !== '/')\n"
        "          base += '/';\n"
        "\n"
        "        fetch(base + encodeURIComponent(file.name), {\n"
        "          method: 'PUT',\n"
        "          body: file,\n"
        "          headers: {\n"
        "            'Content-Type': 'application/octet-stream'\n"
        "          }\n"
        "        })\n"
        "          .then(function (response) {\n"
        "            if (!response.ok)\n"
        "              throw new Error('Upload failed');\n"
        "\n"
        "            window.location.reload();\n"
        "          })\n"
        "          .catch(function () {\n"
        "            alert('Error during file upload');\n"
        "          });\n"
        "      });\n"
        "    }\n"
        "  </script>\n"
        "</body>\n"
        "</html>\n";

    std::string autoindex_file = context.root + "/tmp/" + get_time() + ".html";
    resource.file_tmp.fd =
        open(autoindex_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (resource.file_tmp.fd == -1) {
        closedir(dir);
        throw std::runtime_error("Failed to create autoindex file");
    }
    GarbageCollector::get_instance().add_fd(resource.file_tmp.fd);
    size_t n_bits = 0;
    while (n_bits < body_auto.size()) {
        ssize_t bits_send =
            write(resource.file_tmp.fd, body_auto.c_str() + n_bits,
                  body_auto.size() - n_bits);
        if (bits_send <= 0) {
            close(resource.file_tmp.fd);
            closedir(dir);
            throw std::runtime_error("Failed to write to autoindex file");
        }
        n_bits += static_cast<size_t>(bits_send);
    }
    lseek(resource.file_tmp.fd, 0, SEEK_SET);

    resource.file_tmp.size = body_auto.size();
    resource.file_tmp.name_file = autoindex_file;

    resource.http.status = E_HTTP_OK;
    resource.http.reason = const_cast<char*>(http_error_reason(E_HTTP_OK));

    closedir(dir);

    return resource;
}
