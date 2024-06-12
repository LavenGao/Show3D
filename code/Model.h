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

//extern Camera camera;//相机的世界坐标
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
	/*  函数   */
	void Draw(Shader &shader, Camera camera);
	json gltfToJsonRoot(std::string gltfPath);
	void updateWholeMesh(json root);//具体看children中有几个孩子，就有几个属性
	//读取变换矩阵
	glm::mat4 readMatrix(glm::mat4 &matrix, json root);//读取matrix信息
	/*  模型数据  */
	//最终结果数组
	std::vector<Vertex> vertices;
	std::vector<unsigned short> indices;
	std::vector<Texture> textures;
	std::vector<Mesh> meshes;//此处针对盒子模型，只有一个Mesh对象    
	glm::vec3 translation;//每个mesh有一个平移矩阵(可扩展位Matrix = T * R * S)这里只有T
	//glm::vec4 baseColor;
	std::vector<float> baseColor;
private:
	/*  函数   */
	//读取变换数组
	std::vector<unsigned short> updateIndicesArray(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<unsigned short> &indices, std::string attributeValue);
	//读取顶点相关信息,获取分量数组(eg.一次只能获取Position或者Normal对应的数组)
	std::vector<Vertex> updateVertexArrayEle(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<Vertex> &vertexArray, std::string attributeValue);
	//读取translate矩阵
	void updateTranslate(json root, int meshNum, glm::vec3 &translation);
	void updateMeshParameter(json root, int meshNum);//更新一个mesh的相关信息

	void loadMaterialTextures(json root, json singleMaterialNode);//更新一个纹理信息
	unsigned int TextureFromFile(const char *path);//返回纹理贴图对应的纹理ID
};

#endif
