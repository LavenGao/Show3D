#include "Model.h"

/*
*��ȡgltf�ļ�������ת��Ϊjson����
*in:string gltfPath
*out:json root
*/
json Model::gltfToJsonRoot(std::string gltfPath)
{
	json root;
	std::ifstream in(gltfPath, std::ios::binary);
	if (!in.is_open())
	{
		std::cout << "Error opening file\n";//�ļ�δ�ɹ���
		//return;
	}
	in >> root;//��Json�ļ�����JsonContent����Json����
	return root;
}

//��ȡtranslate����
void Model::updateTranslate(json root, int meshNum, glm::vec3 &translation)
{
	//������//gltfת��jsonԪ�ص�˳��ᷢ���ı䣬����Ҫ����nodes�еĽڵ��ж�mesh��Ӧ��ֵ�ǲ��ǵ��ڵ�ǰ��meshNum
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

//����һ��mesh�������Ϣ 
void Model::updateMeshParameter(json root, int meshNum)
{
	json node = root["meshes"].at(meshNum);
	node = node["primitives"];
	node = node[0];

	if (node.is_object())
	{
		//����obj�е�ÿ��Ԫ��
		for (json::iterator it = node.begin(); it != node.end(); it++)
		{
			if (it.key() == "attributes")
			{
				//std::cout << it.key()<< "\n";
				//��ǰkeyΪ"attributes"����������value
				json curNode = it.value();
				json accessorsNode = root["accessors"];  //attribute�����Զ���ʹ��accesors��ȡ��
				int positionIndex = curNode["POSITION"];//һ������POSITION��ȡPOSITION��Ӧ��accessors��λ��
				json tempNode = accessorsNode[positionIndex];
				int VertexArrayCount = tempNode["count"];//��ǰ�����vertex����Ĵ�С

				//����VertexArrayCount��С�Ķ�̬����,attributes�����Խ��ᱣ�浽vertex���У�vertex.position��vertex.Normal�ȣ�
				std::vector<Vertex> vertexArray(VertexArrayCount);
				vertices = vertexArray;
				int testCount = 0;
				for (json::iterator iSec = curNode.begin(); iSec != curNode.end(); iSec++)
				{
					int curNum = iSec.value();//��ȡ��ǰ���ԣ�eg.POSITION����accessors�е�λ��
					json singleAcessorNode = accessorsNode[curNum];;//��ȡaccessors�е�ǰ���ԣ�eg.POSITION)��Ӧ��json OBJ
					int bufferViewIndex = singleAcessorNode["bufferView"].get<int>();//��ǰ���ԣ�eg.POSITION)��bufferView�е�����
					int byteOffset = 0;//��ǰ���ԣ�eg.POSITION)��bufferView��Ӧ���ڴ��е���ʼƫ����
					if (singleAcessorNode.find("byteOffset") != singleAcessorNode.end())
					{
						//box�������key
						int byteOffset = singleAcessorNode["byteOffset"].get<int>();
					}
					else
					{
						//latern��û��byteOffset���key,û�о�Ĭ�ϴ�0��
						//int byteOffset = 0;
					}
					std::string componentType = AccessorComponentTypeMap[int(singleAcessorNode["componentType"])];//float,5126
					int AccessorElementTypeSize = AccessorElementSizeMap[singleAcessorNode["type"].get<std::string>()];//4
					//����vertices�����е�POSITION����
					testCount++;
					vertices = updateVertexArrayEle(bufferViewIndex, byteOffset, VertexArrayCount, componentType, AccessorElementTypeSize, root, vertices, iSec.key());
				}
				//std::cout <<"����������"<< vertices[5].Position.x<<"," << vertices[5].Position.y << "," << vertices[5].Position.z <<std::endl;
				//std::cout << "����������" << vertices[5].Normal.x << "," << vertices[5].Normal.y << "," << vertices[5].Normal.z << std::endl;

			}
			else if (it.key() == "indices")
			{
				//��value�е�ÿ���������vector<unsigned int> indices;
				//std::cout << it.key() << "\n";
				//��ǰkeyΪ"indices"��ȡindices��Ӧ����accessors�е�λ��
				json accessorsNode = root["accessors"];
				int positionIndex = it.value();//ȡindices��Ӧ����accessors�е�λ��
				json accessorsIndiceNode = accessorsNode[positionIndex];
				int indicesArrayCount = accessorsIndiceNode["count"];
				//std::vector<unsigned short> indices(indicesArrayCount);//////////////////////////////////////////////////
				int bufferViewIndex = accessorsIndiceNode["bufferView"].get<int>();//bufferViews�еĵڼ���
				int byteOffset = 0;//��ǰ���ԣ�eg.POSITION)��bufferView��Ӧ���ڴ��е���ʼƫ����
				if (accessorsIndiceNode.find("byteOffset") != accessorsIndiceNode.end())
				{
					//box�������key
					int byteOffset = accessorsIndiceNode["byteOffset"].get<int>();//indices�����ڶ�ȡ�����ڴ���е���ʼλ��
				}
				else
				{
					//latern��û��byteOffset���key,û�о�Ĭ�ϴ�0��
					//int byteOffset = 0;
				}

				std::string componentType = AccessorComponentTypeMap[int(accessorsIndiceNode["componentType"])];//�˴�����5123ȡ��UNSIGNED_SHORT
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
				int materialIndex = it.value();//ȡmaterial��Ӧ����materials�е�λ��
				json materialsNode = root["materials"];
				json singleMaterialNode = materialsNode[materialIndex];//���ﶼ��0

				loadMaterialTextures(root, singleMaterialNode);

			}
		}
	}
	//��ȡtranslate����
	updateTranslate(root, meshNum, translation);
	meshes.push_back(Mesh(vertices, indices, textures, translation));
}

//����һ��������Ϣ
void Model::loadMaterialTextures(json root, json singleMaterialNode)
{
	//����materials�е�key���洢����Ӧ����ͼ
	for (json::iterator it = singleMaterialNode.begin(); it != singleMaterialNode.end(); it++)
	{
		if (it.key() == "pbrMetallicRoughness")
		{
			//��ֻ����baseColorTexture
			json basecolorNode1 = it.value()["baseColorTexture"];
			int textureIndex1 = basecolorNode1["index"];
			json textureNode1 = root["textures"].at(textureIndex1);
			int imageIndex1 = textureNode1["source"];
			json imageNode1 = root["images"].at(imageIndex1);
			std::string uri1 = imageNode1["uri"].get<std::string>();
			uri1 = "./glTF/" + uri1;//������·��
			const char * binUri1 = uri1.c_str();//"./glTF/Lantern.bin";
		   //Ŀǰ�ļ�ָ����0��λ��
			//std::fstream file;
			//file.open(binUri, std::ios::in | std::ios::binary);
			Texture texture1;
			texture1.id = TextureFromFile(binUri1);
			texture1.type = "texture_basecolor";
			texture1.path = binUri1;
			textures.push_back(texture1);

			//�ٿ���metallicRoughnessTexture
			json basecolorNode2 = it.value()["metallicRoughnessTexture"];
			int textureIndex2 = basecolorNode2["index"];
			json textureNode2 = root["textures"].at(textureIndex2);
			int imageIndex2 = textureNode2["source"];
			json imageNode2 = root["images"].at(imageIndex2);
			std::string uri2 = imageNode2["uri"].get<std::string>();
			uri2 = "./glTF/" + uri2;//������·��
			const char * binUri2 = uri2.c_str();//"./glTF/Lantern.bin";
			Texture texture2;
			texture2.id = TextureFromFile(binUri2);
			texture2.type = "texture_metallicRoughness";
			texture2.path = binUri2;
			textures.push_back(texture2);
		}
		else if (it.key() == "emissiveTexture")
		{
			//���emissiveFactorһ��ʹ��
			json emissiveTextureNode = it.value();
			int emissiveTextureIndex = emissiveTextureNode["index"];
			json textureNode = root["textures"].at(emissiveTextureIndex);
			int imageIndex = textureNode["source"];
			json imageNode = root["images"].at(imageIndex);
			std::string uri = imageNode["uri"].get<std::string>();
			uri = "./glTF/" + uri;//������·��
			const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
			Texture texture;
			texture.id = TextureFromFile(binUri);
			texture.type = "texture_emissive";
			texture.path = binUri;
			textures.push_back(texture);
		}
		else if (it.key() == "normalTexture")
		{
			//������ͼ
			json normalTextureNode = it.value();
			int normalTextureIndex = normalTextureNode["index"];
			json textureNode = root["textures"].at(normalTextureIndex);
			int imageIndex = textureNode["source"];
			json imageNode = root["images"].at(imageIndex);
			std::string uri = imageNode["uri"].get<std::string>();
			uri = "./glTF/" + uri;//������·��
			const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
			Texture texture;
			texture.id = TextureFromFile(binUri);
			texture.type = "texture_normal";
			texture.path = binUri;
			textures.push_back(texture);
		}
		else
		{
			//����չ
		}

	}

}

//����������ͼ��Ӧ������ID
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

//����ȫ��mesh�ڵ㣬���忴children���м������ӣ����м�������
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
	//����meshSize��meshes����
	for (int i = 0; i < meshSize; i++)
	{
		updateMeshParameter(root, i);
		updateTranslate(root, i, translation);
	}
}

