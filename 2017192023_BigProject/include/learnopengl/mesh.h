#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader_m.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// 顶点结构体
struct Vertex {
    // 位置
    glm::vec3 Position;
    // 法线
    glm::vec3 Normal;
    // 纹理坐标
    glm::vec2 TexCoords;
    // Tangent
    glm::vec3 Tangent;
    // Bitangent
    glm::vec3 Bitangent;
};
// 由于结构体的特性，变量会连续排布在内存，从而在传入数据时完美转化而无需改成 float 数组

// 纹理结构体
struct Texture {
    unsigned int id;
    string type;
    string path;
};

// mesh
class Mesh {
public:
    /*  网格数据  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    /*  函数  */
    // 构造函数
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 去设置顶点缓冲区指针和它的属性
        setupMesh();
    }

    // 渲染 mesh
    void Draw(Shader shader)
    {
        // 绑定合适的纹理
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            // 在绑定之前激活相应的纹理单元
            glActiveTexture(GL_TEXTURE0 + i);
            // 获取纹理序号（diffuse_textureN 中的 N）
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++); // 无符号整型转移到流中，下同
            else if (name == "texture_specular")
                number = std::to_string(specularNr++);
            else if (name == "texture_normal")
                number = std::to_string(normalNr++);
            else if (name == "texture_height")
                number = std::to_string(heightNr++);

            // 将取样器设置为正确的纹理单元
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // 绑定纹理
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // 绘制 mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // （好习惯）返回默认配置
        glActiveTexture(GL_TEXTURE0);
    }

private:
    /*  渲染数据  */
    unsigned int VBO, EBO;

    /*  函数  */
    // 初始化所有的缓冲区对象/数组（VBO/VAO）
    void setupMesh()
    {
        // 创建 buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // 加载数据进入 vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置vertex属性指针
        // vertex 位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 第一个参数指定从索引0开始取数据，与顶点着色器中layout(location=0)对应。
        // 第二个参数指定顶点属性大小。
        // 第三个参数指定数据类型。
        // 第四个参数定义是否希望数据被标准化（归一化），只表示方向不表示大小。
        // 第五个参数是步长（Stride），指定在连续的顶点属性之间的间隔。上面传0和传4效果相同，如果传1取值方式为0123、1234、2345……
        // 第六个参数表示我们的位置数据在缓冲区起始位置的偏移量。

        // vertex 法线
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex 纹理坐标
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex Tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex Bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);
    }
};
#endif