// Refer to the learnopengl library

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader_m.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "assimp.lib")

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model {
public:
    /*  模型数据  */
    vector<Texture> textures_loaded; // 保存所有加载过的纹理，以优化被反复使用的相同纹理
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;

    /*  函数  */
    // 构造函数，从路径中读取模型
    Model(string const& path, bool gamma = false)
        : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // 绘制模型的所有网格
    void Draw(Shader shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++) {
            meshes[i].Draw(shader);
        }
    }

private:
    /*  方法  */
    // 从文件加载模型支持 ASSIMP 扩展并存储在网格生成的网格向量
    void loadModel(string const& path)
    {
        // 通过 ASSIMP 加载模型文件
        Assimp::Importer importer;
        // 参数：将模型所有的图元形状变换为三角形 | 在处理的时候翻转y轴的纹理坐标 | 为每个加载的顶点计算出柔和的Tangent和Bitangent向量
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // 判断是否有错误
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // 检索文件路径的目录路径
        directory = path.substr(0, path.find_last_of('/'));

        // 递归处理 ASSIMP 的根节点
        processNode(scene->mRootNode, scene);
    }

    // 以递归方式处理节点
    // 处理节点中每个单独的网格，并对于其子节点（如果有）重复此过程
    // scene 包含所有数据，node 用来表达部件之间的关系
    void processNode(aiNode* node, const aiScene* scene)
    {
        // 处理节点的所有网格
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // 接下来对它的子节点重复这一过程
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 遍历 mesh 的所有顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // 因为assimp使用了自己的向量类，该类不会直接转换为glm的vec3类，因此我们首先将数据传输到该占位符glm::vec3
            glm::vec3 vector;
            // 位置
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // 法线
            if (mesh->mNormals) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            } else {
                vertex.Normal = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            // 纹理坐标
            if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
            {
                glm::vec2 vec;
                // 由于一个顶点可以包含8种不同的纹理坐标，我们假设我们不会使用其中一个顶点可以有多个纹理坐标的模型,所以我们总是取0
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            // tangent
            if (mesh->mTangents) {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            } else {
                vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            // bitangent
            if (mesh->mBitangents) {
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            } else {
                vertex.Bitangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            vertices.push_back(vertex);
        }

        // 处理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // 检索所有面的索引，存储索引向量
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // 处理材质
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // 假定 shader 内的材质相关变量命名都遵循以下规范：（N为0到最大值的一个数字）
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN
        // height: texture_heightN

        // diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // 返回一个 Mesh 对象
        return Mesh(vertices, indices, textures);
    }

    // 加载材质纹理
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // 检查结构是否被加载，如果是则跳过
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // 相同的纹理路径已经加载过了，可以跳过
                    break;
                }
            }
            // 未加载过
            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture); // 添加到已加载的纹理中
            }
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif