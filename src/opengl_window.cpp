#include <GL/glew.h>
#include <unistd.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main(int, char *[])
{
    if (!glfwInit())
        return EXIT_FAILURE;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *const window = glfwCreateWindow(800, 800, "mywindow", nullptr, nullptr);

    if( !window ) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        char const *description;
        glfwGetError(&description);
        std::cout << description  << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (auto const ret{glewInit()}; ret != GLEW_OK) {
        std::cerr << "glewInit: Failed to init: " << glewGetErrorString(ret) << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    auto const key_callback = [](GLFWwindow *window, int key,
                                 [[maybe_unused]]int scancode, int action, [[maybe_unused]]int mode) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                if (action == GLFW_PRESS) {
                    std::cout << "Escape pressed!" << std::endl;
                    glfwSetWindowShouldClose(window, GL_TRUE);
                }
                break;
            default:
                break;
        }
    };

    glfwSetKeyCallback(window, key_callback);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
