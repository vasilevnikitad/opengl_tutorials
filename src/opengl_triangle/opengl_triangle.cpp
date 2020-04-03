#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

// TODO:
// 1. fix error/exception messages: it should be clear to reader what and where error occurred.

using std::literals::string_literals::operator""s;

class basic_shader {
private:
    GLuint shader_type;
    GLuint shader_id;

    inline auto create_shader() {
        auto const val{glCreateShader(shader_type)};

        switch (val) {
            case 0:
                throw std::runtime_error("Error occurred in shader creation");
            case GL_INVALID_ENUM:
                throw std::invalid_argument("Invalid Shader type");

            default:
                break;
        }

        return val;
    }

    static inline void destroy_shader(GLuint const id) noexcept(true) {
        if(id != GL_INVALID_INDEX )
            glDeleteShader(id);
    }

    inline void compile_shader(char const *c_str) {
        auto const id{shader_id};
        glShaderSource(id, 1, &c_str, nullptr);
        glCompileShader(id);

        if (GLint success; !(glGetShaderiv(id, GL_COMPILE_STATUS, &success), success)) {
            char msg[GL_INFO_LOG_LENGTH];
            glGetShaderInfoLog(id, std::size(msg), nullptr, msg);
            throw std::runtime_error{msg};
        }
    }

public:
    basic_shader(unsigned const shader_type, char const *const shader_code) :
            shader_type{shader_type}, shader_id{create_shader()} {
        try {
            compile_shader(shader_code);
        } catch(std::exception const &e) {
            std::cerr << "Failed to compile shader: " << e.what() << std::endl;
            destroy_shader(shader_id);
            throw std::runtime_error{"Unable to construct shader"};
        }
    }

    basic_shader(basic_shader const& o) = delete;
    basic_shader& operator=(basic_shader const& o) = delete;

    basic_shader(basic_shader&& o) noexcept
            : shader_type{std::move(o.shader_type)},
              shader_id{std::exchange(o.shader_id, GL_INVALID_INDEX)}
    { }


    basic_shader& operator=(basic_shader&& o) noexcept
    {
        shader_type = std::move(o.shader_type);
        shader_id = std::exchange(o.shader_id, GL_INVALID_INDEX);
        return *this;
    }

    [[nodiscard]]
    auto get_id() const noexcept {
        return shader_id;
    }

    [[nodiscard]]
    auto get_type() const noexcept {
        return shader_type;
    }

    ~basic_shader() {
        destroy_shader(shader_id);
    };
};

class vertex_shader : public basic_shader
{
public:
    explicit vertex_shader(char const *const shader_code) : basic_shader{GL_VERTEX_SHADER, shader_code}
    { }

    explicit vertex_shader(std::string const &shader_code) : basic_shader{GL_VERTEX_SHADER, shader_code.c_str()}
    { }

    explicit vertex_shader(std::string&& shader_code) : basic_shader{GL_VERTEX_SHADER, shader_code.c_str()}
    { }
};

class fragment_shader : public basic_shader
{
public:
    explicit fragment_shader(char const *const shader_code) : basic_shader{GL_FRAGMENT_SHADER, shader_code}
    {}

    explicit fragment_shader(std::string const &shader_code) : basic_shader{GL_FRAGMENT_SHADER, shader_code.c_str()}
    { }

    explicit fragment_shader(std::string&& shader_code) : basic_shader{GL_FRAGMENT_SHADER, shader_code.c_str()}
    { }
};


class shader_program {
private:
    GLuint program_id;

    static inline auto create_program() noexcept(false)
    {
        auto const val{glCreateProgram()};

        if (val == 0)
            throw std::runtime_error("Error occurred in shader creation");

        return val;
    }

    static void destroy_program(GLuint const id) noexcept(true)
    {
        glDeleteProgram(id);
    }

    inline void compile_program(std::initializer_list<basic_shader> const& init_list) noexcept(false) {

        for(auto const &shader : init_list) {
            glAttachShader(program_id, shader.get_id());

            if(auto const err{glGetError()}; err != GL_NO_ERROR)
                throw std::runtime_error("Error attaching shader: GL returned code "s + std::to_string(err));
        }

        glLinkProgram(program_id);

        if(GLint success; !(glGetProgramiv(program_id, GL_LINK_STATUS, &success), success)) {
            char msg[GL_INFO_LOG_LENGTH];
            glGetProgramInfoLog(program_id, std::size(msg), nullptr, msg);
            throw std::runtime_error("Error linking shader program: "s + msg);
        }
    }

    inline void use_program() noexcept(false)
    {
        glUseProgram(program_id);

        if(auto const err{glGetError()}; err != GL_NO_ERROR)
            throw std::runtime_error("Failed to use program: GL returned code "s + std::to_string(err));
    }

public:

