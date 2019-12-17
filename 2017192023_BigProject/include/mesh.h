#pragma once

#include "Angel.h"
#include <string>
#include <vector>

struct point3f {
    float x;
    float y;
    float z;
    point3f()
    {
        x = 0;
        y = 0;
        z = 0;
    };
    point3f(float xx, float yy, float zz)
    {
        x = xx;
        y = yy;
        z = zz;
    };

    float distance(const point3f& p)
    {
        float d = (x - p.x) * (x - p.x);
        d += (y - p.y) * (y - p.y);
        d += (z - p.z) * (z - p.z);

        return sqrt(d);
    };
};

typedef std::vector<float> VertexList;
typedef std::vector<float> NormalList;
typedef std::vector<float> VtList;

typedef std::vector<float> STLVectorf;
typedef std::vector<int> STLVectori;

typedef std::vector<std::pair<int, int>> Edges;
typedef std::vector<unsigned int> FaceList;

class My_Mesh {
public:
    My_Mesh();
    ~My_Mesh();

    void set_texture_file(std::string s);
    std::string get_texture_file();

    const VertexList& get_vertices();
    const NormalList& get_normals();
    const FaceList& get_faces();
    const STLVectorf& get_colors();
    const VtList& get_vts();

    int num_faces();
    int num_vertices();

    const point3f& get_center();
    void get_boundingbox(point3f& min_p, point3f& max_p) const;

    void generate_from_off(const std::string filename);
    void generate_disk(int num_division = 100);
    void generate_cone(int num_division = 100, float height = 2);
    void generate_cylinder(int num_division = 100, float height = 2);

    static void normal_to_color(float, float, float, float&, float&, float&);
    void set_translate(float, float, float);
    void get_translate(float& x, float& y, float& z);

private:
    void clear_data();

private:
    VertexList m_vertices_;
    NormalList m_normals_;
    FaceList m_faces_;
    STLVectorf m_color_list_;
    VtList m_vt_list_;
    point3f m_center_;
    point3f m_min_box_;
    point3f m_max_box_;

public:
    void add_theta_step();
    void set_theta_step(float x, float y, float z);
    void set_theta(float x, float y, float z);

    void get_theta(float& x, float& y, float& z);

private:
    std::string texture_file_name;

    enum { Xaxis = 0,
        Yaxis = 1,
        Zaxis = 2,
        NumAxes = 3 };
    int Axis = Xaxis;
    GLfloat Theta[3];
    GLfloat Theta_step[3];
    GLfloat vTranslation[3];
};
