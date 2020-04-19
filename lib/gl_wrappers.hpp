#ifndef GL_WRAPPERS__
#define GL_WRAPPERS__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>

using std::literals::string_literals::operator""s;
namespace gl_wrappers {

#define GL_THROW_EXCEPTION_ON_ERROR(msg)                                                       \
do {                                                                                           \
    if(auto const err{glGetError()}; err != GL_NO_ERROR)                                       \
        throw std::runtime_error(""s + msg + ": returned error code "s + std::to_string(err)); \
} while(0);

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

        template<typename T, typename = int>
        struct has_c_str : std::false_type{};

    public:
        template<typename STR>
        basic_shader(unsigned const shader_type, STR &&shader_code) :
                shader_type{shader_type}, shader_id{create_shader()} {
            compile_shader(shader_code.c_str());
        }

        basic_shader(basic_shader const& o) = delete;
        basic_shader& operator=(basic_shader const& o) = delete;

        basic_shader(basic_shader&& o) noexcept
                : shader_type{std::move(o.shader_type)},
                  shader_id{std::exchange(o.shader_id, GL_INVALID_INDEX)}
        { }

        basic_shader& operator=(basic_shader&& o) noexcept {
            shader_type = std::move(o.shader_type);
            shader_id = std::exchange(o.shader_id, GL_INVALID_INDEX);
            return *this;
        }

        [[nodiscard]]
        inline auto get_id() const noexcept {
            return shader_id;
        }

        [[nodiscard]]
        inline auto get_type() const noexcept {
            return shader_type;
        }

