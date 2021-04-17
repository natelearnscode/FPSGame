#ifndef CAMERA_H
#define CAMERA_H
#include <map>

class Camera {
public:
	glm::vec3 position = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	bool isCarryingKey = false;
	char key = '0';
	std::map<char, bool> inventory;
	//float sensitivity = 0.5f;
	int crossHairX = 0;
	int crossHairY = 0;
	bool jumping = false;
	float upVelocity = 0;
	float jumpStartTime = 0;
	float jumpCurrentTime = 0;
	float movementSpeed = 0;
	float yaw = 0;
	float pitch = 0;

	Camera(int w, int h) : crossHairX(w/2), crossHairY(h/2) {};

	glm::vec3 moveForward() {
		return position + movementSpeed * direction;
	}

	glm::vec3 moveBackward() {
		return position - movementSpeed * direction;
	}

	glm::vec3 moveLeft() {
		return position - movementSpeed * glm::normalize(glm::cross(direction, up));
	}

	glm::vec3 moveRight() {
		return position + movementSpeed * glm::normalize(glm::cross(direction, up));
	}

	void startJump(float startTime) {
		jumping = true;
		jumpStartTime = startTime;
		jumpCurrentTime = startTime;
		upVelocity = 0.25;
	}

	void processGravity(float deltaTime) {
		if (!jumping) {
			position.y = 0;
			return;
		}
		if (position.y < 0) {
			position.y = 0;
			jumping = false;
			jumpCurrentTime = 0;
			jumpStartTime = 0;
			upVelocity = 0;
			return;
		}
		jumpCurrentTime += deltaTime;
		float dt = jumpCurrentTime - jumpStartTime;
		float gravity = -0.00001f;
		//printf("current pos %f new position %f velocity %f gravity %f \n", position.y, position.y + upVelocity, upVelocity, gravity * dt);
		upVelocity = upVelocity + gravity * dt;
		float newY = position.y + upVelocity;
		position.y = newY;
	}

	void updateLookDirection(int currentMouseX, int currentMouseY, float sensitivity) {
		yaw = 0;
		pitch = 0;

		float deltaX = (currentMouseX - crossHairX) * sensitivity;
		float deltaY = (currentMouseY - crossHairY) * sensitivity;
		yaw += deltaX;
		pitch += deltaY;

		//Make sure player doesn't look too high up or too low
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		//printf("Angle Yaw: %f, Pitch: %f\n", -yaw, pitch);

		glm::vec3 newDirection(
			cos(glm::radians(yaw)) * cos(glm::radians(-pitch)),
			sin(glm::radians(-pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(-pitch))
		);
		direction = glm::normalize(newDirection);
	}
};
#endif