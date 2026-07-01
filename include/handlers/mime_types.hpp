#ifndef MIME_TYPES_HPP
#define MIME_TYPES_HPP

#include <string>

typedef struct {
    const char* extension;
    const char* mime_type;
} mime_type_t;

static const mime_type_t g_mime_type[] = {
    {".html", "text/html"},        {".htm", "text/html"},
    {".css", "text/css"},          {".js", "application/javascript"},
    {".json", "application/json"}, {".txt", "text/plain"},
    {".png", "image/png"},         {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},       {".gif", "image/gif"},
    {".svg", "image/svg+xml"},     {".ico", "image/x-icon"},
    {".pdf", "application/pdf"},   {".zip", "application/zip"},
    {".tar", "application/x-tar"}, {".xml", "application/xml"},
    {".csv", "text/csv"},          {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},         {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},         {".avi", "video/x-msvideo"},
    {".mpeg", "video/mpeg"},       {".webm", "video/webm"},
    {".ogg", "audio/ogg"},         {".wav", "audio/wav"}};

inline std::string get_extension(const std::string& mime) {
    size_t size_list = sizeof(g_mime_type) / sizeof(*g_mime_type);

    for (size_t i = 0; i < size_list; ++i) {
        if (mime == g_mime_type[i].mime_type)
            return g_mime_type[i].extension;
    }
    return "";
}

static std::string file_extension(const std::string& file) {
    size_t pos = file.find_last_of('.');
    if (pos == std::string::npos)
        return "";
    return file.substr(pos);
}

inline const std::string get_mime_type(const std::string& str) {
    std::string ext = file_extension(str);
    size_t size_list = sizeof(g_mime_type) / sizeof(*g_mime_type);

    for (size_t i = 0; i < size_list; ++i) {
        if (ext == g_mime_type[i].extension)
            return g_mime_type[i].mime_type;
    }
    return "";
}

#endif