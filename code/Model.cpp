#include "Model.h"

/*
*读取gltf文件，将其转化为json对象
*in:string gltfPath
*out:json root
*/
json Model::gltfToJsonRoot(std::string gltfPath)
{
	json root;
	std::ifstream in(gltfPath, std::ios::binary);
	if (!in.is_open())
	{
		std::cout << "Error opening file\n";//文件未成功打开
		//return;
	}
	in >> root;//将Json文件内容JsonContent存入Json对象
	return root;
}

//读取translate矩阵
void Model::updateTranslate(json root, int meshNum, glm::vec3 &translation)
{
	//假命题//gltf转成json元素的顺序会发生改变，所以要遍历nodes中的节点判断mesh对应的值是不是等于当前的meshNum
	json node = root["nodes"];
	for (int i = 0; i < node.size(); i++)
	{
		json curNode = node.at(i);
		if (curNode["mesh"] == meshNum)
		{
			curNode = curNode["translation"];
			translation = glm::vec3(curNode.at(0), curNode.at(1), curNode.at(2));
			break;
		}
	}
}

//更新一个mesh的相关信息 
void Model::updateMeshParameter(json root, int meshNum)
{
	json node = root["meshes"].at(meshNum);
	node = node["primitives"];
	node = node[0];

	if (node.is_object())
	{
		//遍历obj中的每个元素
		for (json::iterator it = node.begin(); it != node.end(); it++)
		{
			if (it.key() == "attributes")
			{
				//std::cout << it.key()<< "\n";
				//当前key为"attributes"，遍历它的value
				json curNode = it.value();
				json accessorsNode = root["accessors"];  //attribute的属性都是使用accesors获取的
				int positionIndex = curNode["POSITION"];//一定会有POSITION，取POSITION对应的accessors的位置
				json tempNode = accessorsNode[positionIndex];
				int VertexArrayCount = tempNode["count"];//提前定义好vertex数组的大小

				//创建VertexArrayCount大小的动态数组,attributes的属性将会保存到vertex当中（vertex.position、vertex.Normal等）
				std::vector<Vertex> vertexArray(VertexArrayCount);
				vertices = vertexArray;
				int testCount = 0;
				for (json::iterator iSec = curNode.begin(); iSec != curNode.end(); iSec++)
				{
					int curNum = iSec.value();//获取当前属性（eg.POSITION）在accessors中的位置
					json singleAcessorNode = accessorsNode[curNum];;//获取accessors中当前属性（eg.POSITION)对应的json OBJ
					int bufferViewIndex = singleAcessorNode["bufferView"].get<int>();//当前属性（eg.POSITION)在bufferView中的索引
					int byteOffset = 0;//当前属性（eg.POSITION)在bufferView对应的内存中的起始偏移量
					if (singleAcessorNode.find("byteOffset") != singleAcessorNode.end())
					{
						//box中有这个key
						int byteOffset = singleAcessorNode["byteOffset"].get<int>();
					}
					else
					{
						//latern中没有byteOffset这个key,没有就默认从0读
						//int byteOffset = 0;
					}
					std::string componentType = AccessorComponentTypeMap[int(singleAcessorNode["componentType"])];//float,5126
					int AccessorElementTypeSize = AccessorElementSizeMap[singleAcessorNode["type"].get<std::string>()];//4
					//更新vertices数组中的POSITION分量
					testCount++;
					vertices = updateVertexArrayEle(bufferViewIndex, byteOffset, VertexArrayCount, componentType, AccessorElementTypeSize, root, vertices, iSec.key());
				}
				//std::cout <<"主函数测试"<< vertices[5].Position.x<<"," << vertices[5].Position.y << "," << vertices[5].Position.z <<std::endl;
				//std::cout << "主函数测试" << vertices[5].Normal.x << "," << vertices[5].Normal.y << "," << vertices[5].Normal.z << std::endl;

			}
			else if (it.key() == "indices")
			{
				//将value中的每个对象存入vector<unsigned int> indices;
				//std::cout << it.key() << "\n";
				//当前key为"indices"，取indices对应的在accessors中的位置
				json accessorsNode = root["accessors"];
				int positionIndex = it.value();//取indices对应的在accessors中的位置
				json accessorsIndiceNode = accessorsNode[positionIndex];
				int indicesArrayCount = accessorsIndiceNode["count"];
				//std::vector<unsigned short> indices(indicesArrayCount);//////////////////////////////////////////////////
				int bufferViewIndex = accessorsIndiceNode["bufferView"].get<int>();//bufferViews中的第几个
				int byteOffset = 0;//当前属性（eg.POSITION)在bufferView对应的内存中的起始偏移量
				if (accessorsIndiceNode.find("byteOffset") != accessorsIndiceNode.end())
				{
					//box中有这个key
					int byteOffset = accessorsIndiceNode["byteOffset"].get<int>();//indices数据在读取到的内存块中的起始位置
				}
				else
				{
					//latern中没有byteOffset这个key,没有就默认从0读
					//int byteOffset = 0;
				}

				std::string componentType = AccessorComponentTypeMap[int(accessorsIndiceNode["componentType"])];//此处根据5123取到UNSIGNED_SHORT
				int AccessorElementTypeSize = AccessorElementSizeMap[accessorsIndiceNode["type"].get<std::string>()];
				//std::string attributeValue eg. POSITION
				//indices = 
				updateIndicesArray(bufferViewIndex, byteOffset, indicesArrayCount, componentType, AccessorElementTypeSize, root, indices, it.key());
				/*for (int i = 0; i < indices.size(); i++)
				{
					std::cout << i<<":" << indices[i] << ", ";
				}
				std::cout << std::endl;*/
			}
			else if (it.key() == "mode")
			{
				//triangle

			}
			else if (it.key() == "material")
			{
				int materialIndex = it.value();//取material对应的在materials中的位置
				json materialsNode = root["materials"];
				json singleMaterialNode = materialsNode[materialIndex];//这里都是0

				loadMaterialTextures(root, singleMaterialNode);

			}
		}
	}
	//读取translate矩阵
	updateTranslate(root, meshNum, translation);
	meshes.push_back(Mesh(vertices, indices, textures, translation));
}

