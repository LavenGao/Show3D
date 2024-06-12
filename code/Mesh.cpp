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
	//��ʼ�����壬����ʹ��Draw������������
	glGenVertexArrays(1, &VAO);//�κ����Ķ������Ե��ö��ᴢ�������VAO��
	glGenBuffers(1, &VBO);//��Ի���ID��VBO����һ��VBO����
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);//Ҫ��ʹ��VAO��Ҫ����ֻ��ʹ��glBindVertexArray��VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//�Ѵ�����VBO�������󶨵�GL_ARRAY_BUFFERĿ����
	//����ʹ�õ��κΣ�GL_ARRAY_BUFFERĿ���ϵģ�������ö����������õ�ǰ�󶨵Ļ��壨VBO��
	//glBufferData��������1��Ŀ�껺������ͣ�2���������ݵĴ�С��3������ϣ�����͵�ʵ�����ݣ�4��ϣ���Կ���ι�����������ݣ�* GL_STATIC_DRAW �����ݲ���򼸺�����ı䡣* GL_DYNAMIC_DRAW�����ݻᱻ�ı�ܶࡣ* GL_STREAM_DRAW ������ÿ�λ���ʱ����ı䣩
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);//��֮ǰ����Ķ������ݸ��Ƶ�������ڴ���

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// ����λ��
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer��������1��ָ������Ҫ���õĶ������ԣ�����������ɫ���еĶ���layout(location = 0)����ϣ�������ݴ��ݵ���һ������������
	//��2���������ԵĴ�СVEC3����С����3.��3�����ݵ����ͣ�4���Ƿ�ϣ�����ݱ���׼��(Normalize)���ǵĻ��ͻ�ӳ�䵽0-1֮��
	//��5��Stride����������������֮��ļ����6������ʾλ�������ڻ�������ʼλ�õ�ƫ����
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// ���㷨��
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));//Normal��Vertex�ṹ���е�ƫ����
	// ������������
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
	//����ɫ�������������п����������ڻ���֮ǰ����һЩuniform
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	unsigned int basecolorNr = 1;
	unsigned int metallicRoughnessNr = 1;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// ������ÿ���������͵�N-����i��������ƴ�ӵ����������ַ����ϣ�����ȡ��Ӧ��uniform����
		glActiveTexture(GL_TEXTURE0 + i); // �ڰ�֮ǰ������Ӧ������Ԫ
		// ��ȡ������ţ�diffuse_textureN �е� N��
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

	// ��������
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}

