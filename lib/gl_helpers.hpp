#ifndef GL_HELPERS__
#define GL_HELPERS__

#include <fstream>
#include <iterator>
#include <iostream>

namespace gl_helpers {
    template<typename T>
    inline std::string get_text_from_file(T&& filename) noexcept(false) {
        std::ifstream ifs{std::forward<T>(filename)};

        ifs.exceptions(std::ios_base::failbit|std::ios_base::badbit);

        return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    }
}
#endif
