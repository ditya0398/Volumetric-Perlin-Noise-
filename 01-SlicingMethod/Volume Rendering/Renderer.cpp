#include "Renderer.h"
#include <iostream>

#include <fstream>

//Loads the shader from the External File
void Renderer::LoadShaderFromFile(GLenum whichShader, const string& filename) {
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if (fp) {
		string line, buffer;
		while (getline(fp, line)) {
			buffer.append(line);
			buffer.append("\r\n");
		}
		//copy to source
		CompileShader(whichShader, buffer);
	}
	else {
		cerr << "Error loading shader: " << filename << endl;
	}
}

//Compiles the Shader
void Renderer::CompileShader(GLenum type, const string& source) {
	GLuint shader = glCreateShader(type);

	const char* ptmp = source.c_str();
	glShaderSource(shader, 1, &ptmp, NULL);

	//check whether the shader loads fine
	GLint status;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
		cerr << "Compile log: " << infoLog << endl;
		delete[] infoLog;
	}
	_shaders[_totalShaders++] = shader;
}
