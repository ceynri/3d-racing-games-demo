#include "Mesh_Painter.h"
#include "FreeImage.h"

Mesh_Painter::Mesh_Painter()
{
}


Mesh_Painter::~Mesh_Painter()
{
}


void Mesh_Painter::draw_meshes()
{
	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		glUseProgram(this->program_all[i]);//指定使用渲染器，不同的模型可以指定不同的渲染器，这里我们使用的渲染器来自相同的文件，学生可以根据自己的爱好对不同的模型设定不同的渲染器
		glBindVertexArray(this->vao_all[i]);

 		glActiveTexture(GL_TEXTURE0);
 		glBindTexture(GL_TEXTURE_2D, this->textures_all[i]);//该语句必须，否则将只使用同一个纹理进行绘制
		
		float x, y, z;
		this->m_my_meshes_[i]->get_theta(x, y, z);
		GLfloat  Theta[3] = {x, y, z};
		this->m_my_meshes_[i]->add_theta_step();
 		glUniform3fv(theta_all[i], 1, Theta);

		this->m_my_meshes_[i]->get_translate(x, y, z);
		GLfloat  vTranslation[3] = { x, y, z };
		glUniform3fv(trans_all[i], 1, vTranslation);

		glDrawArrays(GL_TRIANGLES, 0, this->m_my_meshes_[i]->num_faces() * 3);
		glUseProgram(0);
	}
};


void Mesh_Painter::update_texture()
{
	this->textures_all.clear();

	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		GLuint textures;

		glGenTextures(1, &textures);
		//调用FreeImage生成纹理
		load_texture_FreeImage(this->m_my_meshes_[i]->get_texture_file(), textures);

		//指定最大最小过滤方法，此两行代码必须添加，否则将无法显示纹理
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//将生成的纹理传给shader
		glBindTexture(GL_TEXTURE_2D, textures);
		glUniform1i(glGetUniformLocation(this->program_all[i], "texture"), 0);
		this->textures_all.push_back(textures);
	}


};


void Mesh_Painter::load_texture_FreeImage(std::string file_name, GLuint& m_texName)
{
	//1 获取图片格式
	FREE_IMAGE_FORMAT fifmt = FreeImage_GetFileType(file_name.c_str(), 0);

	//2 加载图片
	FIBITMAP *dib = FreeImage_Load(fifmt, file_name.c_str(), 0);

	//3 转化为rgb 24色;
	dib = FreeImage_ConvertTo24Bits(dib);

	//4 获取数据指针
	BYTE *pixels = (BYTE*)FreeImage_GetBits(dib);

	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	/**
	* 产生一个纹理Id,可以认为是纹理句柄，后面的操作将书用这个纹理id
	*/

	/**
	* 使用这个纹理id,或者叫绑定(关联)
	*/
	glBindTexture(GL_TEXTURE_2D, m_texName);
	/**
	* 指定纹理的放大,缩小滤波，使用线性方式，即当图片放大的时候插值方式
	*/
	/**
	* 将图片的rgb数据上传给opengl.
	*/
	glTexImage2D(
		GL_TEXTURE_2D, //! 指定是二维图片
		0, //! 指定为第一级别，纹理可以做mipmap,即lod,离近的就采用级别大的，远则使用较小的纹理
		GL_RGB, //! 纹理的使用的存储格式
		width, //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
		height, //! 宽度，老一点的显卡，不支持不规则的纹理，即宽度和高度不是2^n。
		0, //! 是否的边
		GL_BGR_EXT, //! 数据的格式，bmp中，windows,操作系统中存储的数据是bgr格式
		GL_UNSIGNED_BYTE, //! 数据是8bit数据
		pixels
		);
	/**
	* 释放内存
	*/
	FreeImage_Unload(dib);
};



