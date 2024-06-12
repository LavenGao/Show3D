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

bool captureMouse = true;  // 用于跟踪当前是否捕获鼠标

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
// extern 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));//相机的世界坐标
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
	//-------------------------------------Part1 Begin: glfw 初始化和相关配置--------------------------------------------------
	//初始化GLFW和配置
	glfwInit();
	//使用glfwWindowHint函数来配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//将主版本号(Major)设为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//次版本号(Minor)设为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//告诉GLFW我们使用的是核心模式(Core-profile)。意味着我们只能使用OpenGL功能的一个子集


	//-------------------------------------Part2 Begin: windows窗口创建--------------------------------------------------------
	//创建一个窗口对象，这个窗口对象存放了所有和窗口相关的数据，而且会被GLFW的其他函数频繁地用到
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//将我们窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);
	//注册这个函数，告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//-------------------------------------Part3 Begin: glad加载opengl的指针-----------------------------------------------------
	//GLAD是用来管理OpenGL的函数指针的，所以在调用任何OpenGL的函数之前我们需要初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	//stbi_set_flip_vertically_on_load(true);
	// -----------------------------Part4 Begin：configure global opengl state 配置全局的opengl状态-----------------------------
	glEnable(GL_DEPTH_TEST);

	//-------------------------------------Part5 Begin: 构建编译着色器程序------------------------------------------------
	//build and compile our shader program
	//vertex shader
	// set up vertex data (and buffer(s)) and configure vertex attributes
	//Part 5.1 顶点着色器
	//Part 5.2 片段着色器
	//Part 5.3  链接着色器
	Shader ourShader("./shaders/shader.vs", "./shaders/shader.fs");

	Model ourModel;
	std::string gltfPath = "./glTF/Lantern.gltf";
	json root = ourModel.gltfToJsonRoot(gltfPath);
	ourModel.updateWholeMesh(root);//读取顶点相关信息(全部),存入Mesh数组

	//-------------------------------------Part6 Begin: 渲染循环(Render Loop)-----------------------------------------------------
	//渲染循环(Render Loop)
	while (!glfwWindowShouldClose(window))//每次循环的开始前检查一次GLFW是否被要求退出，如果是的话该函数返回true然后渲染循环便结束了，之后为我们就可以关闭应用程序了。
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;// 当前帧与上一帧的时间差
		lastFrame = currentFrame;// 上一帧的时间

		// input
		processInput(window);

		// 渲染指令
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT);//清除颜色缓冲，颜色设置成getClearColor的颜色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the depth buffer 

		//激活着色器
		ourShader.use();

		// pass projection matrix to shader
		//perspective(1)视锥上下面之间的夹角，(2)宽高比，即视窗的宽 / 高，(3)近截面的深度，（4）远界面的深度
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		//其他矩阵设置、颜色设置封装在Draw函数 
		ourModel.Draw(ourShader, camera);

		glfwSwapBuffers(window);//交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。
		glfwPollEvents();//函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
	}

	//-------------------------------------Part7 Begin: 可选，一旦超出生命周期，解除分配-----------------------------------------------------
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);


	//-------------------------------------Part8 Begin: glfw终止，清理之前分配的资源-----------------------------------------------------
	glfwTerminate();//当渲染循环结束后我们需要正确释放/删除之前的分配的所有资源

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

	// 如果按下了 F 键，切换捕获鼠标状态
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		captureMouse = !captureMouse;  // 切换捕获状态
		if (captureMouse)
		{
			// 设置为捕获鼠标
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			// 设置为正常鼠标模式
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	// 其他鼠标键盘的操作，都可以加在这里

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