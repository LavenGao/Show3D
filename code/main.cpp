#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "Model.h"
#include "Camera.h"

#include <gl/GL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Json/json.hpp>

#include <iostream>

bool captureMouse = true;  // ���ڸ��ٵ�ǰ�Ƿ񲶻����

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
// extern 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));//�������������
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

extern std::map<std::string, int> AccessorElementSizeMap = {
	{"SCALAR",1},
	{"VEC2",2},{"VEC3",3}, {"VEC4",4},
	{"MAT2",2}, {"MAT3",3},{"MAT4",4}
};
extern std::map<int, std::string> AccessorComponentTypeMap = {
	{5123, "UNSIGNED_SHORT"},
	{5126, "FLOAT"}
};

int main()
{
	//-------------------------------------Part1 Begin: glfw ��ʼ�����������--------------------------------------------------
	//��ʼ��GLFW������
	glfwInit();
	//ʹ��glfwWindowHint����������GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�����汾��(Major)��Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�ΰ汾��(Minor)��Ϊ3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����GLFW����ʹ�õ��Ǻ���ģʽ(Core-profile)����ζ������ֻ��ʹ��OpenGL���ܵ�һ���Ӽ�


	//-------------------------------------Part2 Begin: windows���ڴ���--------------------------------------------------------
	//����һ�����ڶ���������ڶ����������кʹ�����ص����ݣ����һᱻGLFW����������Ƶ�����õ�
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//�����Ǵ��ڵ�����������Ϊ��ǰ�̵߳���������
	glfwMakeContextCurrent(window);
	//ע���������������GLFW����ϣ��ÿ�����ڵ�����С��ʱ������������
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//-------------------------------------Part3 Begin: glad����opengl��ָ��-----------------------------------------------------
	//GLAD����������OpenGL�ĺ���ָ��ģ������ڵ����κ�OpenGL�ĺ���֮ǰ������Ҫ��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	//stbi_set_flip_vertically_on_load(true);
	// -----------------------------Part4 Begin��configure global opengl state ����ȫ�ֵ�opengl״̬-----------------------------
	glEnable(GL_DEPTH_TEST);

	//-------------------------------------Part5 Begin: ����������ɫ������------------------------------------------------
	//build and compile our shader program
	//vertex shader
	// set up vertex data (and buffer(s)) and configure vertex attributes
	//Part 5.1 ������ɫ��
	//Part 5.2 Ƭ����ɫ��
	//Part 5.3  ������ɫ��
	Shader ourShader("./shaders/shader.vs", "./shaders/shader.fs");

	Model ourModel;
	std::string gltfPath = "./glTF/Lantern.gltf";
	json root = ourModel.gltfToJsonRoot(gltfPath);
	ourModel.updateWholeMesh(root);//��ȡ���������Ϣ(ȫ��),����Mesh����

	//-------------------------------------Part6 Begin: ��Ⱦѭ��(Render Loop)-----------------------------------------------------
	//��Ⱦѭ��(Render Loop)
	while (!glfwWindowShouldClose(window))//ÿ��ѭ���Ŀ�ʼǰ���һ��GLFW�Ƿ�Ҫ���˳�������ǵĻ��ú�������trueȻ����Ⱦѭ��������ˣ�֮��Ϊ���ǾͿ��Թر�Ӧ�ó����ˡ�
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;// ��ǰ֡����һ֡��ʱ���
		lastFrame = currentFrame;// ��һ֡��ʱ��

		// input
		processInput(window);

		// ��Ⱦָ��
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);//�����ɫ���壬��ɫ���ó�getClearColor����ɫ
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the depth buffer 

		//������ɫ��
		ourShader.use();

		// pass projection matrix to shader
		//perspective(1)��׶������֮��ļнǣ�(2)��߱ȣ����Ӵ��Ŀ� / �ߣ�(3)���������ȣ���4��Զ��������
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		//�����������á���ɫ���÷�װ��Draw���� 
		ourModel.Draw(ourShader, camera);

		glfwSwapBuffers(window);//������ɫ���壨����һ��������GLFW����ÿһ��������ɫֵ�Ĵ󻺳壩��������һ�����б��������ƣ����ҽ�����Ϊ�����ʾ����Ļ�ϡ�
		glfwPollEvents();//���������û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬�������ö�Ӧ�Ļص�����������ͨ���ص������ֶ����ã���
	}

	//-------------------------------------Part7 Begin: ��ѡ��һ�������������ڣ��������-----------------------------------------------------
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);


	//-------------------------------------Part8 Begin: glfw��ֹ������֮ǰ�������Դ-----------------------------------------------------
	glfwTerminate();//����Ⱦѭ��������������Ҫ��ȷ�ͷ�/ɾ��֮ǰ�ķ����������Դ

	return 0;
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// ��������� F �����л��������״̬
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		captureMouse = !captureMouse;  // �л�����״̬
		if (captureMouse)
		{
			// ����Ϊ�������
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			// ����Ϊ�������ģʽ
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	// ���������̵Ĳ����������Լ�������

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);

}