//更新一个纹理信息
void Model::loadMaterialTextures(json root, json singleMaterialNode)
{
	//遍历materials中的key，存储所对应的贴图
	for (json::iterator it = singleMaterialNode.begin(); it != singleMaterialNode.end(); it++)
	{
		if (it.key() == "pbrMetallicRoughness")
		{
			//先只考虑baseColorTexture
			json basecolorNode1 = it.value()["baseColorTexture"];
			int textureIndex1 = basecolorNode1["index"];
			json textureNode1 = root["textures"].at(textureIndex1);
			int imageIndex1 = textureNode1["source"];
			json imageNode1 = root["images"].at(imageIndex1);
			std::string uri1 = imageNode1["uri"].get<std::string>();
			uri1 = "./glTF/" + uri1;//添加相对路径
			const char * binUri1 = uri1.c_str();//"./glTF/Lantern.bin";
		   //目前文件指针在0的位置
			//std::fstream file;
			//file.open(binUri, std::ios::in | std::ios::binary);
			Texture texture1;
			texture1.id = TextureFromFile(binUri1);
			texture1.type = "texture_basecolor";
			texture1.path = binUri1;
			textures.push_back(texture1);

			//再考虑metallicRoughnessTexture
			json basecolorNode2 = it.value()["metallicRoughnessTexture"];
			int textureIndex2 = basecolorNode2["index"];
			json textureNode2 = root["textures"].at(textureIndex2);
			int imageIndex2 = textureNode2["source"];
			json imageNode2 = root["images"].at(imageIndex2);
			std::string uri2 = imageNode2["uri"].get<std::string>();
			uri2 = "./glTF/" + uri2;//添加相对路径
			const char * binUri2 = uri2.c_str();//"./glTF/Lantern.bin";
			Texture texture2;
			texture2.id = TextureFromFile(binUri2);
			texture2.type = "texture_metallicRoughness";
			texture2.path = binUri2;
			textures.push_back(texture2);
		}
		else if (it.key() == "emissiveTexture")
		{
			//配合emissiveFactor一起使用
			json emissiveTextureNode = it.value();
			int emissiveTextureIndex = emissiveTextureNode["index"];
			json textureNode = root["textures"].at(emissiveTextureIndex);
			int imageIndex = textureNode["source"];
			json imageNode = root["images"].at(imageIndex);
			std::string uri = imageNode["uri"].get<std::string>();
			uri = "./glTF/" + uri;//添加相对路径
			const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
			Texture texture;
			texture.id = TextureFromFile(binUri);
			texture.type = "texture_emissive";
			texture.path = binUri;
			textures.push_back(texture);
		}
		else if (it.key() == "normalTexture")
		{
			//法线贴图
			json normalTextureNode = it.value();
			int normalTextureIndex = normalTextureNode["index"];
			json textureNode = root["textures"].at(normalTextureIndex);
			int imageIndex = textureNode["source"];
			json imageNode = root["images"].at(imageIndex);
			std::string uri = imageNode["uri"].get<std::string>();
			uri = "./glTF/" + uri;//添加相对路径
			const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
			Texture texture;
			texture.id = TextureFromFile(binUri);
			texture.type = "texture_normal";
			texture.path = binUri;
			textures.push_back(texture);
		}
		else
		{
			//可扩展
		}

	}

}

