#ifndef GL_WRAPPERS__
#define GL_WRAPPERS__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <utility>

using std::literals::string_literals::operator""s;
namespace gl_wrappers {

#define GL_THROW_EXCEPTION_ON_ERROR(msg)                                                       \
do {                                                                                           \
    if(auto const err{glGetError()}; err != GL_NO_ERROR)                                       \
        throw std::runtime_error(""s + msg + ": returned error code "s + std::to_string(err)); \
} while(0)

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

        template<typename T,
                std::size_t COLUMNS,
                std::size_t ROWS = COLUMNS>
        void set_matrix_uniform(GLuint const uniform_id, GLsizei count, T const* ptr, GLboolean transpose = GL_FALSE) {

#define CHOOSE_MAT_UNIFORM_FUNC(cols, rows, last_letter, ...)     \
        do {                                                      \
            if constexpr (cols == rows) {                         \
                if constexpr (cols == 2)                          \
                    glUniformMatrix2##last_letter(__VA_ARGS__);   \
                else if constexpr (cols == 3)                     \
                    glUniformMatrix3##last_letter(__VA_ARGS__);   \
                else if constexpr (cols == 4)                     \
                    glUniformMatrix4##last_letter(__VA_ARGS__);   \
            }                                                     \
        } while(0)

            if constexpr (std::is_same_v<T, GLfloat>) {
                CHOOSE_MAT_UNIFORM_FUNC(COLUMNS, ROWS, fv, uniform_id, count, transpose, ptr);
            }
#undef CHOOSE_MAT_UNIFORM_FUNC

            GL_THROW_EXCEPTION_ON_ERROR("Failed to set uniform");
        }
    };

    class glfw_window;
    class glfw_key_callback {
    public:
        virtual void operator()(glfw_window& window,  int key, int scan_code, int action, int mode) = 0;
        virtual ~glfw_key_callback() = default;
    };

    class glfw_window {
    private:
        using key_callback_wrapper_fn = GLFWkeyfun;
        friend class glfw;
        std::unique_ptr<GLFWwindow, decltype(glfwDestroyWindow)*> ptr_window;

        // Some kind of workaround to make callback wrapper
        static inline std::map<GLFWwindow*, std::pair<glfw_window*, glfw_key_callback*>> key_callback_map;


        static void key_callback_thunk(GLFWwindow* const ptr, int key, int scancode, int action, int mode);

        glfw_window(GLFWwindow *ptr) : ptr_window{ptr, glfwDestroyWindow}
        { }

    public:
        glfw_window() = delete;
        glfw_window(glfw_window const&) = delete;
        glfw_window(glfw_window&& o);
        glfw_window& operator=(glfw_window const&) = delete;
        glfw_window& operator=(glfw_window&& o);
        ~glfw_window();

        void swap_buffers();

        void set_should_be_closed(bool const val);

        glfw_key_callback* set_key_callback(glfw_key_callback*);

        unsigned get_width();

        unsigned get_height();

        bool should_be_closed();

    };

    using window_shared_ptr_t = std::shared_ptr<glfw_window>;
    using window_weak_ptr_t = std::weak_ptr<glfw_window>;

    class glfw {
    public:

        static inline std::string get_last_error();

        [[nodiscard]]
        static window_shared_ptr_t create_window(std::string const &title, unsigned width = 800, unsigned height = 600);

        static void poll_events();

        static double get_time();

        static void set_context(window_shared_ptr_t const& window);

        static window_weak_ptr_t get_context();

        static void reset_context();
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

    glfw_window::glfw_window(glfw_window &&o) : ptr_window{std::exchange(o.ptr_window, nullptr)}
    { }

    glfw_window &glfw_window::operator=(glfw_window &&o) {
        ptr_window = std::exchange(o.ptr_window, nullptr);
        return *this;
    }

    void glfw_window::swap_buffers() {
        glfwSwapBuffers(ptr_window.get());
    }

    void glfw_window::set_should_be_closed(const bool val) {
        glfwSetWindowShouldClose(ptr_window.get(), val);
    }

    glfw_window::~glfw_window() {
        key_callback_map.erase(ptr_window.get());
    }

    bool glfw_window::should_be_closed() {
        return glfwWindowShouldClose(ptr_window.get());
    }

    unsigned glfw_window::get_width() {
        int width;
        glfwGetWindowSize(ptr_window.get(), &width, nullptr);

        if (!width)
            throw std::runtime_error(glfw::get_last_error());

        return static_cast<unsigned>(width);
    }

    unsigned glfw_window::get_height() {
        int height;
        glfwGetWindowSize(ptr_window.get(), nullptr, &height);

        if (!height)
            throw std::runtime_error(glfw::get_last_error());

        return static_cast<unsigned>(height);
    }

    void glfw_window::key_callback_thunk(GLFWwindow* const ptr, int key, int scancode, int action, int mode) {
        auto &[obj, callback]{key_callback_map[ptr]};

        (callback)->operator()(*obj, key, scancode, action, mode);
    }

    glfw_key_callback* glfw_window::set_key_callback(glfw_key_callback* callback) {
        if (auto const map_it{key_callback_map.find(ptr_window.get())};
                map_it != std::end(key_callback_map)) {
            if (callback)
                std::swap(callback, std::get<1>(map_it->second));
            else {
                callback = std::get<1>(map_it->second);
                key_callback_map.erase(map_it);
            }
            return callback;
        }

        if (!callback)
            glfwSetKeyCallback(ptr_window.get(), nullptr);
        else {
            key_callback_map.insert({ptr_window.get(), std::make_pair(this, callback)});
            glfwSetKeyCallback(ptr_window.get(), key_callback_thunk);
        }
        return nullptr;
    }

    window_shared_ptr_t glfw::create_window(const std::string &title, unsigned int width, unsigned int height) {
        GLFWwindow* const ptr{glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr)};

        if (!ptr)
            throw std::runtime_error("Failed to create window: " + get_last_error());

        return window_shared_ptr_t {new glfw_window{ptr}};
    }

    std::string glfw::get_last_error() {
        return {internal.get_error_msg()};
    }

    void glfw::poll_events() {
        glfwPollEvents();
    }

    double glfw::get_time() {
        return glfwGetTime();
    }

    void glfw::set_context(const window_shared_ptr_t &window) {
        context_window = window;
        glfwMakeContextCurrent(context_window->ptr_window.get());

        static glew_lib glew{};
    }

    window_weak_ptr_t glfw::get_context() {
        return context_window;
    }

    void glfw::reset_context() {
        context_window.reset();
        glfwMakeContextCurrent(nullptr);
    }

#undef GL_THROW_EXCEPTION_ON_ERROR
}
#endif
