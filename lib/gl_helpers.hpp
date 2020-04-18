#ifndef GL_HELPERS__
#define GL_HELPERS__

#define STB_IMAGE_IMPLEMENTATION
#include "3dparty/stb_image.h"

#include <fstream>
#include <iterator>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

namespace gl_helpers {

    using std::literals::string_literals::operator""s;

    template<typename T>
    inline std::string get_text_from_file(T&& filename) noexcept(false) {
        std::ifstream ifs{std::forward<T>(filename)};

        ifs.exceptions(std::ios_base::failbit|std::ios_base::badbit);

        return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    }


    //< TODO: not the best function. Should be deprecated.
    template<typename T>
    inline std::tuple<std::vector<std::uint8_t>, unsigned, unsigned, unsigned>
    get_data_from_image(T&& filename) noexcept(false) {
        int width, height, channels;

        auto free_buffer = [](std::uint8_t* ptr) { stbi_image_free(ptr); };
        std::unique_ptr<std::uint8_t, decltype(free_buffer)>
                buffer(stbi_load(filename, &width, &height, &channels, 0), free_buffer);

        if (!buffer) {
            throw std::runtime_error("Failed to read image file"s + filename + ": " + stbi_failure_reason());
        }

        std::size_t buffer_sz(width * height * channels);

        return {{buffer.get(), buffer.get() + buffer_sz}, static_cast<unsigned>(width),
                                                          static_cast<unsigned>(height),
                                                          static_cast<unsigned>(channels)};
    }

}
#endif