        ~basic_shader() {
            destroy_shader(shader_id);
        };
    };

    class vertex_shader : public basic_shader
    {
    public:
        template<typename T>
        inline vertex_shader(T &&shader_code) : basic_shader{GL_VERTEX_SHADER, std::forward<T>(shader_code)}
        { }
    };

    class fragment_shader : public basic_shader
    {
    public:
        template<typename T>
        inline fragment_shader(T &&shader_code) : basic_shader{GL_FRAGMENT_SHADER, std::forward<T>(shader_code)}
        { }
    };


    class shader_program {
    private:
        GLuint program_id;

        static inline auto create_program() noexcept(false) {
            auto const val{glCreateProgram()};

            if (val == 0)
                throw std::runtime_error("Error occurred in shader creation");

            return val;
        }

        inline void destroy_program() noexcept(true) {
            if( program_id != GL_INVALID_INDEX)
                glDeleteProgram(program_id);
        }

        template <typename T>
        using is_shader = typename std::is_base_of<basic_shader, std::remove_reference_t<T>>;

        template<typename T,
                typename = std::enable_if_t<is_shader<T>::value>>
        inline void attach_shader(T&& shader) {
            glAttachShader(program_id, shader.get_id());

            if(auto const err{glGetError()}; err != GL_NO_ERROR)
                throw std::runtime_error("Error attaching shader: GL returned code "s + std::to_string(err));
        }

        template<typename ...Args,
                typename = std::enable_if_t<std::conjunction_v<is_shader<Args&&>...>>>
        inline void compile_program(Args&&... args) noexcept(false) {

            (attach_shader(std::forward<Args>(args)), ...);

            glLinkProgram(program_id);

            if(GLint success; !(glGetProgramiv(program_id, GL_LINK_STATUS, &success), success)) {
                char msg[GL_INFO_LOG_LENGTH];
                glGetProgramInfoLog(program_id, std::size(msg), nullptr, msg);
                throw std::runtime_error("Error linking shader program: "s + msg);
            }
        }

        inline void use_program() noexcept(false) {
            glUseProgram(program_id);

            if(auto const err{glGetError()}; err != GL_NO_ERROR)
                throw std::runtime_error("Failed to use program: GL returned code "s + std::to_string(err));
        }

    public:
        template<typename ...Args,
                typename = std::enable_if_t<std::conjunction_v<is_shader<Args>...>>>
        shader_program(Args&&...args)
                : program_id{create_program()}  {
            try {
                compile_program(std::forward<Args>(args)...);
            } catch (std::exception const& e) {
                std::cerr << "Failed to create shader_program: "s + e.what() << std::endl;
                destroy_program();
                throw std::runtime_error{"Unable to construct shader_program"};
            }
        }

        shader_program() = delete;
        shader_program(shader_program const& o) = delete;
        shader_program& operator=(shader_program const& o) = delete;

        shader_program(shader_program&& o) noexcept : program_id{std::exchange(o.program_id, GL_INVALID_INDEX)}
        { }

        shader_program& operator=(shader_program&& o) noexcept {
            program_id = std::exchange(o.program_id, GL_INVALID_INDEX);
            return *this;
        }

        [[nodiscard]]
        auto get_id() {
            return program_id;
        }

        void apply() {
            use_program();
        }

        ~shader_program() {
            destroy_program();
        }

        template<typename T>
        struct helper {
            using value_type = T;
        };

        GLuint get_uniform_id(std::string const& name) noexcept(false)
        {
            auto const uniform_id{glGetUniformLocation(program_id, name.c_str())};

            if (uniform_id == -1)
                throw std::runtime_error("Cannot find \"" + name +  "\" uniform");

            return uniform_id;
        }

        template<typename T,
                 typename... Args,
                 typename = std::enable_if_t<std::conjunction_v<std::is_same<T, Args>...>>>
        void set_uniform(GLuint const uniform_id, Args&& ...args) {
            constexpr std::size_t args_cnt{sizeof...(args)};

            static_assert(args_cnt < 5, "Unable to set more then 4 parameters to uniform");

#define CHOOSE_UNIFORM_FUNC(args_cnt, last_letter)            \
        do {                                                  \
            if constexpr (args_cnt == 1)                      \
                glUniform1##last_letter(uniform_id, args...); \
            else if constexpr (args_cnt == 2)                 \
                glUniform2##last_letter(uniform_id, args...); \
            else if constexpr (args_cnt == 3)                 \
                glUniform3##last_letter(uniform_id, args...); \
            else if constexpr (args_cnt == 4)                 \
                glUniform4##last_letter(uniform_id, args...); \
        } while(0)

        if constexpr (std::is_same_v<T, GLfloat>)
            CHOOSE_UNIFORM_FUNC(args_cnt, f);
        else if constexpr (std::is_same_v<T, GLboolean>)
            CHOOSE_UNIFORM_FUNC(args_cnt, b);
        else if constexpr (std::is_same_v<T, GLint>)
            CHOOSE_UNIFORM_FUNC(args_cnt, i);
#undef CHOOSE_UNIFORM_FUNC

            GL_THROW_EXCEPTION_ON_ERROR("Failed to set uniform");
        }
    };

    class glfw_window {
    private:
        friend class glfw;
        GLFWwindow *ptr_window;

        glfw_window(GLFWwindow *ptr) : ptr_window{ptr}
        { }
    public:
        glfw_window() = delete;
        glfw_window(glfw_window const&) = delete;
        glfw_window& operator=(glfw_window const&) = delete;


        glfw_window(glfw_window&& o) : ptr_window{std::exchange(o.ptr_window, nullptr)}
        { }

        glfw_window& operator=(glfw_window&& o)
        {
            ptr_window = std::exchange(o.ptr_window, nullptr);
            return *this;
        }

        void swap_buffers() {
            glfwSwapBuffers(ptr_window);
        }

        void set_should_be_closed(bool const val)
        {
            glfwSetWindowShouldClose(ptr_window, val);
        }

        bool should_be_closed()
        {
            return glfwWindowShouldClose(ptr_window);
        }

        ~glfw_window(){
            glfwDestroyWindow(ptr_window);
        }
    };

    using window_shared_ptr_t = std::shared_ptr<glfw_window>;
    using window_weak_ptr_t = std::weak_ptr<glfw_window>;

    class glfw {
    public:

        static inline std::string get_last_error() {
            return {internal.get_error_msg()};
        }

        [[nodiscard]]
        static window_shared_ptr_t create_window(std::string const &title, unsigned width = 800, unsigned height = 600) {
            GLFWwindow* const ptr{glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr)};

            if (!ptr)
                throw std::runtime_error("Failed to create window: " + get_last_error());

            ;

            return window_shared_ptr_t {new glfw_window{ptr}};
        }

        static void poll_events() {
            glfwPollEvents();
        }

        static void set_context(window_shared_ptr_t const& window) {
            context_window = window;
            glfwMakeContextCurrent(context_window->ptr_window);

            static glew_lib glew{};
        }

        static window_weak_ptr_t get_context() {
            return context_window;
        }

        static void reset_context() {
            context_window.reset();
            glfwMakeContextCurrent(nullptr);
        }
    private:
        struct glew_lib {
            glew_lib() {
                glewExperimental = GL_TRUE;

                if (auto const ret{glewInit()}; ret != GLEW_OK) {
                    throw std::runtime_error("Failed to init GLEW lib: "s +
                                             reinterpret_cast<char const *>(glewGetErrorString(ret)));
                }
            }
        };

        class glfw_internal {
        private:
            inline static thread_local std::string str_error;
            static void error_callback(int error, char const* const msg) {
                str_error = "Error " + std::to_string(error) + ": " + msg;
            }

        public:

            glfw_internal() {

                if (!glfwInit())
                    throw std::runtime_error("Failed to init GLFW lib");

                glfwSetErrorCallback(error_callback);

                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
            }

            static std::string get_error_msg() {
                return str_error;
            }

            ~glfw_internal() {
                glfwTerminate();
            }
        };

        inline static glfw_internal internal{};
        inline static thread_local window_shared_ptr_t context_window{nullptr};
    };

#undef GL_THROW_EXCEPTION_ON_ERROR
}
#endif
