#ifndef FLOOR_H
#define FLOOR_H
#include "OBJFileParser.h"

using namespace std;

class Floor {
public:
	Shader* shader;
	OBJFileParser objFileParser;
	float* modelData;
	GLuint vao;
	GLuint vbo[1];
	int numLines = 0;
	float numTris = 0;

	int width;
	int height;

	Floor() {};

	void loadModel(Shader* s, int w, int h) {
		shader = s;
		//Load model data
		glm::vec3 color = glm::vec3(0.8, 0.1, 0.2);
		modelData = objFileParser.loadOBJFile("models/test.obj", numLines, numTris, color);

		width = w;
		height = h;

		//Build VAO from model data
		buildVAO();
	};

	void buildVAO() {
		//Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
		glGenVertexArrays(1, &vao); //Create a VAO
		glBindVertexArray(vao); //Bind the above created VAO to the current context

		//Allocate memory on the graphics card to store geometry (vertex buffer object)
		glGenBuffers(1, vbo);  //Create 1 buffer called vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
		glBufferData(GL_ARRAY_BUFFER, numLines * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo

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
		glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(colAttrib);
		glBindVertexArray(0); //Unbind the VAO
	};

	void draw() {
		glBindVertexArray(vao);
		glm::mat4 model = glm::mat4(1);
		model = glm::translate(model,glm::vec3(1, -0.5, 1));
		model = glm::scale(model,glm::vec3(width, 0.1, height));
		shader->setUniform("model", model);
		glDrawArrays(GL_TRIANGLES, 0, numTris); //(Primitives, Which VBO, Number of vertices)
		glBindVertexArray(0); //Unbind the VAO
	};

	void cleanUp() {
		glDeleteBuffers(1, vbo);
		glDeleteVertexArrays(1, &vao);
	}
};
#endif
