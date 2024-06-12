#pragma once
#ifndef MESH_H
#define MESH_H

#include "Shader.h"
//#include <glad/glad.h> // holds all OpenGL type declarations

//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include <string>
#include <vector>



struct Vertex {
	// position 顶点位置向量
	glm::vec3 Position;
	// normal 法向量
	glm::vec3 Normal;
	// texCoords 纹理坐标
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;//纹理的id
	std::string type;//纹理类型，比如是漫反射贴图或者是镜面光贴图
	std::string path;
};

class Mesh {
public:
	/*  网格数据  */
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::vector<Texture> textures;
	glm::vec3 translation;//每个mesh有一个平移矩阵(可扩展位Matrix = T * R * S)这里只有T
	/*  函数  */
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned short> indices, std::vector<Texture> textures, glm::vec3 translation);
	void Draw(Shader shader);//绘制网格,将着色器传入网格类中可以让我们在绘制之前设置一些uniform（像是链接采样器到纹理单元）
private:
	/*  渲染数据  */
	unsigned int VAO, VBO, EBO;
	unsigned int lightCubeVAO;//增加光源VAO
	/*  函数  */
	void setupMesh();//初始化缓冲
};
#endif