void Mesh_Painter::update_vertex_buffer()
{


	this->vao_all.clear();
	this->buffer_all.clear();
	this->vPosition_all.clear();
	this->vColor_all.clear();
	this->vTexCoord_all.clear();
	this->vNormal_all.clear();

	for (unsigned int m_i = 0; m_i < this->m_my_meshes_.size(); m_i++)
	{
		int num_face = this->m_my_meshes_[m_i]->num_faces();
		int num_vertex = this->m_my_meshes_[m_i]->num_vertices();

		const VertexList& vertex_list = this->m_my_meshes_[m_i]->get_vertices();
		const NormalList& normal_list = this->m_my_meshes_[m_i]->get_normals();
		const FaceList&  face_list = this->m_my_meshes_[m_i]->get_faces();
		const STLVectorf& color_list = this->m_my_meshes_[m_i]->get_colors();
		const VtList& vt_list = this->m_my_meshes_[m_i]->get_vts();


		// Create a vertex array object
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);


		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vec3)*num_face * 3
			+ sizeof(vec3)*num_face * 3
			+ sizeof(vec3)*num_face * 3
			+ sizeof(vec2)*num_face * 3
			,
			NULL, GL_STATIC_DRAW)
			;
		//获得足够的空间存储坐标，颜色，法线以及纹理坐标等等
		// Specify an offset to keep track of where we're placing data in our
		//   vertex array buffer.  We'll use the same technique when we
		//   associate the offsets with vertex attribute pointers.

		vec3* points = new vec3[num_face * 3];
		point3f  p_center_ = this->m_my_meshes_[m_i]->get_center();
		point3f p_min_box_, p_max_box_;
		this->m_my_meshes_[m_i]->get_boundingbox(p_min_box_, p_max_box_);
		float d = p_min_box_.distance(p_max_box_);

		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3 * i];
			points[3 * i] = vec3(
				(vertex_list[index * 3 + 0] - p_center_.x) / (1.5 * d),
				(vertex_list[index * 3 + 1] - p_center_.y) / (1.5 * d),
				(vertex_list[index * 3 + 2] - p_center_.z) / (1.5 * d)
				);

			index = face_list[3 * i + 1];
			points[3 * i + 1] = vec3(
				(vertex_list[index * 3 + 0] - p_center_.x) / (1.5 * d),
				(vertex_list[index * 3 + 1] - p_center_.y) / (1.5 * d),
				(vertex_list[index * 3 + 2] - p_center_.z) / (1.5 * d)
				);

			index = face_list[3 * i + 2];
			points[3 * i + 2] = vec3(
				(vertex_list[index * 3 + 0] - p_center_.x) / (1.5 * d),
				(vertex_list[index * 3 + 1] - p_center_.y) / (1.5 * d),
				(vertex_list[index * 3 + 2] - p_center_.z) / (1.5 * d)
				);

		}
		GLintptr offset = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3)*num_face * 3, points);
		//顶点坐标传给shader
		offset += sizeof(vec3)*num_face * 3;
		delete[] points;
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		points = new vec3[num_face * 3];
		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3 * i];
			points[3 * i] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);

			index = face_list[3 * i + 1];
			points[3 * i + 1] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);

			index = face_list[3 * i + 2];
			points[3 * i + 2] = vec3(
				(normal_list[index * 3 + 0]),
				(normal_list[index * 3 + 1]),
				(normal_list[index * 3 + 2])
				);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * num_face * 3, points);
		offset += sizeof(vec3) * num_face * 3;
		delete[] points;
		//法线传给shader

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		points = new vec3[num_face * 3];
		for (int i = 0; i < num_face; i++)
		{
			int index = face_list[3 * i];
			points[3 * i] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);

			index = face_list[3 * i + 1];
			points[3 * i + 1] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);

			index = face_list[3 * i + 2];
			points[3 * i + 2] = vec3(
				(color_list[index * 3 + 0]),
				(color_list[index * 3 + 1]),
				(color_list[index * 3 + 2])
				);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec3) * num_face * 3, points);
		//颜色传给shader
		offset += sizeof(vec3) * num_face * 3;
		delete[] points;
		

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		vec2* points_2 = new vec2[num_face * 3];
		for (int i = 0; i < num_face; i++)
		{
			points_2[i * 3] = vec2(vt_list[i * 6 + 0], vt_list[i * 6 + 1]);
			points_2[i * 3 + 1] = vec2(vt_list[i * 6 + 2], vt_list[i * 6 + 3]);
			points_2[i * 3 + 2] = vec2(vt_list[i * 6 + 4], vt_list[i * 6 + 5]);
		}
		glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * num_face * 3, points_2);
		offset += sizeof(vec2) * num_face * 3;
		delete points_2;
		//纹理坐标传给shader
		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		// Load shaders and use the resulting shader program


		// set up vertex arrays
		offset = 0;
		GLuint vPosition;
		vPosition = glGetAttribLocation(this->program_all[m_i], "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		//指定vPosition在shader中使用时的位置

		GLuint vNormal;
		vNormal = glGetAttribLocation(this->program_all[m_i], "vNormal");
		glEnableVertexAttribArray(vNormal);
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		//指定vNormal在shader中使用时的位置

		GLuint vColor;
		vColor = glGetAttribLocation(this->program_all[m_i], "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		offset += sizeof(vec3) * num_face * 3;
		//指定vColor在shader中使用时的位置

		GLuint vTexCoord;
		vTexCoord = glGetAttribLocation(this->program_all[m_i], "vTexCoord");
		glEnableVertexAttribArray(vTexCoord);
		glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(offset));
		//指定vTexCoord在shader中使用时的位置

		/************************************************************************/
		/*                                                                      */
		/************************************************************************/

		this->vao_all.push_back(vao);
		this->buffer_all.push_back(buffer);
		this->vPosition_all.push_back(vPosition);
		this->vColor_all.push_back(vColor);
		this->vTexCoord_all.push_back(vTexCoord);
		this->vNormal_all.push_back(vNormal);
	}
};
void Mesh_Painter::init_shaders(std::string vs, std::string fs)
{
	this->program_all.clear();
	this->theta_all.clear();
	this->trans_all.clear();
	for (unsigned int i = 0; i < this->m_my_meshes_.size(); i++)
	{
		GLuint program = InitShader(vs.c_str(), fs.c_str());
		//指定使用的渲染器，不同的模型可以指定不同的渲染器，这里我们使用的渲染器来自相同的文件，学生可以根据自己的爱好对不同的模型设定不同的渲染器
		this->program_all.push_back(program);

		GLuint 	theta = glGetUniformLocation(program, "theta");
		GLuint  trans = glGetUniformLocation(program, "translation");
		theta_all.push_back(theta);
		trans_all.push_back(trans);
	}

};
void Mesh_Painter::add_mesh(My_Mesh* m)
{
	this->m_my_meshes_.push_back(m);
};
void Mesh_Painter::clear_mehs()
{
	this->m_my_meshes_.clear();

	this->textures_all.clear();
	this->program_all.clear();
	this->vao_all.clear();
	this->buffer_all.clear();
	this->vPosition_all.clear();
	this->vColor_all.clear();
	this->vTexCoord_all.clear();
	this->vNormal_all.clear();
};
