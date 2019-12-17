#pragma once
#include "mesh.h"
class Mesh_Painter
{
public:
	Mesh_Painter();
	~Mesh_Painter();

	void draw_meshes();
	void update_vertex_buffer();
	void update_texture();

	void init_shaders(std::string vs, std::string fs);
	void add_mesh(My_Mesh*);
	void clear_mehs();

private:

	void load_texture_FreeImage(std::string file_name, GLuint& m_texName);







	std::vector<GLuint> textures_all;
	std::vector<GLuint> program_all;
	std::vector<GLuint> vao_all;
	std::vector<GLuint> buffer_all;
	std::vector<GLuint> vPosition_all;
	std::vector<GLuint> vColor_all;
	std::vector<GLuint> vTexCoord_all;
	std::vector<GLuint> vNormal_all;
	std::vector<GLuint>  theta_all;
	std::vector<GLuint>  trans_all;
	   

	std::vector<My_Mesh*> m_my_meshes_;
};

