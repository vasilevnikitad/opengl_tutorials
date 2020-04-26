#include "gl_wrappers.hpp"
#include "gl_helpers.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include <exception>
#include <iostream>

using namespace gl_wrappers;

#define GL_THROW_EXCEPTION_ON_ERROR(msg)                                                       \
do {                                                                                           \
    if(auto const err{glGetError()}; err != GL_NO_ERROR)                                       \
        throw std::runtime_error(""s + msg + ": returned error code "s + std::to_string(err)); \
} while(0);

template<typename T>
void main_loop(T&& window) {
    auto load_texture = [](auto const texture_id, auto&& filename) {
        auto get_color_model = [](unsigned channels_cnt) {
            switch (channels_cnt) {
                case 3:
                    std::cerr << __LINE__ << std::endl;
                    return GL_RGB;
                case 4:
                    std::cerr << __LINE__ << std::endl;
                    return GL_RGBA;
                default:
                    throw std::runtime_error("Cannot get color model from "s +
                                             std::to_string(channels_cnt) + " channels");
            }
        };

        auto [data, width, height, channels] =
                gl_helpers::get_data_from_image(std::forward<decltype(filename)>(filename));

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, get_color_model(channels), GL_UNSIGNED_BYTE, data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
    };
    struct coordinate3D { GLfloat x, y, z; };
    struct rgb_color { GLfloat r, g, b; };
    struct coordinate2D { GLfloat x, y; };

    struct vertex {
        coordinate3D pos;
        rgb_color color;
        coordinate2D texture_pos;
    };

    static const vertex vertices[] {
            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f, -0.5f}, {}, {1.0f, 0.0f}},
            {{+0.5f,  0.5f, -0.5f}, {}, {1.0f, 1.0f}},
            {{+0.5f,  0.5f, -0.5f}, {}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 0.0f}},

            {{-0.5f, -0.5f,  0.5f}, {}, {0.0f, 0.0f}},
            {{+0.5f, -0.5f,  0.5f}, {}, {1.0f, 0.0f}},
            {{+0.5f,  0.5f,  0.5f}, {}, {1.0f, 1.0f}},
            {{+0.5f,  0.5f,  0.5f}, {}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {}, {0.0f, 0.0f}},

            {{-0.5f,  0.5f,  0.5f}, {}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {}, {1.0f, 0.0f}},

            {{+0.5f,  0.5f,  0.5f}, {}, {1.0f, 0.0f}},
            {{+0.5f,  0.5f, -0.5f}, {}, {1.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{+0.5f, -0.5f,  0.5f}, {}, {0.0f, 0.0f}},
            {{+0.5f,  0.5f,  0.5f}, {}, {1.0f, 0.0f}},

            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},
            {{+0.5f, -0.5f, -0.5f}, {}, {1.0f, 1.0f}},
            {{+0.5f, -0.5f,  0.5f}, {}, {1.0f, 0.0f}},
            {{+0.5f, -0.5f,  0.5f}, {}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {}, {0.0f, 1.0f}},

            {{-0.5f,  0.5f, -0.5f}, {},  {0.0f, 1.0f}},
            {{+0.5f,  0.5f, -0.5f}, {},  {1.0f, 1.0f}},
            {{+0.5f,  0.5f,  0.5f}, {},  {1.0f, 0.0f}},
            {{+0.5f,  0.5f,  0.5f}, {},  {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {},  {0.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {},  {0.0f, 1.0f}}
    };

    static const glm::vec3 cube_positions[] {
            { 0.0f,  0.0f,  0.0f},
            { 2.0f,  5.0f, -15.0f},
            {-1.5f, -2.2f, -2.5f},
            {-3.8f, -2.0f, -12.3f},
            { 2.4f, -0.4f, -3.5f},
            {-1.7f,  3.0f, -7.5f},
            { 1.3f, -2.0f, -2.5f},
            { 1.5f,  2.0f, -2.5f},
            { 1.5f,  0.2f, -1.5f},
            {-1.3f,  1.0f, -1.5f}
    };

    GLuint vbo, vao, ebo, textures[2];

    glfw::set_context(std::forward<T>(window));

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenTextures(std::size(textures), textures);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    load_texture(textures[0], "textures/wall.jpg");
    load_texture(textures[1], "textures/awesomeface.png");


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void *>(offsetof(vertex, pos)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void *>(offsetof(vertex, color)));
    glEnableVertexAttribArray(1); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void *>(offsetof(vertex, texture_pos)));
    glEnableVertexAttribArray(2);


    shader_program program{vertex_shader{gl_helpers::get_text_from_file("shaders/simple.vert")},
                           fragment_shader{gl_helpers::get_text_from_file("shaders/color.frag")}};

    program.apply();
    program.set_uniform<GLint>(program.get_uniform_id("uniform_texture0"), 0);
    program.set_uniform<GLint>(program.get_uniform_id("uniform_texture1"), 1);

    auto const model_id{program.get_uniform_id("model")};
    auto const view_id{program.get_uniform_id("view")};
    auto const projection_id{program.get_uniform_id("projection")};

    auto get_model = [](float const phi, glm::vec3 const &pos) {
        float const angle(glfw::get_time() * glm::radians(-55.0f) + phi);
        return glm::rotate(glm::translate(glm::mat4{1.0f}, pos), angle, glm::vec3{1.f, 0.2f, 0.f});
    };

    auto get_view = []() {
        return glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -3.f});
    };

    auto get_projection = [window_ratio = window->get_width() / window->get_height()]() {
        return glm::perspective(glm::radians(45.f), static_cast<float>(window_ratio), 0.1f, 100.0f);
    };

    program.set_matrix_uniform<GLfloat, 4>(view_id, 1, glm::value_ptr(get_view()));
    program.set_matrix_uniform<GLfloat, 4>(projection_id, 1, glm::value_ptr(get_projection()));

    glEnable(GL_DEPTH_TEST);
    while(!window->should_be_closed()) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        program.apply();
        for (size_t i{0}; i < std::size(cube_positions); ++i) {
            std::cerr << i << std::endl;
            program.set_matrix_uniform<GLfloat, 4>(model_id, 1, glm::value_ptr(get_model(20.f * i, cube_positions[i])));
            glDrawArrays(GL_TRIANGLES, 0, std::size(vertices));
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);


        glfw::poll_events();
        window->swap_buffers();
    }
}

int main() try {
    main_loop(glfw::create_window("textures"));
    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
    return EXIT_FAILURE;
}

