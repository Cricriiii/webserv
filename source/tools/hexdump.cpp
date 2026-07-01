#include "tools/hexdump.hpp"

void hexdump(const char* c_string) {
    size_t line_count = 0;
    const char* ptr = c_string;
    size_t cur_line_count;
    std::ostringstream oss;

    oss << '\n';

    while (*c_string) {
        ptr = c_string;
        cur_line_count = 0;

        oss << std::setw(LINE_NUM_WIDTH) << std::setfill('0') << std::dec
            << line_count << "  ";

        size_t i = 0;

        for (; i < HEXDUMP_WIDTH / CHAR_WIDTH; ++i) {
            if (*c_string) {
                oss << std::setw(CHAR_WIDTH) << std::setfill('0') << std::hex
                    << static_cast<unsigned char>(*c_string++) << SP;
            } else {
                oss << std::setw(CHAR_WIDTH + 1) << SP;
            }
        }

        oss << SP;

        for (; i < HEXDUMP_WIDTH; ++i) {
            if (*c_string) {
                oss << std::setw(CHAR_WIDTH) << std::setfill('0') << std::hex
                    << static_cast<unsigned char>(*c_string++) << SP;
            } else {
                oss << std::setw(CHAR_WIDTH + 1) << SP;
            }
        }

        oss << " |";

        for (i = 0; i < HEXDUMP_WIDTH; ++i) {
            if (*ptr) {
                char c;
                ::isprint(*ptr) ? c = *ptr : c = '.';
                oss << std::setw(1) << c;
                ++ptr;
                ++cur_line_count;
            } else {
                oss << std::setw(1) << SP;
            }
        }

        oss << "|\n";

        line_count += HEXDUMP_WIDTH;
    }

    std::cout << oss.str() << "────────\n"
              << "  " << line_count + cur_line_count << '\n'
              << "────────\n";
}

void hexdump(const char* c_string, size_t size) {
    const char* start = c_string;
    const char* end = c_string + size;

    std::ostringstream oss;

    oss << '\n';

    for (size_t n = 0; n < size; n += HEXDUMP_WIDTH) {
        const char* line_start = start + n;
        const char* p = line_start;

        oss << std::setw(LINE_NUM_WIDTH) << std::setfill('0') << std::dec << n
            << "  ";

        size_t i = 0;

        for (; i < HEXDUMP_WIDTH / 2; ++i) {
            if (p < end)
                oss << std::setw(CHAR_WIDTH) << std::setfill('0') << std::hex
                    << static_cast<int>(static_cast<unsigned char>(*p++)) << SP;
            else
                oss << std::setw(CHAR_WIDTH + 1) << SP;
        }

        oss << SP;

        for (; i < HEXDUMP_WIDTH; ++i) {
            if (p < end)
                oss << std::setw(CHAR_WIDTH) << std::setfill('0') << std::hex
                    << static_cast<int>(static_cast<unsigned char>(*p++)) << SP;
            else
                oss << std::setw(CHAR_WIDTH + 1) << SP;
        }

        oss << " |";

        p = line_start;
        for (i = 0; i < HEXDUMP_WIDTH; ++i) {
            if (p < end) {
                char c = ::isprint((unsigned char)*p) ? *p : '.';
                oss << c;
                ++p;
            } else {
                oss << SP;
            }
        }

        oss << "|\n";
    }

    std::cout << oss.str() << "────────\n"
              << "  " << size << '\n'
              << "────────\n";
}
