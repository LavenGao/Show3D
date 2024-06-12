#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
//const float YAW = 0.0f;

const float PITCH = 20.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;//相机位置
	glm::vec3 Front;//摄像机方向，原点-摄像机位置
	glm::vec3 Up;//相机上轴
	glm::vec3 Right;//相机右轴
	glm::vec3 WorldUp;//世界上
	// euler Angles
	float Yaw;//偏航角，往左和往右看的程度
	float Pitch;//俯仰角,往上或往下看的角
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;//缩放(Zoom)接口
	// constructor with vectors,构造函数后跟（冒号和类成员）表示先对冒号后的类成员（参数中的那个）进行初始化，然后做为冒号前类的成员。分别用括号中的值去初始化括号前面的值
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	// constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
	// returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix();
	// Custom implementation of the LookAt function
	glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp);
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset);
private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};
#endif