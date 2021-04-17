#ifndef DOOR_H
#define DOOR_H
#include <map>

class Door {
public:
	std::map<char, glm::vec3> dictionary;
	Shader* shader;
	OBJFileParser objFileParser;
	float* modelData;
	GLuint vao;
	GLuint vbo[1];
	unsigned int texture;
	int numLines = 0;
	float numTris = 0;
	float doorRadius = 1.0f;

	Door() {};

	bool checkCollision(glm::vec3 position, float playerRadius, char carriedKey, bool& isUnlocked) {
		for (auto key : dictionary) {
			glm::vec3 location = key.second;
			//Check x values
			float wallXMin = (location.x - doorRadius);
			float wallXMax = (location.x + doorRadius);
			float playerXMin = (position.x - playerRadius);
			float playerXMax = (position.x + playerRadius);
			bool playerInKeyX = playerXMax >= wallXMin && playerXMin <= wallXMax;

			//Check y values
			float wallYMin = (location.y - doorRadius);
			float wallYMax = (location.y + doorRadius);
			float playerYMin = (position.y - playerRadius);
			float playerYMax = (position.y + playerRadius);
			bool playerInKeyY = playerYMax >= wallYMin && playerYMin <= wallYMax;

			//Check z values
			float wallZMin = (location.z - doorRadius);
			float wallZMax = (location.z + doorRadius);
			float playerZMin = (position.z - playerRadius);
			float playerZMax = (position.z + playerRadius);
			bool playerInKeyZ = playerZMax >= wallZMin && playerZMin <= wallZMax;
			if (playerInKeyX && playerInKeyY && playerInKeyZ) {
				//printf("carried key %c door %c\n");
				if (carriedKey == tolower(key.first)) {
					dictionary[key.first].y += 2;
					isUnlocked = true;
				}
				//dictionary.erase(key.first);
				return true;
			}
		}
		return false;
	}

	void loadModel(Shader* s) {
		shader = s;
		modelData = objFileParser.loadOBJFile("models/wall.obj", numLines, numTris);

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
		unsigned char* data = stbi_load("models/door.png", &width, &height, &nrChannels, 0);
		if (data) {
			//printf("w %d h %d channels %d\n", width, height, nrChannels);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		//Attribute, vals/attrib., type, normalized?, stride, offset
		//Binds to VBO current GL_ARRAY_BUFFER 
		glEnableVertexAttribArray(posAttrib);

		GLint textAttrib = glGetAttribLocation(shader->ID, "inTextCoord");
		glVertexAttribPointer(textAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(textAttrib);

		GLint normAttrib = glGetAttribLocation(shader->ID, "inNormal");
		glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(normAttrib);

		glBindVertexArray(0); //Unbind the VAO
	};


	void draw() {
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		for (auto door : dictionary) {
			char doorType = door.first;
			glm::vec3 color;
			if (doorType == 'A') {
				color = glm::vec3(1, 0, 0); //red
			}
			else if (doorType == 'B') {
				color = glm::vec3(0, 1, 0); //green
			}
			else if (doorType == 'C') {
				color = glm::vec3(0, 0, 1); //blue
			}
			else if (doorType == 'D') {
				color = glm::vec3(1, 1, 0); //yellow
			}
			else if (doorType == 'E') {
				color = glm::vec3(1, 0, 1); //purple
			}
			glm::mat4 model = glm::mat4(1);
			model = glm::translate(model, door.second);
			shader->setUniformMatrix("model", model);
			shader->setUniformVec3("inColor", color);
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