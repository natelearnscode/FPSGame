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

//For Visual Studios
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/string_cast.hpp>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include "shader.h"
using namespace std;

float timePast = 0;
float lastFrameTime = 0;
float deltaTime = 0;
bool fullscreen = false;
int screenWidth = 800;
int screenHeight = 600;

char windowTitle[] = "My OpenGL Program";

float avgRenderTime = 0;

void LoadMap(const char* mapFile) {
	cout << "Loading map..." << endl;
	ifstream map_file;
	map_file.open(mapFile);
	if (!map_file) {	// if not a valid input source, abort
		cout << "File: " << mapFile << " does not exist. Aborting..." << endl;
		exit(0);
	}
	int width = 0;
	int height = 0;
	map_file >> width >> height;
	map_file.close();
	cout << width << " , " << height << endl;
}

SDL_Window* InitSDL() {
	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
							  
	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	return SDL_CreateWindow(windowTitle, 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
	//The above window cannot be resized which makes some code slightly easier.
	//Below show how to make a full screen window or allow resizing
	//SDL_Window* window = SDL_CreateWindow(window_title, 0, 0, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
	//return SDL_CreateWindow(window_title, 100, 100, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
	//SDL_Window* window = SDL_CreateWindow(window_title,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen
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
	LoadMap("map.txt");

	//Load Model
	ifstream modelFile;
	modelFile.open("teapot.txt");
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
	Shader shader = Shader("teapot.vert", "teapot.frag");
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

	glm::vec3 positions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  -2.0f,  0.0f),
		glm::vec3(0.0f,  2.0f,  0.0f)
	};

	glm::vec3 cam_pos = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 cam_dir = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);

	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	while (!quit) {
		float t_start = SDL_GetTicks();
		deltaTime = t_start - lastFrameTime;
		lastFrameTime = t_start;
		float speed = 0.05f * deltaTime;
		cout << to_string(speed) << endl;
		while (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) quit = true; //Exit event loop
		//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
		//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
				fullscreen = !fullscreen;
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_a) { //If "a" is pressed
				cam_pos -= speed * glm::normalize(glm::cross(cam_dir, cam_up));
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_d) { //If "d" is pressed
				cam_pos += speed * glm::normalize(glm::cross(cam_dir, cam_up));
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_w) { //If "w" is pressed
				cam_pos += speed * cam_dir;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_s) { //If "s" is pressed
				cam_pos -= speed * cam_dir;
			}
			SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen 
		}

		// Clear the screen to default color
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		timePast = SDL_GetTicks() / 1000.f;

		//Set up view matrix
		glm::mat4 view = glm::lookAt(
			cam_pos,  //Cam Position
			cam_pos + cam_dir,  //Look at point
			cam_up); //Up
		shader.setUniform("view", view);

		//Set up projection matrix
		glm::mat4 proj = glm::perspective(3.14f / 4, aspect, 1.0f, 100.0f); //FOV, aspect, near, far
		shader.setUniform("proj", proj);

		glBindVertexArray(vao);
		//Draw Objects
		for (int i = 0; i < 3; i++) {
			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, positions[i]);
			model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
			model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
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