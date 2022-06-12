#ifndef CRASY_EXAMPLES_HELPERS_HPP
#define CRASY_EXAMPLES_HELPERS_HPP

namespace {

template <typename T>
crasy::option<T> check_result(crasy::result<T> res) {
    if (res.is_err()) {
        auto err = res.err().code();
        std::cerr << err.message() << " (" << err.value() << ")\n";
        return crasy::nullopt;
    }
    return res.ok();
}

inline bool check_result(crasy::result<void> res) {
    if (res.is_err()) {
        auto err = res.err().code();
        std::cerr << err.message() << " (" << err.value() << ")\n";
        return false;
    }
    return true;
}

inline void print_data(std::span<const std::byte> data) {
    std::cout << '"';
    for (auto byte : data) {
        auto c = static_cast<char>(byte);
        if (std::isprint(c)) {
            std::cout << c;
            continue;
        }
        switch (c) {
            case '\a':
                std::cout << "\\a";
                break;
            case '\b':
                std::cout << "\\b";
                break;
            case '\n':
                std::cout << "\\n";
                break;
            case '\\':
                std::cout << "\\\\";
                break;
            case '"':
                std::cout << "\\\"";
                break;
            case '\r':
                std::cout << "\\r";
                break;
            case '\t':
                std::cout << "\\t";
                break;
            case '\v':
                std::cout << "\\v";
                break;
            case '\f':
                std::cout << "\\f";
                break;
            default: {
                auto val = static_cast<unsigned char>(byte);
                std::cout << "\\x";
                auto upper = val >> 4;
                auto lower = val & 0xF;
                if (upper < 10) {
                    std::cout << static_cast<char>('0' + upper);
                } else {
                    std::cout << static_cast<char>('a' + (upper - 10));
                }
                if (lower < 10) {
                    std::cout << static_cast<char>('0' + lower);
                } else {
                    std::cout << static_cast<char>('a' + (lower - 10));
                }
            } break;
        }
    }
    std::cout << '"' << std::endl;
}

}

#endif
