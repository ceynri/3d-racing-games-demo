#ifndef CAR_H
#define CAR_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <queue>

// 定义汽车移动的几个可能的选项
enum Direction {
    CAR_FORWARD,
    CAR_BACKWARD,
    CAR_LEFT,
    CAR_RIGHT
};

class Car {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    float Yaw;
    
    // 存储旧Yaw信息，实现漂移
    queue<float> HistoryYaw;
    int DelayFrameNum = 20;
    float DelayYaw;

    float MovementSpeed;
    float TurningSpeed;

    Car(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
        : MovementSpeed(30.0f)
        , TurningSpeed(60.0f)
        , Yaw(0.0f)
        , DelayYaw(0.0f)
    {
        Position = position;
        updateFront();
    }

    glm::vec3 getPosition()
    {
        return Position;
    }

    float getYaw()
    {
        return Yaw;
    }

    float getDelayYaw()
    {
        return DelayYaw;
    }

    float getYawDif()
    {
        return Yaw - DelayYaw;
    }

    // 计算视图矩阵
    glm::mat4 GetViewMatrix(glm::vec3 cameraPosition)
    {
        return glm::lookAt(cameraPosition, Position, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // 接受键盘输入
    void ProcessKeyboard(Direction direction, float deltaTime)
    {
        if (direction == CAR_FORWARD)
            Position += Front * MovementSpeed * deltaTime;
        if (direction == CAR_BACKWARD)
            Position -= Front * MovementSpeed * deltaTime;
        if (direction == CAR_LEFT)
            Yaw += TurningSpeed * deltaTime;
        if (direction == CAR_RIGHT)
            Yaw -= TurningSpeed * deltaTime;
        updateFront();
    }

    // 更新DalayYaw
    void UpdateDelayYaw()
    {
        HistoryYaw.push(Yaw);
        while (HistoryYaw.size() > DelayFrameNum) {
            HistoryYaw.pop();
        }
        DelayYaw = HistoryYaw.front();
    }

private:
    // 计算新的 Front 向量
    void updateFront()
    {
        glm::vec3 front;
        front.x = -sin(glm::radians(Yaw));
        front.y = 0.0f;
        front.z = -cos(glm::radians(Yaw));
        Front = glm::normalize(front);
    }
};

#endif