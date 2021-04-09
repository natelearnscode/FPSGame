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

using namespace std;

float timePast = 0;
float lastFrameTime = 0;
float deltaTime = 0;
bool fullscreen = false;
int screenWidth = 800;
int screenHeight = 600;

char windowTitle[] = "My OpenGL Program";

float avgRenderTime = 0;

vector<glm::vec3> wallPositions;
glm::vec3 goalPostion;

Camera camera = Camera(screenWidth, screenHeight);

void LoadMap(const char* mapFilePath) {
	cout << "Loading map " << mapFilePath << "..." << endl;
	ifstream mapFile;
	mapFile.open(mapFilePath);
	if (!mapFile) {	// if not a valid input source, abort
		cout << "File: " << mapFilePath << " does not exist. Aborting..." << endl;
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

	//Read map data
	for (int i = 0; i < width; i++) {
		//cout << "i: " << to_string(i) << endl;
		getline(mapFile,line);
		for (int j = 0; j < height; j++) {
			//cout << "i: " << to_string(i) << "j: " << to_string(j) << " val: " << line[j] << endl;
			if (line[j] == '0') {
				continue;
			}
			else if (line[j] == 'W') { //handle walls
				wallPositions.push_back(glm::vec3(i, 0, j));
				printf("%c (%d, 0, %d)\n",line[j] ,i, j);
			}
			else if (line[j] == 'S') { //handle start
				camera.position = glm::vec3(i,0,j);
			}
			else if (line[j] == 'G') { //handle goal

			}
			else if (line[j] == 'A' || line[j] == 'B' || line[j] == 'C' || line[j] == 'D' || line[j] == 'E') {  //handle doors

			}
			else if (line[j] == 'a' || line[j] == 'b' || line[j] == 'c' || line[j] == 'd' || line[j] == 'e') {  //handle keys

			}
		}
		//cout << endl;
	}
	mapFile.close();
	cout << width << " , " << height << endl;
	//TODO: Add walls around the perimeter of map

	//TODO: Load models
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

	//Get map info
	LoadMap("maps/map.txt");

	//Load Model
	ifstream modelFile;
	modelFile.open("models/cube.txt");
	int numLines = 0;
	modelFile >> numLines;
	float* modelData = new float[numLines];
	for (int i = 0; i < numLines; i++) {
		modelFile >> modelData[i];
	}
	printf("Mode line count: %d\n", numLines);
	float numTris = numLines / 8;
	modelFile.close();

	//Loader the shaders
	Shader shader = Shader("shaders/shader.vert", "shaders/shader.frag");
	shader.use(); //Set the active shader (only one can be used at a time)


	//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, numLines * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

  //Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(shader.ID, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	//Attribute, vals/attrib., type, normalized?, stride, offset
	//Binds to VBO current GL_ARRAY_BUFFER 
	glEnableVertexAttribArray(posAttrib);

	//GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);

	GLint normAttrib = glGetAttribLocation(shader.ID, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	glBindVertexArray(0); //Unbind the VAO

	glEnable(GL_DEPTH_TEST);

	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	EventHandler eventHandler;
	bool quit = false;
	while (!quit) {
		float t_start = SDL_GetTicks();
		deltaTime = t_start - lastFrameTime;
		lastFrameTime = t_start;
		float movementSpeed = 0.01f * deltaTime;
		//cout << to_string(speed) << endl;
		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit event loop
		//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
		//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
				fullscreen = !fullscreen;
			if (windowEvent.type == SDL_WINDOWEVENT && windowEvent.window.event == SDL_WINDOWEVENT_RESIZED) { //If window is resized
				cout << "Resizing Window..." << endl;
				screenWidth = windowEvent.window.data1;
				screenHeight = windowEvent.window.data2;
				camera.crossHairX = screenWidth / 2;
				camera.crossHairY = screenHeight / 2;
				aspect = screenWidth / (float)screenHeight;
				glViewport(0, 0, screenWidth, screenHeight);
			}

			eventHandler.handleEvent(windowEvent);

			//Update player movement
			if (eventHandler.moveForward) { //If "w" is pressed
				camera.moveForward(movementSpeed);
			}
			if (eventHandler.moveBackward) { //If "s" is pressed
				camera.moveBackward(movementSpeed);
			}
			if (eventHandler.moveLeft) { //If "a" is pressed
				camera.moveLeft(movementSpeed);
			}
			if (eventHandler.moveRight) { //If "d" is pressed
				camera.moveRight(movementSpeed);
			}

			//Make sure camera is always at ground level
			camera.position.y = 0;

			//Update Camera's look at direction based on mouse position
			camera.updateLookDirection(eventHandler.mouseXPos, eventHandler.mouseYPos);


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
		shader.setUniform("view", view);

		//Set up projection matrix
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 1.0f, 100.0f); //FOV, aspect, near, far
		shader.setUniform("proj", proj);

		glBindVertexArray(vao);
		//Draw Objects
		for (int i = 0; i < wallPositions.size(); i++) {
			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, wallPositions[i]);
			//model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
			//model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
			shader.setUniform("model", model);
			glDrawArrays(GL_TRIANGLES, 0, numTris); //(Primitives, Which VBO, Number of vertices)
		}


		SDL_GL_SwapWindow(window); //Double buffering

		float t_end = SDL_GetTicks();
		char update_title[100];
		float time_per_frame = t_end - t_start;
		avgRenderTime = .98 * avgRenderTime + .02 * time_per_frame; //Weighted average for smoothing
		sprintf(update_title, "%s [Update: %3.0f ms]\n", windowTitle, avgRenderTime);
		SDL_SetWindowTitle(window, update_title);
	}


	//Clean Up
	shader.deleteShader();

	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}