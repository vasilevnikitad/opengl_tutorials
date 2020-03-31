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

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
