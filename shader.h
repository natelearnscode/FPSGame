#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
	GLuint ID; //program id

	Shader(const char* vertexPath, const char* fragmentPath) {
		//Get source code from file
		std::string vertexSource;
		std::string fragmentSource;

		std::ifstream vfile;
		std::ifstream ffile;
		
		//Read from vertex and fragment source files
		try {
			vfile.open(vertexPath);
			ffile.open(fragmentPath);
			vertexSource = std::string((std::istreambuf_iterator<char>(vfile)), std::istreambuf_iterator<char>());
			fragmentSource= std::string((std::istreambuf_iterator<char>(ffile)), std::istreambuf_iterator<char>());
			vfile.close();
			ffile.close();
		}
		catch(std::ifstream::failure error){
			std::cout << "Error reading from shader file!" << std::endl;
		}
		const char* vSource = vertexSource.c_str();
		const char* fSource = fragmentSource.c_str();

		//Compile Shaders

		//Load the vertex Shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vSource, NULL);
		glCompileShader(vertexShader);

		//Check that the shader compiled 
		GLint status;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
		if (!status) {
			char buffer[512];
			glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				"Compilation Error",
				"Failed to Compile: Check Consol Output.",
				NULL);
			printf("Vertex Shader Compile Failed. Info:\n\n%s\n", buffer);
		}

		//Load the fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fSource, NULL);
		glCompileShader(fragmentShader);

		//Check that the shader compiled 
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
		if (!status) {
			char buffer[512];
			glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				"Compilation Error",
				"Failed to Compile: Check Consol Output.",
				NULL);
			printf("Fragment Shader Compile Failed. Info:\n\n%s\n", buffer);
		}

		//Join the vertex and fragment shaders together into one program
		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glBindFragDataLocation(ID, 0, "outColor"); // set output
		glLinkProgram(ID); //run the linker

		//Delete unneeded shaders after they've been linked
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	};

	//Method that sets the uniform matrix values in the shader
	void setUniformMatrix(const std::string &name, glm::mat4 value) {
		GLint uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
	};

	//Method that sets the uniform vec3 values in the shader
	void setUniformVec3(const std::string& name, glm::vec3 value) {
		GLint uniformLocation = glGetUniformLocation(ID, name.c_str());
		glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
	};

	void use() { glUseProgram(ID); };
	void deleteShader() { glDeleteProgram(ID); };

};



#endif