//�������������񣬲��������Ǹ��Ե�Draw����
void Model::Draw(Shader &shader, Camera camera)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		//std::cout << i << std::endl;
		//����ÿ��meshƽ����ת���ŵ����������ֻ��ƽ�ƣ�����һ������
		glm::mat4 view = camera.GetViewMatrix();
		view = glm::translate(view, glm::vec1(0.1f) * meshes[i].translation);
		//glm::mat4 view = camera.calculate_lookAt_matrix(meshes[i].translation, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		shader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
		//model = glm::rotate(model, glm::radians(40.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.setMat4("model", model);
		shader.setVec3("emissiveFactor", 1.0f, 1.0f, 1.0f);//΢���Ļ�����
		meshes[i].Draw(shader);
	}
}

//��ȡ�任����
glm::mat4 Model::readMatrix(glm::mat4 &matrix, json root)
{
	json itemNodes = root["nodes"];
	for (int i = 0; i < itemNodes.size(); i++)//δ�� itemNodes.size()����1ʱ������matrix����
	{
		json curNode = itemNodes.at(i);
		if (curNode.find("matrix") != curNode.end())
		{
			//��matrix�ڵ�
			float matrixOri[16];
			for (int i = 0; i < 16; i++)
			{
				matrixOri[i] = curNode["matrix"].at(i);
			}
			//matrix = glm::make_mat4(matrixOri);
			memcpy(glm::value_ptr(matrix), matrixOri, sizeof(matrixOri));
			//glm::mat4 matrix = glm::transpose(matrix);//�������Ӧ���������޶�ȡ��,���д洢��
		}
	}
	return matrix;
}

