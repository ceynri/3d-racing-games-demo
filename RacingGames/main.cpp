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
#include <my/fixed_camera.h>

#include <iostream>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "assimp.lib")

// ------------------------------------------
// 函数声明
// ------------------------------------------

GLFWwindow* windowInit();
bool init();
void depthMapFBOInit();
void skyboxInit();

void setDeltaTime();
void changeLightPosAsTime();
void updateFixedCamera();

// 使用“&”引用性能更好
void renderLight(Shader& shader);
void renderCarAndCamera(Model& carModel, Model& cameraModel, Shader& shader);
void renderCar(Model& model, glm::mat4 modelMatrix, Shader& shader);
void renderCamera(Model& model, glm::mat4 modelMatrix, Shader& shader);
void renderStopSign(Model& model, Shader& shader);
void renderRaceTrack(Model& model, Shader& shader);
void renderSkyBox(Shader& shader);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void handleKeyInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

// ------------------------------------------
// 全局变量
// ------------------------------------------

// 窗口尺寸
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 渲染阴影时的窗口分辨率（会影响阴影的锯齿边情况）
const unsigned int SHADOW_WIDTH = 1024 * 10;
const unsigned int SHADOW_HEIGHT = 1024 * 10;

// 是否为线框图模式
bool isPolygonMode = false;

// 世界坐标系Y轴单位向量
glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);

// 汽车
Car car(glm::vec3(0.0f, 0.05f, 0.0f));

// 相机
glm::vec3 cameraPos(0.0f, 2.0f, 5.0f);
Camera camera(cameraPos);
FixedCamera fixedCamera(cameraPos);
bool isCameraFixed = false;

// 光照相关属性
glm::vec3 lightPos(-1.0f, 1.0f, -1.0f);
glm::vec3 lightDirection = glm::normalize(lightPos);
glm::mat4 lightSpaceMatrix;

// 深度Map的ID
unsigned int depthMap;
unsigned int depthMapFBO;

// 将鼠标设置在屏幕中心
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing 用来平衡不同电脑渲染水平所产生的速度变化
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 天空盒
unsigned int cubemapTexture;
unsigned int skyboxVAO, skyboxVBO;

// 天空盒顶点数据
const float skyboxVertices[] = {
    // positions
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f
};

// 天空盒的面数据
const vector<std::string> faces{
    FileSystem::getPath("asset/textures/skybox/right.tga"),
    FileSystem::getPath("asset/textures/skybox/left.tga"),
    FileSystem::getPath("asset/textures/skybox/top.tga"),
    FileSystem::getPath("asset/textures/skybox/bottom.tga"),
    FileSystem::getPath("asset/textures/skybox/front.tga"),
    FileSystem::getPath("asset/textures/skybox/back.tga")
};

// ------------------------------------------
// main函数
// ------------------------------------------

