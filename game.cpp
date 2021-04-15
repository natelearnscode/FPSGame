#define _CRT_SECURE_NO_WARNINGS

#include "glad/glad.h"  //Include order can matter here
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#include "shader.h"
#include "EventHandler.h"
#include "Camera.h"
#include "Wall.h"
#include "Floor.h"

using namespace std;

float timePast = 0;
float lastFrameTime = 0;
float deltaTime = 0;
bool fullscreen = false;
int screenWidth = 800;
int screenHeight = 600;
glm::vec3 initialPosition;

char windowTitle[] = "My OpenGL Program";

float avgRenderTime = 0;
Wall wallModel;
Floor floorModel;

Camera camera = Camera(screenWidth, screenHeight);

void LoadMap(const char* mapFilePath, Shader* shader) {
	std::cout << "Loading map " << mapFilePath << "..." << endl;
	//Load Models
	wallModel.loadModel(shader);

	ifstream mapFile;
	mapFile.open(mapFilePath);
	if (!mapFile) {	// if not a valid input source, abort
		std::cout << "File: " << mapFilePath << " does not exist. Aborting..." << endl;
		exit(0);
	}
	//Get width and height of the map
	int width = 0;
	int height = 0;
	string line;
	getline(mapFile, line);
	stringstream ss;
	ss << line;
	ss >> width >> height;
	int mapScale = 2;

	std::cout << "Loading floor... " << endl;
	floorModel.loadModel(shader, width, height, mapScale);

	//Add walls to the perimeter of the map
	for (int i = -1; i <= width; i++) {
		wallModel.locations.push_back(glm::vec3(-1 * mapScale, 0, i * mapScale));
		wallModel.locations.push_back(glm::vec3(height * mapScale, 0, i * mapScale));
	}
	for (int i = -1; i <= height; i++) {
		wallModel.locations.push_back(glm::vec3(i * mapScale, 0, -1 * mapScale));
		wallModel.locations.push_back(glm::vec3(i * mapScale, 0, width * mapScale));
	}

	//Format file data to be indexed correctly
	vector<std::string> mapContent;
	while (getline(mapFile, line)) {
		mapContent.push_back(line);
	}
	std::reverse(mapContent.begin(), mapContent.end());

	//Read map data
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//printf("(%d, %d): %c\n", i, j, mapContent[j][i]);
			if (mapContent[j][i] == '0') {
				continue;
			}
			else if (mapContent[j][i] == 'W') { //handle walls
				wallModel.locations.push_back(glm::vec3(j * mapScale, 0, i * mapScale));
			}
			else if (mapContent[j][i] == 'S') { //handle start
				initialPosition = glm::vec3(j * mapScale, 0, i * mapScale);
				camera.position = initialPosition;
			}
			else if (mapContent[j][i] == 'G') { //handle goal

			}
			else if (mapContent[j][i] == 'A' || mapContent[j][i] == 'B' || mapContent[j][i] == 'C' || mapContent[j][i] == 'D' || mapContent[j][i] == 'E') {  //handle doors

			}
			else if (mapContent[j][i] == 'a' || mapContent[j][i] == 'b' || mapContent[j][i] == 'c' || mapContent[j][i] == 'd' || mapContent[j][i] == 'e') {  //handle keys

			}
		}
	}
	mapFile.close();
	std::cout << "done." << endl;
	//exit(0);
}

SDL_Window* InitSDL() {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
							  
	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	return SDL_CreateWindow(windowTitle, 100, 100, screenWidth, screenHeight, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
}

int main(int argc, char* argv[]) {
	float aspect = screenWidth / (float)screenHeight; //aspect ratio (needs to be updated if the window is resized)
	SDL_Window* window = InitSDL();
	SDL_GLContext context = SDL_GL_CreateContext(window);	//Create a context to draw in

	//Load OpenGL using glad
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	//Loader the shaders
	Shader* shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
	shader->use(); //Set the active shader (only one can be used at a time)

	//Load map from text file
	LoadMap("maps/map.txt", shader);

	glEnable(GL_DEPTH_TEST);

	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	EventHandler eventHandler;
	bool quit = false;
	while (!quit) {
		float t_start = SDL_GetTicks();
		deltaTime = t_start - lastFrameTime;
		lastFrameTime = t_start;
		float movementSpeed = 0.003f * deltaTime;
		float cameraSensitivity = 0.5f;
		//printf("player position (%f, %f)\n", camera.position.x, camera.position.z);
		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
				fullscreen = !fullscreen;
			if (windowEvent.type == SDL_WINDOWEVENT && windowEvent.window.event == SDL_WINDOWEVENT_RESIZED) { //If window is resized
				std::cout << "Resizing Window..." << endl;
				screenWidth = windowEvent.window.data1;
				screenHeight = windowEvent.window.data2;
				camera.crossHairX = screenWidth / 2;
				camera.crossHairY = screenHeight / 2;
				aspect = screenWidth / (float)screenHeight;
				glViewport(0, 0, screenWidth, screenHeight);
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_r) //If "r" is pressed reset player position
				camera.position = initialPosition;
			eventHandler.handleEvent(windowEvent);

			//Update player movement
			float radius = 0.25f;
			if (eventHandler.moveForward) { //If "w" is pressed
				glm::vec3 newPosition = camera.moveForward(movementSpeed);
				bool wallCollision = wallModel.checkCollision(newPosition, radius);
				if (!wallCollision) {
					camera.position = newPosition;
				}
			}
			if (eventHandler.moveBackward) { //If "s" is pressed
				glm::vec3 newPosition = camera.moveBackward(movementSpeed);
				bool wallCollision = wallModel.checkCollision(newPosition, radius);
				if (!wallCollision) {
					camera.position = newPosition;
				}
			}
			if (eventHandler.moveLeft) { //If "a" is pressed
				glm::vec3 newPosition = camera.moveLeft(movementSpeed);
				bool wallCollision = wallModel.checkCollision(newPosition, radius);
				if (!wallCollision) {
					camera.position = newPosition;
				}
			}
			if (eventHandler.moveRight) { //If "d" is pressed
				glm::vec3 newPosition = camera.moveRight(movementSpeed);
				bool wallCollision = wallModel.checkCollision(newPosition, radius);
				if (!wallCollision) {
					camera.position = newPosition;
				}
			}

			//Calculate Gravity

			//Make sure camera is always at ground level
			camera.position.y = 0;

			//Update Camera's look direction based on mouse position
			camera.updateLookDirection(eventHandler.mouseXPos, eventHandler.mouseYPos, cameraSensitivity);


			SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen 
		}

		// Clear the screen to default color
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		timePast = SDL_GetTicks() / 1000.f;

		//Set up view matrix
		glm::mat4 view = glm::lookAt(
			camera.position,  //Cam Position
			camera.position + camera.direction,  //Look at point
			camera.up); //Up
		shader->setUniform("view", view);

		//Set up projection matrix
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f); //FOV, aspect, near, far
		shader->setUniform("proj", proj);

		//Draw Objects
		wallModel.draw();
		floorModel.draw();

		SDL_GL_SwapWindow(window); //Double buffering

		float t_end = SDL_GetTicks();
		char update_title[100];
		float time_per_frame = t_end - t_start;
		avgRenderTime = .98 * avgRenderTime + .02 * time_per_frame; //Weighted average for smoothing
		sprintf(update_title, "%s [Update: %3.0f ms]\n", windowTitle, avgRenderTime);
		SDL_SetWindowTitle(window, update_title);
	}


	//Clean Up
	shader->deleteShader();
	wallModel.cleanUp();
	floorModel.cleanUp();

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}