#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/model.h>
#include <learnopengl/shader_m.h>

#include <my/car.h>

#include <iostream>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

// 一些声明
GLFWwindow* windowInit();
bool init();
void setDeltaTime();
void renderLight(Shader shader);
void renderCar(Model model, Shader shader);
void renderRaceTrack(Model model, Shader shader);
void setTransMatrix(Shader shader, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projMatrix);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// 窗口尺寸
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Car car(glm::vec3(0.0f, 0.0f, 0.0f));

// camera
Camera camera(glm::vec3(0.0f, 50.0f, 0.0f));

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
    GLFWwindow* window = windowInit();
    bool isInit = init();
    // 一些初始化
    if (window == NULL || !isInit) {
        return -1;
    }

    // 构建和编译着色器
    Shader shader("shader/light.vs", "shader/light.fs");
    Shader simpleShader("shader/model_loading.vs", "shader/model_loading.fs");

    // 加载模型
    // Model model(FileSystem::getPath("asset/model/obj/nanosuit/nanosuit.obj"));
    // Model model(FileSystem::getPath("asset/model/obj/roomdoor/Door_Component_BI3.obj"));
    
    // Model model(FileSystem::getPath("asset/model/obj/simple-car/Car.obj"));
    // Model model(FileSystem::getPath("asset/model/obj/LowPolyCar/Car.obj"));
    
    // Model model(FileSystem::getPath("asset/model/obj/Residential Buildings/Residential Buildings 001.obj"));
    
    Model model(FileSystem::getPath("asset/model/obj/Lamborghini/Lamborghini.obj"));

    Model raceTrack(FileSystem::getPath("asset/model/obj/race-track/race-track.obj"));

    // -------
    // 循环渲染
    while (!glfwWindowShouldClose(window)) {
        // 计算一帧的时间长度以便于使帧绘制速度均匀
        setDeltaTime();

        // 监听按键
        processInput(window);

        // 渲染背景
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置uniform变量之前先应用shader
        shader.use();

        // 设置光照相关属性
        renderLight(shader);
        // 使用shader渲染car
        renderCar(model, shader);

        simpleShader.use();
        renderRaceTrack(raceTrack, simpleShader);

        // 交换缓冲区和调查IO事件（按下的按键,鼠标移动等）
        glfwSwapBuffers(window);

        // 轮询事件
        glfwPollEvents();
    }

    // 关闭glfw
    glfwTerminate();
    return 0;
}

// -----
// 初始化
GLFWwindow* windowInit()
{
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
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 令GLFW捕捉用户的鼠标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

bool init()
{
    // ------------------
    // 加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        system("pause");
        return false;
    }

    // 配置全局openGL状态
    glEnable(GL_DEPTH_TEST);

    // 画线框图
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    return true;
}

// 设置光照相关属性
void renderLight(Shader shader)
{
    // 方向和位置
    shader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("viewPos", camera.Position);

    // 光照属性
    shader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    shader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    shader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    // 材质属性
    shader.setFloat("shininess", 32.0f);
}

void renderCar(Model model, Shader shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, car.getPosition());
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90 + car.getYaw()), glm::vec3(0.0, 1.0, 0.0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.004f, 0.004f, 0.004f));
    // 投影转换
    glm::mat4 projMatrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // 应用变换矩阵
    setTransMatrix(shader, viewMatrix, modelMatrix, projMatrix);

    model.Draw(shader);
}

void renderRaceTrack(Model model, Shader shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    // 投影转换
    glm::mat4 projMatrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // 应用变换矩阵
    setTransMatrix(shader, viewMatrix, modelMatrix, projMatrix);

    model.Draw(shader);
}

void setTransMatrix(Shader shader, glm::mat4 viewMatrix, glm::mat4 modelMatrix, glm::mat4 projMatrix)
{
    shader.setMat4("view", viewMatrix);
    shader.setMat4("model", modelMatrix);
    shader.setMat4("projection", projMatrix);
}

// 计算一帧的时间长度
void setDeltaTime()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

// -------
// 监听按键
void processInput(GLFWwindow* window)
{
    // esc退出
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 相机 WSAD 前后左右 Space上 左Ctrl下
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    // 车车移动
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) 
        car.ProcessKeyboard(CAR_FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        car.ProcessKeyboard(CAR_BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        car.ProcessKeyboard(CAR_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        car.ProcessKeyboard(CAR_RIGHT, deltaTime);

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