int main()
{
    // ------------------------------
    // 初始化
    // ------------------------------

    // 窗口初始化
    GLFWwindow* window = windowInit();
    // OpenGL初始化
    bool isInit = init();
    if (window == NULL || !isInit) {
        return -1;
    }
    // 深度Map的FBO配置
    depthMapFBOInit();
    // 天空盒的配置
    skyboxInit();

    // ------------------------------
    // 构建和编译着色器
    // ------------------------------

    // 为所有物体添加光照和阴影的shader
    Shader shader("shader/light_and_shadow.vs", "shader/light_and_shadow.fs");
    // 从太阳平行光角度生成深度信息的shader
    Shader depthShader("shader/shadow_mapping_depth.vs", "shader/shadow_mapping_depth.fs");
    // 天空盒shader
    Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");

    // ------------------------------
    // 模型加载
    // ------------------------------

    // 汽车模型
    Model carModel(FileSystem::getPath("asset/models/obj/Lamborghini/Lamborghini.obj"));
    // 相机模型
    Model cameraModel(FileSystem::getPath("asset/models/obj/camera-cube/camera-cube.obj"));
    // 赛道模型
    Model raceTrackModel(FileSystem::getPath("asset/models/obj/race-track/race-track.obj"));
    // STOP牌模型
    Model stopSignModel(FileSystem::getPath("asset/models/obj/StopSign/StopSign.obj"));

    // ---------------------------------
    // shader 纹理配置
    // ---------------------------------

    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("shadowMap", 15); // 这里的15是指"GL_TEXTURE15"，需要与后面的对应

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // ---------------------------------
    // 循环渲染
    // ---------------------------------

    while (!glfwWindowShouldClose(window)) {
        // 计算一帧的时间长度以便于使帧绘制速度均匀
        setDeltaTime();

        // 随着时间改变光源位置
        // changeLightPosAsTime();

        // 监听按键
        handleKeyInput(window);

        // 渲染背景
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---------------------------------
        // 渲染获得场景的深度信息
        // ---------------------------------

        // 定义光源视见体，即阴影生成范围的正交投影矩阵
        glm::mat4 lightProjection = glm::ortho(
            -200.0f, 200.0f,
            -200.0f, 200.0f,
            -200.0f, 200.0f);
        // TODO lightPos跟随相机位置进行移动，使相机周围的地方总会生成影子
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), WORLD_UP);
        lightSpaceMatrix = lightProjection * lightView;

        // 从光源角度渲染整个场景
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // 改变视口大小以便于进行深度的渲染
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        // 使用深度shader渲染生成场景
        glClear(GL_DEPTH_BUFFER_BIT);
        renderCarAndCamera(carModel, cameraModel, depthShader);
        renderRaceTrack(raceTrackModel, depthShader);
        renderStopSign(stopSignModel, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 复原视口
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ---------------------------------
        // 模型渲染
        // ---------------------------------

        shader.use();

        // 设置光照相关属性
        renderLight(shader);

        car.UpdateDelayYaw();
        car.UpdateDelayPosition();

        // 切换为相机固定时，需要每次帧修改相机状态
        if (isCameraFixed) {
            updateFixedCamera();
        }

        // 使用shader渲染car和Camera（层级模型）
        renderCarAndCamera(carModel, cameraModel, shader);

        // 渲染Stop牌
        renderStopSign(stopSignModel, shader);

        // 渲染赛道
        renderRaceTrack(raceTrackModel, shader);

        // --------------
        // 最后再渲染天空盒

        // 改变深度测试，使深度等于1.0时为无穷远
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        renderSkyBox(skyboxShader);
        // 复原深度测试
        glDepthFunc(GL_LESS);

        // 交换缓冲区和调查IO事件（按下的按键,鼠标移动等）
        glfwSwapBuffers(window);

        // 轮询事件
        glfwPollEvents();
    }

    // 关闭glfw
    glfwTerminate();
    return 0;
}

// ------------------------------------------
// 其他函数
// ------------------------------------------


// ---------------------------------
// 初始化
// ---------------------------------

GLFWwindow* windowInit()
{
    // 初始化配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, u8"RacingGamesDemo | Ceynri", NULL, NULL);
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
    // 加载所有OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        system("pause");
        return false;
    }

    // 配置全局openGL状态
    glEnable(GL_DEPTH_TEST);

    return true;
}

// 深度图配置
void depthMapFBOInit()
{
    glGenFramebuffers(1, &depthMapFBO);
    // 创建深度纹理
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // 把生成的深度纹理作为帧缓冲的深度缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 天空盒配置
void skyboxInit()
{
    // skybox VAO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // 纹理加载
    cubemapTexture = loadCubemap(faces);
}

// ---------------------------------
// 时间相关函数
// ---------------------------------

// 计算一帧的时间长度
void setDeltaTime()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void changeLightPosAsTime()
{
    float freq = 0.1;
    lightPos.x = 1.0 + cos(glfwGetTime() * freq) * -0.5f;
    lightPos.z = -1.0 + sin(glfwGetTime() * freq) * 0.5f;
    lightPos.y = 1.0 + cos(glfwGetTime() * freq) * 0.5f;
    lightDirection = glm::normalize(lightPos);
}

// ---------------------------------
// 相机位置更新
// ---------------------------------

void updateFixedCamera()
{
    // 自动逐渐复原Zoom为默认值
    camera.ZoomRecover();

    // 处理相机相对于车坐标系下的向量坐标转换为世界坐标系下的向量
    float angle = glm::radians(-car.getMidValYaw());
    glm::mat4 rotateMatrix(
        cos(angle), 0.0, sin(angle), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(angle), 0.0, cos(angle), 0.0,
        0.0, 0.0, 0.0, 1.0);
    glm::vec3 rotatedPosition = glm::vec3(rotateMatrix * glm::vec4(fixedCamera.getPosition(), 1.0));

    camera.FixView(rotatedPosition + car.getMidValPosition(), fixedCamera.getYaw() + car.getMidValYaw());
}

// ---------------------------------
// 渲染函数
// ---------------------------------

// 设置光照相关属性
void renderLight(Shader& shader)
{
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightDirection", lightDirection);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

void renderCarAndCamera(Model& carModel, Model& cameraModel, Shader& shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    shader.setMat4("view", viewMatrix);
    // 投影转换
    glm::mat4 projMatrix = camera.GetProjMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
    shader.setMat4("projection", projMatrix);

    // -------
    // 层级建模

    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, car.getMidValPosition());
    modelMatrix = glm::rotate(modelMatrix, glm::radians(car.getDelayYaw() / 2), WORLD_UP);

    // 渲染汽车
    renderCar(carModel, modelMatrix, shader);

    // 由于mat4作函数参数时为值传递，故不需要备份modelMatrix

    // 渲染相机
    renderCamera(cameraModel, modelMatrix, shader);
}

// 渲染汽车
void renderCar(Model& model, glm::mat4 modelMatrix, Shader& shader)
{
    modelMatrix = glm::rotate(modelMatrix, glm::radians(car.getYaw() - car.getDelayYaw() / 2), WORLD_UP);
    // 抵消模型原本自带的旋转
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), WORLD_UP);
    // 调整模型大小
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.004f, 0.004f, 0.004f));

    // 应用变换矩阵
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}

