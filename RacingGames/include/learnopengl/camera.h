#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

// 定义了相机运动的几个可能的选项
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// 默认值
const float YAW = 0.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 80.0f;
const float ZOOM_RANGE = 20.0f;
const float ZOOM_SPEED = 2.0f;
const float STADIA = 1000.0f;

// 抽象相机类
// 处理输入和计算相应的欧拉角、向量和矩阵以用于OpenGL
class Camera {
public:
    // 相机的属性
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // 欧拉角
    float Yaw; // 偏航角
    float Pitch; // 倾斜角
    // 相机的选项
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    // 构造函数
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , MovementSpeed(SPEED)
        , MouseSensitivity(SENSITIVITY)
        , Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 构造函数和标量值
    Camera(
        float posX, float posY, float posZ,
        float upX, float upY, float upZ,
        float yaw,
        float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , MovementSpeed(SPEED)
        , MouseSensitivity(SENSITIVITY)
        , Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 计算视图矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 计算投影矩阵
    glm::mat4 GetProjMatrix(float aspect)
    {
        return glm::perspective(glm::radians(Zoom), aspect, 0.1f, STADIA);
    }

    // 接受键盘输入
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += WorldUp * velocity;
        if (direction == DOWN)
            Position -= WorldUp * velocity;
    }

    // 接受鼠标移动
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw -= xoffset;
        Pitch += yoffset;

        // 限制角度以避免翻转问题
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // 使用欧拉角更新 Front、Up、Right 向量
        updateCameraVectors();
    }

    // 接受鼠标滚轮纵向滚动
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= ZOOM)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= ZOOM)
            Zoom = ZOOM;
    }

    void FixView(glm::vec3 position, float yaw)
    {
        Position = position;
        Yaw = yaw;
        Pitch = 0.0f;
        updateCameraVectors();
    }

    void ZoomIn()
    {
        if (Zoom >= ZOOM - ZOOM_RANGE)
            Zoom -= ZOOM_SPEED;
    }

    void ZoomOut()
    {
        if (Zoom <= ZOOM + ZOOM_RANGE)
            Zoom += ZOOM_SPEED;
    }

    void ZoomRecover()
    {
        if (Zoom < ZOOM)
            Zoom += ZOOM_SPEED / 2;
        if (Zoom > ZOOM)
            Zoom -= ZOOM_SPEED / 2;
    }

private:
    // 通过相机的欧拉角计算 Front、Right、Up 向量
    void updateCameraVectors()
    {
        // 计算新的 Front 向量
        glm::vec3 front;
        front.x = -sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = -cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // 重新计算 Right 和 Up 向量
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif