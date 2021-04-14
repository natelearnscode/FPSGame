#ifndef OBJFILEPARSER_H
#define OBJFILEPARSER_H
#include <iostream>

class OBJFileParser {
public:

	OBJFileParser() {};

	float* loadOBJFile(const char* modelFilePath, int &modelDataLength, float &numTris, glm::vec3 color) {
		std::ifstream modelFile;
		modelFile.open(modelFilePath);
		if (!modelFile) {	// if not a valid input source, abort
			std::cout << "Unable to load file: " << modelFilePath << " Aborting..." << std::endl;
			exit(0);
		}

		std::string line;
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> textureUV;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> vertexIndices;

		//Parse through obj file
		while (getline(modelFile, line)) {
			std::string element = line.substr(0, line.find(" "));

			if (element == "v") { //Vertex Position
				float x, y, z;
				int numArgs = sscanf(line.c_str(), "v %f %f %f", &x, &y, &z);
				if (numArgs < 3) {
					std::cout << "Error parsing line" << line << std::endl;
				}
				vertices.push_back(glm::vec3(x, y, z));
				//printf("v %f %f %f\n", x, y, z);
			}
			if (element == "vt") {//Vertex UV Texture Coordinates
				float x, y;
				int numArgs = sscanf(line.c_str(), "vt %f %f", &x, &y);
				if (numArgs < 2) {
					std::cout << "Error parsing line" << line << std::endl;
				}
				textureUV.push_back(glm::vec2(x, y));
				//printf("vt %f %f\n", x, y);
			}
			if (element == "vn") {//Vertex Normal
				float x, y, z;
				int numArgs = sscanf(line.c_str(), "vn %f %f %f", &x, &y, &z);
				if (numArgs < 3) {
					std::cout << "Error parsing line" << line << std::endl;
				}
				normals.push_back(glm::vec3(x, y, z));
				//printf("vn %f %f %f\n", x, y, z);
			}
			if (element == "f") {//Faces
				float v1, t1, n1, v2, t2, n2, v3, t3, n3;
				int numArgs = sscanf(line.c_str(), "f %f/%f/%f %f/%f/%f %f/%f/%f", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
				if (numArgs != 9) {
					std::cout << "Error parsing line" << line << std::endl;
				}
				//printf("f %f/%f/%f %f/%f/%f %f/%f/%f\n", v1, t1, n1, v2, t2, n2, v3, t3, n3);

				//Add face indices
				vertexIndices.push_back(glm::vec3(v1, t1, n1)); //vertex 1
				vertexIndices.push_back(glm::vec3(v2, t2, n2)); //vertex 2
				vertexIndices.push_back(glm::vec3(v3, t3, n3)); //vertex 3
			}
		}

		//Format model data for OpenGl
		modelDataLength = vertexIndices.size() * 11;
		numTris = vertexIndices.size();

		float * modelData = new float[modelDataLength];
		for (int i = 0; i < modelDataLength; i+= 11) {
			glm::vec3 vertexIndex = vertexIndices[i/11];
			int vIndex = vertexIndex.x - 1;
			int tIndex = vertexIndex.y - 1;
			int nIndex = vertexIndex.z - 1;

			//Add position
			glm::vec3 v = vertices[vIndex];
			//printf("vertex index: %d, (%f,%f,%f) (%f,%f,%f)\n", i / 11, v.x, v.y, v.z, vertices[vIndex].x, vertices[vIndex].y, vertices[vIndex].z);
			modelData[i] = v.x;
			modelData[i + 1] = v.y;
			modelData[i + 2] = v.z;

			//Add texture uv
			glm::vec2 t = textureUV[tIndex];
			modelData[i + 3] = t.x;
			modelData[i + 4] = t.y;

			//Add normal
			glm::vec3 n = normals[nIndex];
			modelData[i + 5] = v.x;
			modelData[i + 6] = v.y;
			modelData[i + 7] = v.z;

			//Add color
			modelData[i + 8] = color.x;
			modelData[i + 9] = color.y;
			modelData[i + 10] = color.z;
		}
		return modelData;
	}
};

#endif