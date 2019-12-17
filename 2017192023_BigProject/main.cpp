#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Angel.h"
#include "FreeImage.h"
#include "Mesh_Painter.h"
#include "mesh.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "FreeImage.lib")
#pragma comment(lib, "assimp.lib")

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;


std::vector<My_Mesh*> my_meshs;
Mesh_Painter* mp_;

// Texture objects and storage for texture image

// Vertex data arrays

// Array of rotation angles (in degrees) for each coordinate axis

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mp_->draw_meshes();
    glutSwapBuffers();
};

//----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y)
{
}

//----------------------------------------------------------------------------
void idle(void)
{
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int mousex, int mousey)
{

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("2017191023_陈扬锐_期末大作业");

    glewExperimental = GL_TRUE;
    glewInit();

    init();

    mp_ = new Mesh_Painter;

    My_Mesh* my_mesh1 = new My_Mesh;
    my_mesh1->generate_cylinder(100, 3); //生成圆柱表面
    my_mesh1->set_texture_file("asset/texture/cylinder10.jpg"); //指定纹理图像文件
    my_mesh1->set_translate(-0.5, 0, 0);
    my_mesh1->set_theta(90, 0., 30.); //旋转轴
    my_mesh1->set_theta_step(1, 1, 1); //旋转速度
    mp_->add_mesh(my_mesh1);
    my_meshs.push_back(my_mesh1);

    // My_Mesh* my_mesh2 = new My_Mesh;
    // my_mesh2->generate_from_off("asset/model/off/oildrum/oildrum.off");
    // mp_->add_mesh(my_mesh2);
    // my_meshs.push_back(my_mesh2);

    mp_->init_shaders("shader/v_texture.glsl", "shader/f_texture.glsl");
    mp_->update_vertex_buffer();
    mp_->update_texture();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutMainLoop();

    for (unsigned int i = 0; i < my_meshs.size(); i++) {
        delete my_meshs[i];
    }
    delete mp_;

    return 0;
}