//返回纹理贴图对应的纹理ID
unsigned int Model::TextureFromFile(const char *path)//, bool gamma)
{
	std::string filename = std::string(path);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
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
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

//更新全部mesh节点，具体看children中有几个孩子，就有几个属性
void Model::updateWholeMesh(json root)
{
	json node = root["nodes"];
	int meshSize = 0;
	for (int i = 0; i < node.size(); i++)
	{
		if (node.at(i).find("children") != node.at(i).end())
		{
			meshSize = node.at(i)["children"].size();
			break;
		}
	}
	//更新meshSize次meshes数组
	for (int i = 0; i < meshSize; i++)
	{
		updateMeshParameter(root, i);
		updateTranslate(root, i, translation);
	}
}

//遍历了所有网格，并调用它们各自的Draw函数
void Model::Draw(Shader &shader, Camera camera)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		//std::cout << i << std::endl;
		//设置每个mesh平移旋转缩放的情况，这里只有平移，即第一个分量
		glm::mat4 view = camera.GetViewMatrix();
		view = glm::translate(view, glm::vec1(0.1f) * meshes[i].translation);
		//glm::mat4 view = camera.calculate_lookAt_matrix(meshes[i].translation, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
		//model = glm::rotate(model, glm::radians(40.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.setMat4("model", model);
		shader.setVec3("emissiveFactor", 1.0f, 1.0f, 1.0f);//微弱的环境光
		meshes[i].Draw(shader);
	}
}

//读取变换数组
glm::mat4 Model::readMatrix(glm::mat4 &matrix, json root)
{
	json itemNodes = root["nodes"];
	for (int i = 0; i < itemNodes.size(); i++)//未来 itemNodes.size()大于1时，返回matrix数组
	{
		json curNode = itemNodes.at(i);
		if (curNode.find("matrix") != curNode.end())
		{
			//有matrix节点
			float matrixOri[16];
			for (int i = 0; i < 16; i++)
			{
				matrixOri[i] = curNode["matrix"].at(i);
			}
			//matrix = glm::make_mat4(matrixOri);
			memcpy(glm::value_ptr(matrix), matrixOri, sizeof(matrixOri));
			//glm::mat4 matrix = glm::transpose(matrix);//如果矩阵应该是列有限读取的,按行存储的
		}
	}
	return matrix;
}