void renderCamera(Model& model, glm::mat4 modelMatrix, Shader& shader)
{
    modelMatrix = glm::rotate(modelMatrix, glm::radians(fixedCamera.getYaw() + car.getYaw() / 2), WORLD_UP);
    modelMatrix = glm::translate(modelMatrix, cameraPos);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f, 0.01f, 0.01f));

    // 应用变换矩阵
    shader.setMat4("model", modelMatrix);

    model.Draw(shader);
}

void renderStopSign(Model& model, Shader& shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    shader.setMat4("view", viewMatrix);
    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(3.0f, 1.5f, -4.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-120.0f), WORLD_UP);
    shader.setMat4("model", modelMatrix);
    // 投影转换
    glm::mat4 projMatrix = camera.GetProjMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
    shader.setMat4("projection", projMatrix);

    model.Draw(shader);
}

void renderRaceTrack(Model& model, Shader& shader)
{
    // 视图转换
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    shader.setMat4("view", viewMatrix);
    // 模型转换
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    shader.setMat4("model", modelMatrix);
    // 投影转换
    glm::mat4 projMatrix = camera.GetProjMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);
    shader.setMat4("projection", projMatrix);

    model.Draw(shader);
}

void renderSkyBox(Shader& shader)
{
    // viewMatrix 通过构造，移除相机的移动
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    // 投影
    glm::mat4 projMatrix = camera.GetProjMatrix((float)SCR_WIDTH / (float)SCR_HEIGHT);

    shader.setMat4("view", viewMatrix);
    shader.setMat4("projection", projMatrix);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// ---------------------------------
// 键盘/鼠标监听
// ---------------------------------

void handleKeyInput(GLFWwindow* window)
{
    // esc退出
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!isCameraFixed) {
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
    } else {
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            fixedCamera.ProcessKeyboard(CAMERA_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            fixedCamera.ProcessKeyboard(CAMERA_RIGHT, deltaTime);
    }

    // 车车移动
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        car.ProcessKeyboard(CAR_FORWARD, deltaTime);

        // 只有车车动起来的时候才可以左右旋转
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            car.ProcessKeyboard(CAR_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            car.ProcessKeyboard(CAR_RIGHT, deltaTime);

        if (isCameraFixed)
            camera.ZoomOut();
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        car.ProcessKeyboard(CAR_BACKWARD, deltaTime);

        // 同上
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            car.ProcessKeyboard(CAR_LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            car.ProcessKeyboard(CAR_RIGHT, deltaTime);

        if (isCameraFixed)
            camera.ZoomIn();
    }
    
    // 回调监听按键（一个按键只会触发一次事件）
    glfwSetKeyCallback(window, key_callback);
}

// 按键回调函数，使得一次按键只触发一次事件
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        isCameraFixed = !isCameraFixed;
        string info = isCameraFixed ? "切换为固定视角" : "切换为自由视角";
        std::cout << "[CAMERA]" << info << std::endl;
    }
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        isPolygonMode = !isPolygonMode;
        if (isPolygonMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        string info = isPolygonMode ? "切换为线框图渲染模式" : "切换为正常渲染模式";
        std::cout << "[POLYGON_MODE]" << info << std::endl;
    }
}

// 鼠标移动
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!isCameraFixed) {
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
}

// 鼠标滚轮
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// ---------------------------------
// 窗口相关函数
// ---------------------------------

// 改变窗口大小的回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // 确保窗口匹配的新窗口尺寸
    glViewport(0, 0, width, height);
}

// ---------------------------------
// 加载相关函数
// ---------------------------------

// 将六份纹理加载为一个cubemap纹理
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