//��ȡindice����
/*
	fun:
		updateIndicesArray
	in:
		1.int bufferViewIndex: accessorsNode[bufferView]//bufferViews�ϵĵڼ���Ԫ��
		2.int byteOffset: 0 //��δ��ȡ���Ŀ��е���ʼ�ֽ�byteOffset��ʼ����
		3.int count: 36 //ѭ���Ĵ�����ȡtype�������ݵĴ���
		4.int componentTypeSize //��С��Ԫ�����ݵĴ�С��float��ռ4���ַ�
		5.int AccessorElementTypeSize//����һ���ڵ��м���Ԫ�أ�VEC3��������Ԫ�أ�δ��������
		6.json root //���ڵ�
		7.vector<unsigned short> indices //���õ����ݴ���indices����
	out:
		void
	���̣�
		��primitives��indices��Ӧ�����ݣ���ȡ�����indices����
*/
std::vector<unsigned short> Model::updateIndicesArray(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<unsigned short> &indices, std::string attributeValue)
{
	std::fstream file;
	json bufferViewsNode = root["bufferViews"].at(bufferViewIndex);//ȡ��bufferViews�еĵ�bufferViewIndex�Ķ���
	int bufferIndex = bufferViewsNode["buffer"].get<int32_t>();//���ڷ���buffers�еĵ�bufferIndex��Ԫ��
	std::string uri = root["buffers"].at(bufferIndex)["uri"].get<std::string>();
	uri = "./glTF/" + uri;//������·��
	const char * binUri = uri.c_str();//"./glTF/Lantern.bin";
   //Ŀǰ�ļ�ָ����0��λ��
	file.open(binUri, std::ios::in | std::ios::binary);
	int binByteOffset = bufferViewsNode["byteOffset"];//bin�ļ�����ʼλ��
	//test
	//long size = file.tellg();//��ǰget ��ָ���λ��
	//std::cout << "��ʼ��ָ���λ��:" << size << std::endl;
	//���ļ�ָ���ƶ���binByteOffset��λ��
	file.seekg(binByteOffset, std::ios::beg);//���ļ��Ķ�ָ����ļ���ͷ�����1234���ֽ�
	//size = file.tellg();
	//std::cout << "��ǰbufferView����ָ���λ��:" << size << std::endl;
	file.seekg(byteOffset, std::ios::cur);
	//size = file.tellg();
	//std::cout << "��ǰaccessors��ȡ���ݵ���ָ���λ��:" << size << std::endl;
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
			//�Ժ���չ
		}
		countIndex++;
	}
	return indices;
}

