#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public:
	glm::vec3 position = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	float sensitivity = 0.5f;
	int crossHairX = 0;
	int crossHairY = 0;


	Camera(int w, int h) : crossHairX(w/2), crossHairY(h/2) {};

	void moveForward(float movementSpeed) {
		position += movementSpeed * direction;
	}

	void moveBackward(float movementSpeed) {
		position -= movementSpeed * direction;
	}

	void moveLeft(float movementSpeed) {
		position -= movementSpeed * glm::normalize(glm::cross(direction, up));
	}

	void moveRight(float movementSpeed) {
		position += movementSpeed * glm::normalize(glm::cross(direction, up));
	}

	void updateLookDirection(int currentMouseX, int currentMouseY) {
		float yaw = 0;
		float pitch = 0;

		float deltaX = (currentMouseX - crossHairX) * sensitivity;
		float deltaY = (currentMouseY - crossHairY) * sensitivity;
		yaw += deltaX;
		pitch += deltaY;

		//Make sure player doesn't look too high up or too low
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		printf("Angle Yaw: %f, Pitch: %f\n", -yaw, pitch);

		glm::vec3 newDirection(
			cos(glm::radians(yaw)) * cos(glm::radians(-pitch)),
			sin(glm::radians(-pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(-pitch))
		);
		direction = glm::normalize(newDirection);
	}
};
#endif