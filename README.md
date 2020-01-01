<h1 style="text-align: center">3D Racing Games Demo with OpenGL</h1>
<p style="text-align: center">使用 C++ 和 OpenGL 实现简单的 3D 赛车游戏</p>

<img src="https://i.loli.net/2020/01/01/PAJ5ZVzrMORbcjw.png" style="width: 100%">

<br/>

## 简介 Introduction

计算机图形学课程的大作业，参考 [LearnOpenGL 中文教程](https://learnopengl-cn.github.io/) 完成该 Demo。

引用的 LearnOpenGL 库代码以及自己编写的代码均做有中文注释。

<br/>

##  使用方式 Usage

选择 Debug 或 Release 版本下载解压后点击 `RacingGames.exe` 运行程序。

仓库存放的源码需要在 Windows 平台下使用 VS 2015 编译运行（无兼容性优化）。

<br/>

## 控制 Control

### 键盘控制

全局按键：

| 按键 | 效果                                 |
| ---- | ------------------------------------ |
| ↑    | 汽车向前行驶                         |
| ↓    | 倒车                                 |
| ←    | 左转弯（需要汽车处于移动中才会生效） |
| →    | 右转弯（需要汽车处于移动中才会生效） |
| Esc  | 退出游戏                             |
| C    | 切换相机视角为固定视角/自由视角      |
| X    | 切换渲染方式为线框图模式/正常模式    |

自由视角下：

| 按键   | 效果             |
| ------ | ---------------- |
| W      | 镜头向前移动     |
| S      | 镜头向后移动     |
| A      | 镜头向左移动     |
| D      | 镜头向右移动     |
| Space  | 镜头竖直向上移动 |
| 左Ctrl | 镜头竖直向下移动 |

固定视角下：

| 按键 | 效果               |
| ---- | ------------------ |
| A    | 镜头绕车身向左旋转 |
| D    | 镜头绕车身向右旋转 |

<br/>

### 鼠标控制

| 按键     | 效果                         |
| -------- | ---------------------------- |
| 鼠标移动 | 视角方向移动（仅自由视角下） |
| 鼠标滚轮 | 镜头变焦                     |

<br/>

---
## 项目介绍 Project Introduction

### 项目结构 Project Structure

- asset 存放工程相关资源
  - textures 存放天空盒的原图
  - models 存放模型
- include 存放引用的头文件
- lib 存放引用的 lib 文件
- shader 存放着色器文件

<br/>

### 实现功能 Implement

1.	obj 模型的读取与渲染（assimp 库）
2.	纹理贴图
3.	Blinn-Phong 光照模型
4.  阴影效果实现（阴影贴图方式）
5.  阴影边缘优化（PCF）
5.	天空盒
6.	相机固定视角与自由视角的实现与切换
7.	汽车移动与转弯实现
8.	转弯视角偏移延迟
9.	汽车的缓动、缓停与漂移效果
10.	行驶加速视觉效果
11.	赛车-相机的层级建模结构

<br/>

### 运行环境 Environment

? 64位电脑 Win10 系统下 Visual Studio 2015 使用 x86 Debug/Release 版本编译测试通过

<br/>

### 引用库 Reference Library

- assimp
- glad
- glfw
- glm
- khr
- stb_image
- learnopengl

<br/>
