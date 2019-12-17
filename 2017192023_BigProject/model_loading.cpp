#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>

#include <iostream>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

// 一些声明
void init();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// 窗口尺寸
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const vec4 BG_COLOR(0.1f, 0.1f, 0.1f, 1.0f);

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));

// 将鼠标设置在屏幕中心
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing 用来平衡不同电脑渲染水平所产生的速度变化
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --------
// main 函数
int main()
{
    // 一些初始化
    init();

    // 构建和编译着色器
    Shader ourShader("shader/model_loading.vs", "shader/model_loading.fs");

    // 加载模型
    // Model ourModel(FileSystem::getPath("asset/model/obj/nanosuit/nanosuit.obj"));
    // Model ourModel(FileSystem::getPath("asset/model/obj/roomdoor/Door_Component_BI3.obj"));
    Model ourModel(FileSystem::getPath("asset/model/obj/Residential Buildings/Residential Buildings 001.obj"));
    // Model ourModel(FileSystem::getPath("asset/model/obj/Residential Buildings/3d-Hologramm-(Wavefront OBJ).obj"));

    // -------
    // 循环渲染
    while (!glfwWindowShouldClose(window)) {
        // 每帧的时间逻辑
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 监听按键
        processInput(window);

        // 渲染
        glClearColor(BG_COLOR);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置uniform变量之前先应用shader
        ourShader.use();

        // 视图/投影转换
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // 渲染加载的模型
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // 将其下移以位于scene的中心
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f)); // 缩小
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // ----------------------------------------
        // 交换缓冲区和调查IO事件（按下的按键,鼠标移动等）
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 关闭glfw
    glfwTerminate();
    return 0;
}

// -----
// 初始化
void init() {
    // ---------
    // 初始化配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // -------
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, u8"2017191023_陈扬锐_期末大作业", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        system("pause");
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 令GLFW捕捉用户的鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --------------------
    // 加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        system("pause");
        return -1;
    }

    // 配置全局openGL状态
    glEnable(GL_DEPTH_TEST);

    // 画线框图
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

// -------
// 监听按键
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
// -------
// 鼠标移动
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 坐标翻转以对应坐标系

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// -------
// 鼠标滚轮
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// ----------
// 改变窗口大小
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 确保窗口匹配的新窗口尺寸
    // 宽度和高度将明显大于指定在retina屏上显示
    glViewport(0, 0, width, height);
}
