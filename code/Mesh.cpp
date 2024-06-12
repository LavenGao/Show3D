#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned short> indices, std::vector<Texture> textures, glm::vec3 translation)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->translation = translation;
	setupMesh();
}

void Mesh::setupMesh()
{
	//初始化缓冲，最终使用Draw函数绘制网格
	glGenVertexArrays(1, &VAO);//任何随后的顶点属性调用都会储存在这个VAO中
	glGenBuffers(1, &VBO);//针对缓冲ID：VBO生成一个VBO对象
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);//要想使用VAO，要做的只是使用glBindVertexArray绑定VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//把创建的VBO缓冲对象绑定到GL_ARRAY_BUFFER目标上
	//我们使用的任何（GL_ARRAY_BUFFER目标上的）缓冲调用都会用来配置当前绑定的缓冲（VBO）
	//glBufferData参数：（1）目标缓冲的类型（2）传输数据的大小（3）我们希望发送的实际数据（4）希望显卡如何管理给定的数据（* GL_STATIC_DRAW ：数据不会或几乎不会改变。* GL_DYNAMIC_DRAW：数据会被改变很多。* GL_STREAM_DRAW ：数据每次绘制时都会改变）
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);//把之前定义的顶点数据复制到缓冲的内存中

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// 顶点位置
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer参数：（1）指定我们要配置的顶点属性，具体依据着色器中的定义layout(location = 0)我们希望把数据传递到这一个顶点属性中
	//（2）顶点属性的大小VEC3，大小就是3.（3）数据的类型（4）是否希望数据被标准化(Normalize)，是的话就会映射到0-1之间
	//（5）Stride步长，顶点属性组之间的间隔（6）它表示位置数据在缓冲中起始位置的偏移量
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// 顶点法线
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));//Normal在Vertex结构体中的偏移量
	// 顶点纹理坐标
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	glBindVertexArray(0);
}

void  Mesh::Draw(Shader shader)
{
	//将着色器传入网格类中可以让我们在绘制之前设置一些uniform
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	unsigned int basecolorNr = 1;
	unsigned int metallicRoughnessNr = 1;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// 计算了每个纹理类型的N-分量i，并将其拼接到纹理类型字符串上，来获取对应的uniform名称
		glActiveTexture(GL_TEXTURE0 + i); // 在绑定之前激活相应的纹理单元
		// 获取纹理序号（diffuse_textureN 中的 N）
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		else if (name == "texture_normal")
			number = std::to_string(normalNr++); // transfer unsigned int to stream
		else if (name == "texture_height")
			number = std::to_string(heightNr++); // transfer unsigned int to stream
		else if (name == "texture_basecolor")
			number = std::to_string(basecolorNr++); // transfer unsigned int to stream
		else if (name == "texture_metallicRoughness")
			number = std::to_string(metallicRoughnessNr++); // transfer unsigned int to stream

		//texture_diffuseN/texture_specularN
		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	// 绘制网格
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}

