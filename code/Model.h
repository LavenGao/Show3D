#pragma once

#ifndef MODEL_H
#define MODEL_H

#include "stb_image.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include <Json\json.hpp>
#include <map>

using json = nlohmann::json;

//extern Camera camera;//�������������
extern std::map<std::string, int> AccessorElementSizeMap;
extern std::map<int, std::string> AccessorComponentTypeMap;
//std::map<std::string, int> AccessorElementSizeMap = {
//	{"SCALAR",1},
//	{"VEC2",2},{"VEC3",3}, {"VEC4",4},
//	{"MAT2",2}, {"MAT3",3},{"MAT4",4}
//};
//std::map<int, std::string> AccessorComponentTypeMap = {
//	{5123, "UNSIGNED_SHORT"},
//	{5126, "FLOAT"}
//};

class Model
{
public:
	/*  ����   */
	void Draw(Shader &shader, Camera camera);
	json gltfToJsonRoot(std::string gltfPath);
	void updateWholeMesh(json root);//���忴children���м������ӣ����м�������
	//��ȡ�任����
	glm::mat4 readMatrix(glm::mat4 &matrix, json root);//��ȡmatrix��Ϣ
	/*  ģ������  */
	//���ս������
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::vector<Texture> textures;
	std::vector<Mesh> meshes;//�˴���Ժ���ģ�ͣ�ֻ��һ��Mesh����    
	glm::vec3 translation;//ÿ��mesh��һ��ƽ�ƾ���(����չλMatrix = T * R * S)����ֻ��T
	//glm::vec4 baseColor;
	std::vector<float> baseColor;
private:
	/*  ����   */
	//��ȡ�任����
	std::vector<unsigned short> updateIndicesArray(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<unsigned short> &indices, std::string attributeValue);
	//��ȡ���������Ϣ,��ȡ��������(eg.һ��ֻ�ܻ�ȡPosition����Normal��Ӧ������)
	std::vector<Vertex> updateVertexArrayEle(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<Vertex> &vertexArray, std::string attributeValue);
	//��ȡtranslate����
	void updateTranslate(json root, int meshNum, glm::vec3 &translation);
	void updateMeshParameter(json root, int meshNum);//����һ��mesh�������Ϣ

	void loadMaterialTextures(json root, json singleMaterialNode);//����һ��������Ϣ
	unsigned int TextureFromFile(const char *path);//����������ͼ��Ӧ������ID
};

#endif
