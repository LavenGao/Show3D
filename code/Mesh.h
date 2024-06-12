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
	// position ����λ������
	glm::vec3 Position;
	// normal ������
	glm::vec3 Normal;
	// texCoords ��������
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;//�����id
	std::string type;//�������ͣ���������������ͼ�����Ǿ������ͼ
	std::string path;
};

class Mesh {
public:
	/*  ��������  */
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::vector<Texture> textures;
	glm::vec3 translation;//ÿ��mesh��һ��ƽ�ƾ���(����չλMatrix = T * R * S)����ֻ��T
	/*  ����  */
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned short> indices, std::vector<Texture> textures, glm::vec3 translation);
	void Draw(Shader shader);//��������,����ɫ�������������п����������ڻ���֮ǰ����һЩuniform���������Ӳ�����������Ԫ��
private:
	/*  ��Ⱦ����  */
	unsigned int VAO, VBO, EBO;
	unsigned int lightCubeVAO;//���ӹ�ԴVAO
	/*  ����  */
	void setupMesh();//��ʼ������
};
#endif