//��ȡ���������Ϣ
/*
	fun:
		updateVertexArray
	in:
		1.int bufferViewIndex: accessorsNode[bufferView]//bufferViews�ϵĵڼ���Ԫ��
		2.int byteOffset: 0 //��δ��ȡ���Ŀ��е���ʼ�ֽ�byteOffset��ʼ����
		3.int count: 36 //ѭ���Ĵ�����ȡtype�������ݵĴ���
		4.int componentTypeSize //��С��Ԫ�����ݵĴ�С��float��ռ4���ַ�
		5.int AccessorElementTypeSize//����һ���ڵ��м���Ԫ�أ�VEC3��������Ԫ�أ�δ��������
		6.json root //���ڵ�
		7.vector<Vertex> vertexArray //���õ����ݴ���ڵ�ͨ��Vertex���ض�Ӧ������
	out:
		void
	���̣�
		��primitives��attributes���ÿһ�����ԣ�NORMAL��POSITION�����ļ��ж�ȡ�󣬴���vertexArray
*/
std::vector<Vertex> Model::updateVertexArrayEle(int bufferViewIndex, int byteOffset, int count, std::string componentType, int AccessorElementTypeSize, json root, std::vector<Vertex> &vertexArray, std::string attributeValue)
{
	std::fstream file;
	json bufferViewsNode = root["bufferViews"].at(bufferViewIndex);//ȡ��bufferViews�еĵ�bufferViewIndex�Ķ���
	int bufferIndex = bufferViewsNode["buffer"].get<int32_t>();//���ڷ���buffers�еĵ�bufferIndex��Ԫ��
	std::string uri = root["buffers"].at(bufferIndex)["uri"].get<std::string>();
	uri = "./glTF/" + uri;
	const char * binUri = uri.c_str();
	//root["buffers"].at(bufferIndex)["uri"].get<std::string>().c_str();
	//Ŀǰ�ļ�ָ����0��λ��
	file.open(binUri, std::ios::in | std::ios::binary);
	int binByteOffset = 0;
	if (bufferViewsNode.find("byteOffset") != bufferViewsNode.end())
	{
		binByteOffset = int(bufferViewsNode["byteOffset"]);//bin�ļ�����ʼλ��
	}

	//test
	//long size = file.tellg();//��ǰget ��ָ���λ��
	//std::cout << "��ʼ��ָ���λ��:" << size << std::endl;
	//���ļ�ָ���ƶ���binByteOffset��λ��
	file.seekg(binByteOffset, std::ios::beg);//���ļ��Ķ�ָ����ļ���ͷ�����1234���ֽ�
	//size = file.tellg();
	//std::cout << "��ǰbufferView����ָ���λ��:" << size << std::endl;
	file.seekg(byteOffset, std::ios::cur);
	//size = file.tellg();
	//std::cout << "��ǰaccessors��ȡ���ݵ���ָ���λ��:" << size << std::endl;
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
				vertexArray[countIndex].Position = glm::vec3(numArray[0], numArray[1], numArray[2]);//�˴���Position��Ӧ�ľ���VEC3���ɼ��ж���չ
				//std::cout << "(" << numArray[0] << ", " << numArray[1] << ", " << numArray[2] << ")" << std::endl;
				//std::cout << "position��������" << vertexArray[countIndex].Position.x << "," << vertexArray[countIndex].Position.y << "," << vertexArray[countIndex].Position.z << std::endl;
			}
			else if (componentType == "UNSIGNED_SHORT")
			{
				std::vector<unsigned short> numArray(AccessorElementTypeSize);
				for (int i = AccessorElementTypeSize; i > 0; i--)
				{
					file.read(reinterpret_cast<char*>(&numArray[AccessorElementTypeSize - i]), 2);
				}
				vertexArray[countIndex].Position = glm::vec3(numArray[0], numArray[1], numArray[2]);//�˴���Position��Ӧ�ľ���VEC3���ɼ��ж���չ
			}
			else
			{
				//�Ժ���չ
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
				//std::cout << "normal��������" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
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
				//�Ժ���չ
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
				//std::cout << "normal��������" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
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
				//�Ժ���չ
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
				//std::cout << "normal��������" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
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
				//�Ժ���չ
			}
		}
		else
		{
			//�Ժ���չ
		}
		countIndex++;
		//std::cout << "position��������" << vertexArray[countIndex].Position.x << "," << vertexArray[countIndex].Position.y << "," << vertexArray[countIndex].Position.z << std::endl;
		//std::cout << "normal��������" << vertexArray[countIndex].Normal.x << "," << vertexArray[countIndex].Normal.y << "," << vertexArray[countIndex].Normal.z << std::endl;
	}
	return vertexArray;
}