//读取indice数组
/*
	fun:
		updateIndicesArray
	in:
		1.int bufferViewIndex: accessorsNode[bufferView]//bufferViews上的第几个元素
		2.int byteOffset: 0 //从未来取出的块中的起始字节byteOffset开始访问
		3.int count: 36 //循环的次数读取type类型数据的次数
		4.int componentTypeSize //最小单元的数据的大小，float，占4个字符
		5.int AccessorElementTypeSize//决定一个节点有几个元素，VEC3，有三个元素，未来读三次
		6.json root //根节点
		7.vector<unsigned short> indices //读好的数据存入indices数组
	out:
		void
	过程：
		将primitives中indices对应的数据，读取后存入indices数组
*/
std::vector<unsigned short> Model::updateIndicesArray(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<unsigned short> &indices, std::string attributeValue)
{
	std::fstream file;
	json bufferViewsNode = root["bufferViews"].at(bufferViewIndex);//取出bufferViews中的第bufferViewIndex的对象
	int bufferIndex = bufferViewsNode["buffer"].get<int32_t>();//用于访问buffers中的第bufferIndex个元素
	std::string uri = root["buffers"].at(bufferIndex)["uri"].get<std::string>();
	uri = "./glTF/" + uri;//添加相对路径
	const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
   //目前文件指针在0的位置
	file.open(binUri, std::ios::in | std::ios::binary);
	int binByteOffset = bufferViewsNode["byteOffset"];//bin文件的起始位置
	//test
	//long size = file.tellg();//当前get 流指针的位置
	//std::cout << "初始流指针的位置:" << size << std::endl;
	//将文件指针移动到binByteOffset的位置
	file.seekg(binByteOffset, std::ios::beg);//把文件的读指针从文件开头向后移1234个字节
	//size = file.tellg();
	//std::cout << "当前bufferView的流指针的位置:" << size << std::endl;
	file.seekg(byteOffset, std::ios::cur);
	//size = file.tellg();
	//std::cout << "当前accessors读取数据的流指针的位置:" << size << std::endl;
	int countIndex = 0;
	while (countIndex < count)
	{
		if (componentType == "UNSIGNED_SHORT")
		{
			unsigned short num;
			file.read(reinterpret_cast<char*>(&num), 2);
			indices.push_back(num);
		}
		/*else if (componentType == "FLOAT")
		{
			float num;
			file.read(reinterpret_cast<char*>(&num), 4);
			indices.push_back(num);
		}*/
		else
		{
			//以后扩展
		}
		countIndex++;
	}
	return indices;
}

