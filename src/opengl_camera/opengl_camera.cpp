#include "gl_wrappers.hpp"
#include "gl_helpers.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

#include <exception>
#include <iostream>

using namespace gl_wrappers;

#define GL_THROW_EXCEPTION_ON_ERROR(msg)                                                       \
do {                                                                                           \
    if(auto const err{glGetError()}; err != GL_NO_ERROR)                                       \
        throw std::runtime_error(""s + msg + ": returned error code "s + std::to_string(err)); \
} while(0)

std::ostream &operator <<(std::ostream &o, glm::vec3 const &vec3) {
    return o << '(' << vec3.x << ", " << vec3.y << ", " << vec3.z << ')';
}
using radian = double;

class camera {
private:
    glm::vec3 position;
    radian pitch;
    radian yaw;
    radian roll;
    radian fov;
public:

    template<typename T>
    camera(T&& start_pos = {}, radian pitch = 0.f, radian yaw = 0.f, radian roll = 0.f, radian fov = M_PI_2 / 2)
            : position{std::forward<T>(start_pos)}, pitch{pitch}, yaw{yaw}, roll{roll}, fov{fov} {
    }

    radian get_fov() const {
        return fov;
    }

    void set_fov(radian value) {
        fov = value;
    }

    glm::vec3 get_pos_vec() const {
        return position;
    }

    void set_pos_vec(glm::vec3 const& pos) {
        position = pos;
    }

    void set_pos_vec(glm::vec3&& pos) {
        position = pos;
    }

    void set_pitch(radian value) {
        constexpr auto max{M_PI_2};
        constexpr auto min{-M_PI_2};
        if( min < value && value < max)
            pitch = value;
    }

    radian get_pitch() const {
        return pitch;
    }

    void set_yaw(radian value) {
        yaw = value;
    }

    radian get_yaw() const {
        return yaw;
    }

    void set_roll(radian value) {
        roll = value;
    }

    radian get_roll() const {
        return roll;
    }

    glm::vec3 get_direction_vec() const {
        auto const pitch_val{get_pitch()};
        auto const roll_val{get_roll()};
        auto const yaw_val{get_yaw()};

        auto const sin_pitch{std::sin(pitch_val)};
        auto const cos_pitch{std::cos(pitch_val)};
        auto const cos_roll{std::cos(roll_val)};
        auto const sin_roll{std::sin(roll_val)};
        auto const cos_yaw{std::cos(yaw_val)};
        auto const sin_yaw{std::sin(yaw_val)};

        glm::vec3&& vec{sin_pitch * sin_roll,
                        sin_pitch * cos_roll,
                        cos_pitch};

        return glm::vec3{vec.x * cos_yaw - vec.z * sin_yaw,
                         vec.y,
                         vec.x * sin_yaw + vec.z * cos_yaw};
    }

    glm::vec3 get_up_vec() const {
        auto const pitch_val{get_pitch()};
        auto const roll_val{get_roll()};
        auto const yaw_val{get_yaw()};

        auto const sin_pitch{std::sin(pitch_val)};
        auto const cos_pitch{std::cos(pitch_val)};
        auto const cos_roll{std::cos(roll_val)};
        auto const sin_roll{std::sin(roll_val)};
        auto const cos_yaw{std::cos(yaw_val)};
        auto const sin_yaw{std::sin(yaw_val)};

        glm::vec3&& vec{sin_roll,
                        cos_roll * cos_pitch,
                        -sin_roll * sin_pitch};

        return glm::vec3{vec.x * cos_yaw - vec.z * sin_yaw,
                         vec.y,
                         vec.x * sin_yaw + vec.z * cos_yaw};
    }

    void move_forward(float speed) {
        set_pos_vec(get_pos_vec() - speed * get_direction_vec());
    }

    void move_backward(float speed) {
        move_forward(-speed);
    }

    void move_to_right(float speed) {
        glm::vec3&& diff_vec{speed * glm::cross(get_up_vec(), get_direction_vec())};
        set_pos_vec(get_pos_vec() - diff_vec);
    }

    void move_to_left(float speed) {
        move_to_right(-speed);
    }

    class camera_scroll_callback : public glfw_scroll_callback {
    private:
        camera& cam;
    public:
        camera_scroll_callback(camera& cam) : cam{cam}{};

        virtual void operator() (glfw_window&, double, double y_off ) {
            constexpr auto max_fov{M_PI_2};
            constexpr auto min_fov{M_PI_2 / 180};

            auto fov_new{cam.get_fov() - glm::radians(y_off)};

            if (fov_new > max_fov)
                fov_new = max_fov;
            else if( fov_new < min_fov)
                fov_new = min_fov;

            cam.set_fov(fov_new);
        }
    } scroll_callback{*this};

    class camera_key_callback : public glfw_key_callback {
    private:
        camera& cam;
        static constexpr float speed{0.1f};
    public:
       camera_key_callback(camera& cam) : cam{cam}{};

