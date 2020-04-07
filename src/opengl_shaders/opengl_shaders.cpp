#include "gl_wrappers.hpp"
#include "gl_helpers.hpp"

#include <exception>
#include <iostream>

using namespace gl_wrappers;

template<typename T>
void main_loop(T&& window) {
    glfw::set_context(std::forward<T>(window));
    GLuint vbo, vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);


    struct vertex {
        GLfloat pos[3];
        GLfloat color[3];
    };
    static const vertex vertices[] = {
            vertex{{0.5f, -0.5, 0.0f},{1.0f, 0.0f, 0.0f}},
            vertex{{-0.5f, -0.5, 0.0f},{0.0f, 1.0f, 0.0f}},
            vertex{{0.0f, 0.5, 0.0f},{0.0f, 0.0f, 1.0f}},
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void *>(offsetof(vertex, pos)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void *>(offsetof(vertex, color)));
    glEnableVertexAttribArray(1);

    shader_program program{vertex_shader{gl_helpers::get_text_from_file("shaders/simple.vert")},
                           fragment_shader{gl_helpers::get_text_from_file("shaders/color.frag")}};


    GLuint uniform_color{program.get_uniform_id("color")};
    while(!window->should_be_closed()) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        program.apply();
        program.set_uniform<GLfloat>(uniform_color, 0.9f, 0.1f, 0.1f, 0.1f);
        glDrawArrays(GL_TRIANGLES, 0, std::size(vertices));

        glfw::poll_events();
        window->swap_buffers();
    }
}

int main() try {
    main_loop(glfw::create_window("shaders"));
    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
    return EXIT_FAILURE;
}

