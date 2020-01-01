#ifndef FIXED_CAMERA_H
#define FIXED_CAMERA_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 定义汽车移动的几个可能的选项
enum Rotation {
    CAMERA_LEFT,
    CAMERA_RIGHT
};

class FixedCamera {
public:
    glm::vec3 Position;
    float radius;
    float Yaw;
    float TurningSpeed;

    FixedCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
        : Yaw(0.0f)
        , TurningSpeed(80.0f)
    {
        Position = position;
        radius = Position.z;
    }

    glm::vec3 getPosition()
    {
        return Position;
    }

    float getYaw()
    {
        return Yaw;
    }

    // 计算视图矩阵
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, glm::vec3(0.0f, Position.y, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 接受键盘输入
    void ProcessKeyboard(Rotation direction, float deltaTime)
    {
        if (direction == CAMERA_LEFT)
            Yaw -= TurningSpeed * deltaTime;
        if (direction == CAMERA_RIGHT)
            Yaw += TurningSpeed * deltaTime;
        updatePosition();
    }

private:
    // 计算新的位置
    void updatePosition()
    {
        Position.x = radius * sin(glm::radians(Yaw));
        Position.z = radius * cos(glm::radians(Yaw));
    }
};

#endif