       virtual void operator() (glfw_window& window, int key, [[maybe_unused]]int scan_code, int action, [[maybe_unused]]int mode)
       {
           switch (key) {
               case GLFW_KEY_ESCAPE:
                   if (action == GLFW_PRESS)
                       window.set_should_be_closed(true);
                   break;
               case GLFW_KEY_Q:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.set_roll(cam.get_roll() + 0.2f);
                   break;
               case GLFW_KEY_E:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.set_roll(cam.get_roll() - 0.2f);
                   break;
               case GLFW_KEY_W:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.move_forward(speed);
                   break;
               case GLFW_KEY_S:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.move_backward(speed);
                   break;
               case GLFW_KEY_A:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.move_to_right(speed);
                   break;

               case GLFW_KEY_D:
                   if (action == GLFW_PRESS || action == GLFW_REPEAT)
                       cam.move_to_left(speed);
                   break;

           }
       }
    } key_callback{*this};

    class camera_cursor_pos_callback : public glfw_cursor_pos_callback {
    private:
        camera& cam;

        double prev_x_pos{};
        double prev_y_pos{};
        bool call_once{true};

        static constexpr float sensitivity{0.005f};
    public:
        camera_cursor_pos_callback(camera& cam) : cam{cam}{};

        virtual void operator() (glfw_window&,  double x_pos, double y_pos)
        {
            if (call_once) {
                prev_x_pos = x_pos;
                prev_y_pos = y_pos;
                call_once = false;
            }

            double const x_off{sensitivity * (x_pos - prev_x_pos)};
            double const y_off{sensitivity * (prev_y_pos - y_pos)};

            cam.set_yaw(cam.get_yaw() + x_off);
            cam.set_pitch(cam.get_pitch() - y_off);

            prev_x_pos = x_pos;
            prev_y_pos = y_pos;
        }
    } cursor_pos_callback{*this};

    glm::mat4 get_view() const {
        auto const pos_vec{get_pos_vec()};

        return glm::lookAt(pos_vec, pos_vec - get_direction_vec(), get_up_vec());
    }
};

std::ostream &operator <<(std::ostream &o, camera const &cam) {
    return o << "pos = " << cam.get_pos_vec() <<
              "; direction = " << cam.get_direction_vec() <<
              "; up = " << cam.get_up_vec() <<
              "; roll = " << glm::degrees(cam.get_roll()) <<
              "; pitch = " << glm::degrees(cam.get_pitch()) <<
              "; yaw = " << glm::degrees(cam.get_yaw()) <<
              "; fov = " << glm::degrees(cam.get_fov());
}

template<typename T>
void main_loop(T&& window) {
    auto load_texture = [](auto const texture_id, auto&& filename) {
        auto get_color_model = [](unsigned channels_cnt) {
            switch (channels_cnt) {
                case 3:
                    return GL_RGB;
                case 4:
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
            { 5.0f,  0.0f,  0.0f},
            { 0.0f,  5.0f, 0.0f},
            {0.0f, 0.0f, 5.0f},
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

    camera main_cam{glm::vec3{0.f, 0.f, 10.f}, glm::radians(0.f), glm::radians(0.f), glm::radians(0.f)};

    struct callback_handler {
        glfw_window &window;
        glfw_key_callback        *prev_key_cb;
        glfw_cursor_pos_callback *prev_cursor_pos_cb;
        glfw_scroll_callback     *prev_scroll_cb;

        callback_handler(glfw_window &w, camera &cam)
                : window{w},
                  prev_key_cb{window.set_key_callback(&cam.key_callback)},
                  prev_cursor_pos_cb{window.set_cursor_pos_callback(&cam.cursor_pos_callback)},
                  prev_scroll_cb{window.set_scroll_callback(&cam.scroll_callback)}
        { }

        ~callback_handler() {
            window.set_key_callback(prev_key_cb);
            window.set_cursor_pos_callback(prev_cursor_pos_cb);
            window.set_scroll_callback(prev_scroll_cb);
        }
    } cb_handler{*window, main_cam};

    window->disable_cursor();

    auto get_projection = [window_ratio = window->get_width() / window->get_height()](radian fov) {
        return glm::perspective(static_cast<float>(fov), static_cast<float>(window_ratio), 0.1f, 100.0f);
    };

    glEnable(GL_DEPTH_TEST);
    while(!window->should_be_closed()) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        program.apply();
        program.set_matrix_uniform<GLfloat, 4>(projection_id, 1, glm::value_ptr(get_projection(main_cam.get_fov())));
        program.set_matrix_uniform<GLfloat, 4>(view_id, 1, glm::value_ptr(main_cam.get_view()));
        for (size_t i{0}; i < std::size(cube_positions); ++i) {
            program.set_matrix_uniform<GLfloat, 4>(model_id, 1, glm::value_ptr(get_model(20.f * i, cube_positions[i])));
            glDrawArrays(GL_TRIANGLES, 0, std::size(vertices));
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures[1]);

        std::cout << main_cam << std::endl;
        glfw::poll_events();
        window->swap_buffers();
    }
}

int main() try {
    main_loop(glfw::create_window("textures", 800, 800));
    return EXIT_SUCCESS;
} catch (std::exception const& e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
    return EXIT_FAILURE;
}