    shader_program(std::initializer_list<basic_shader> const init_list)
            : program_id{create_program()}  {
        try {
            compile_program(init_list);
        } catch (std::exception const& e) {
            std::cerr << "Failed to create shader_program: "s + e.what() << std::endl;
            destroy_program(program_id);
            throw std::runtime_error{"Unable to construct shader_program"};
        }
    }

    shader_program(shader_program const& o) = delete;
    shader_program& operator=(shader_program const& o) = delete;

    shader_program(shader_program&& o) noexcept : program_id{std::exchange(o.program_id, GL_INVALID_INDEX)}
    { }


    shader_program& operator=(shader_program&& o) noexcept
    {
        program_id = std::exchange(o.program_id, GL_INVALID_INDEX);
        return *this;
    }

    [[nodiscard]]
    auto get_id()
    {
        return program_id;
    }

    void apply()
    {
        use_program();
    }

    ~shader_program() = default;
};

std::string get_text_from_file(std::string const &filename)
{
    std::ifstream ifs{filename};

    if (!ifs)
        throw std::runtime_error{"Cannot open file "s + filename};

    return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

#define GL_THROW_EXCEPTION_ON_ERROR(func, ...)                                             \
do {                                                                                       \
    func(__VA_ARGS__);                                                                     \
    if(auto const err{glGetError()}; err != GL_NO_ERROR)                                   \
        throw std::runtime_error(#func + ": returned error code "s + std::to_string(err)); \
} while(0);


static std::string get_glfw_err_msg() {
    char const *description;
    glfwGetError(&description);

    return description;
}

static void gl_init() noexcept(false) {
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW lib");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

static void gl_deinit() noexcept {
    glfwTerminate();
}

GLFWwindow *create_window(unsigned const width, unsigned const height) {
    GLFWwindow *const window{glfwCreateWindow(width, height, "first triangle", nullptr, nullptr)};

    if( !window )
        throw std::runtime_error("Cannot create window: "s + get_glfw_err_msg());

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *const, int width, int height){
        glViewport(0, 0, width, height);
    });

    return window;
}

static void main_cycle(unsigned const width, unsigned height) {
    auto window{create_window(width, height)};

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (auto const ret{glewInit()}; ret != GLEW_OK) {
        throw std::runtime_error("Failed to init GLEW lib: "s +
                                 reinterpret_cast<char const *>(glewGetErrorString(ret)));
    }

    GLuint vao;
    GLuint vbo;
    GLfloat vertices[] {
            -0.5f, -0.5f, 0.0f,
            +0.5f, -0.5f, 0.0f,
             0.0f, +0.5f, 0.0f,
    };


    GL_THROW_EXCEPTION_ON_ERROR(glGenVertexArrays, 1, &vao);
    GL_THROW_EXCEPTION_ON_ERROR(glGenBuffers, 1, &vbo);

    GL_THROW_EXCEPTION_ON_ERROR(glBindVertexArray, vao);
    GL_THROW_EXCEPTION_ON_ERROR(glBindBuffer, GL_ARRAY_BUFFER, vbo);
    GL_THROW_EXCEPTION_ON_ERROR(glBufferData, GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GL_THROW_EXCEPTION_ON_ERROR(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    GL_THROW_EXCEPTION_ON_ERROR(glEnableVertexAttribArray, 0);

    GL_THROW_EXCEPTION_ON_ERROR(glBindBuffer, GL_ARRAY_BUFFER, 0);
    GL_THROW_EXCEPTION_ON_ERROR(glBindVertexArray, 0);


    shader_program program{vertex_shader{get_text_from_file("shaders/triangle.vert")},
                           fragment_shader{get_text_from_file("shaders/yellow.frag")}};

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        program.apply();
        GL_THROW_EXCEPTION_ON_ERROR(glBindVertexArray, vao);
        GL_THROW_EXCEPTION_ON_ERROR(glDrawArrays, GL_TRIANGLES, 0, 3);
        GL_THROW_EXCEPTION_ON_ERROR(glBindVertexArray, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int, char *[]) try {
    constexpr unsigned width{800};
    constexpr unsigned height{800};
    gl_init();

    try {
        main_cycle(width, height);
    } catch (std::exception const &e) {
        std::cerr << "exception in main cycle: " << e.what() << std::endl;
        gl_deinit();
        throw std::runtime_error{"Unable to continue work"};
    }

    gl_deinit();

    return EXIT_SUCCESS;
} catch(std::exception const& e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
} catch(...) {
    std::cerr << "Undefined exception has been caught: " << std::endl;
}