//读取顶点相关信息
/*
	fun:
		updateVertexArray
	in:
		1.int bufferViewIndex: accessorsNode[bufferView]//bufferViews上的第几个元素
		2.int byteOffset: 0 //从未来取出的块中的起始字节byteOffset开始访问
		3.int count: 36 //循环的次数读取type类型数据的次数
		4.int componentTypeSize //最小单元的数据的大小，float，占4个字符
		5.int AccessorElementTypeSize//决定一个节点有几个元素，VEC3，有三个元素，未来读三次
		6.json root //根节点
		7.vector<Vertex> vertexArray //读好的数据存入节点通过Vertex加载对应的数据
	out:
		void
	过程：
		将primitives中attributes里的每一个属性（NORMAL、POSITION）从文件中读取后，存入vertexArray
*/
std::vector<Vertex> Model::updateVertexArrayEle(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<Vertex> &vertexArray, std::string attributeValue)
{
	std::fstream file;
	json bufferViewsNode = root["bufferViews"].at(bufferViewIndex);//取出bufferViews中的第bufferViewIndex的对象
	int bufferIndex = bufferViewsNode["buffer"].get<int32_t>();//用于访问buffers中的第bufferIndex个元素
	std::string uri = root["buffers"].at(bufferIndex)["uri"].get<std::string>();
	uri = "./glTF/" + uri;
	const char * binUri = uri.c_str();
	//root["buffers"].at(bufferIndex)["uri"].get<std::string>().c_str();
	//目前文件指针在0的位置
	file.open(binUri, std::ios::in | std::ios::binary);
	int binByteOffset = 0;
	if (bufferViewsNode.find("byteOffset") != bufferViewsNode.end())
	{
		binByteOffset = int(bufferViewsNode["byteOffset"]);//bin文件的起始位置
	}

	//test
	//long size = file.tellg();//当前get 流指针的位置
	//std::cout << "初始流指针的位置:" << size << std::endl;
	//将文件指针移动到binByteOffset的位置
	file.seekg(binByteOffset, std::ios::beg);//把文件的读指针从文件开头向后移1234个字节
	//size = file.tellg();
	//std::cout << "当前bufferView的流指针的位置:" << size << std::endl;
	file.seekg(byteOffset, std::ios::cur);
	//size = file.tellg();
	//std::cout << "当前accessors读取数据的流指针的位置:" << size << std::endl;
	int countIndex = 0;
	while (countIndex < count)
	{
		if (attributeValue == "POSITION")
		{
			//std::cout << "POSITION:" << std::endl;
			if (componentType == "FLOAT")
			{
				std::vector<float> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 4);
				}
				vertexArray[countIndex].Position = glm::vec3(numArray[0], numArray[1], numArray[2]);//此处的Position对应的就是VEC3，可加判断扩展
				//std::cout << "(" << numArray[0] << ", " << numArray[1] << ", " << numArray[2] << ")" << std::endl;
				//std::cout << "position函数测试" << vertexArray[countIndex].Position.x << "," << vertexArray[countIndex].Position.y << "," << vertexArray[countIndex].Position.z << std::endl;
			}
			else if (componentType == "UNSIGNED_SHORT")
			{
				std::vector<unsigned short> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 2);
				}
				vertexArray[countIndex].Position = glm::vec3(numArray[0], numArray[1], numArray[2]);//此处的Position对应的就是VEC3，可加判断扩展
			}
			else
			{
				//以后扩展
			}
		}
		else if (attributeValue == "NORMAL")
		{
			//std::cout << "NORMAL:" << std::endl;
			if (componentType == "FLOAT")
			{
				std::vector<float> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 4);
				}
				vertexArray[countIndex].Normal = glm::vec3(numArray[0], numArray[1], numArray[2]);
				//std::cout << "(" << numArray[0] << ", " << numArray[1] << ", " << numArray[2] << ")" << std::endl;
				//std::cout << "normal函数测试" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
			}
			else if (componentType == "UNSIGNED_SHORT")
			{
				std::vector<unsigned short> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 2);
				}
				vertexArray[countIndex].Normal = glm::vec3(numArray[0], numArray[1], numArray[2]);
			}
			else
			{
				//以后扩展
			}
		}
		else if (attributeValue == "TEXCOORD_0")
		{
			if (componentType == "FLOAT")
			{
				std::vector<float> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 4);
				}
				vertexArray[countIndex].TexCoords = glm::vec2(numArray[0], numArray[1]);
				//std::cout << "(" << numArray[0] << ", " << numArray[1] << ", " << numArray[2] << ")" << std::endl;
				//std::cout << "normal函数测试" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
			}
			else if (componentType == "UNSIGNED_SHORT")
			{
				std::vector<unsigned short> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 2);
				}
				vertexArray[countIndex].TexCoords = glm::vec2(numArray[0], numArray[1]);
			}
			else
			{
				//以后扩展
			}
		}
		else if (attributeValue == "TANGENT")
		{
			if (componentType == "FLOAT")
			{
				std::vector<float> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 4);
				}
				vertexArray[countIndex].Tangent = glm::vec4(numArray[0], numArray[1], numArray[2], numArray[3]);
				//std::cout << "(" << numArray[0] << ", " << numArray[1] << ", " << numArray[2] << ")" << std::endl;
				//std::cout << "normal函数测试" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
			}
			else if (componentType == "UNSIGNED_SHORT")
			{
				std::vector<unsigned short> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 2);
				}
				vertexArray[countIndex].Tangent = glm::vec4(numArray[0], numArray[1], numArray[2], numArray[3]);
			}
			else
			{
				//以后扩展
			}
		}
		else
		{
			//以后扩展
		}
		countIndex++;
		//std::cout << "position函数测试" << vertexArray[countIndex].Position.x << "," << vertexArray[countIndex].Position.y << "," << vertexArray[countIndex].Position.z << std::endl;
		//std::cout << "normal函数测试" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
	}
	return vertexArray;
}

