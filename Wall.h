#ifndef WALL_H
#define WALL_H
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "OBJFileParser.h"

using namespace std;

class Wall {
public:
	vector<glm::vec3> locations;
	Shader* shader;
	OBJFileParser objFileParser;
	float* modelData;
	GLuint vao;
	GLuint vbo[1];
	unsigned int texture;
	int numLines = 0;
	float numTris = 0;
	float wallRadius = 1.0f;


	Wall() {};

	bool checkCollision(glm::vec3 position, float playerRadius) {
		bool playerInWallX = false;
		bool playerInWallY = false;
		bool playerInWallZ = false;
		for (auto location : locations) {
			//Check x values
			float wallXMin = (location.x - wallRadius);
			float wallXMax = (location.x + wallRadius);
			float playerXMin = (position.x - playerRadius);
			float playerXMax = (position.x + playerRadius);
			bool playerInWallX = playerXMax >= wallXMin && playerXMin <= wallXMax;

			//Check y values
			float wallYMin = (location.y - wallRadius);
			float wallYMax = (location.y + wallRadius);
			float playerYMin = (position.y - playerRadius);
			float playerYMax = (position.y + playerRadius);
			bool playerInWallY = playerYMax >= wallYMin && playerYMin <= wallYMax;

			//Check z values
			float wallZMin = (location.z - wallRadius);
			float wallZMax = (location.z + wallRadius);
			float playerZMin = (position.z - playerRadius);
			float playerZMax = (position.z + playerRadius);
			bool playerInWallZ = playerZMax >= wallZMin && playerZMin <= wallZMax;
			if (playerInWallX && playerInWallY && playerInWallZ) {
				printf("%d %d %d\n", playerInWallX, playerInWallY, playerInWallZ);
				return true;
			}
		}
		return false;
	}

	void loadModel(Shader* s) {
		std::cout << "Loading Walls... " << std::endl;
		shader = s;
		glm::vec3 color = glm::vec3(1, 1, 1);
		modelData = objFileParser.loadOBJFile("models/wall.obj", numLines, numTris, color);

		//Build VAO from model data
		buildVAO();
	};

	void buildVAO() {
		//Load texture
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		unsigned char* data = stbi_load("models/wall128x128.png", &width, &height, &nrChannels, 0);
		if (data){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture. Error: " << stbi_failure_reason() << std::endl;
		}
		stbi_image_free(data);
		glUniform1i(glGetUniformLocation(shader->ID, "Texture"), 0); 
		//Unbind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
		glGenVertexArrays(1, &vao); //Create a VAO
		glBindVertexArray(vao); //Bind the above created VAO to the current context

		//Allocate memory on the graphics card to store geometry (vertex buffer object)
		glGenBuffers(1, vbo);  //Create 1 buffer called vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
		glBufferData(GL_ARRAY_BUFFER, numLines * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
		//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
		//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

	  //Tell OpenGL how to set fragment shader input 
		GLint posAttrib = glGetAttribLocation(shader->ID, "position");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), 0);
		//Attribute, vals/attrib., type, normalized?, stride, offset
		//Binds to VBO current GL_ARRAY_BUFFER 
		glEnableVertexAttribArray(posAttrib);

		GLint textAttrib = glGetAttribLocation(shader->ID, "inTextCoord");
		glVertexAttribPointer(textAttrib, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(textAttrib);

		GLint normAttrib = glGetAttribLocation(shader->ID, "inNormal");
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(normAttrib);

		GLint colAttrib = glGetAttribLocation(shader->ID, "inColor");
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(8*sizeof(float)));
		glEnableVertexAttribArray(colAttrib);

		glBindVertexArray(0); //Unbind the VAO
	};

	void draw() {
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		for (int i = 0; i < locations.size(); i++) {
			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, locations[i]);
			//model = glm::translate(model, glm::vec3(0,i,0));
			//model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
			//model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
			//model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
			shader->setUniform("model", model);
			glDrawArrays(GL_TRIANGLES, 0, numTris); //(Primitives, Which VBO, Number of vertices)
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0); //Unbind the texture
		glBindVertexArray(0); //Unbind the VAO
	};

	void cleanUp() {
		glDeleteBuffers(1, vbo);
		glDeleteVertexArrays(1, &vao);
	}
};
#endif
