#include "Renderer.h"
#include <iostream>

#include <fstream>

Renderer::Renderer(void)
{
	_totalShaders = 0;
	_shaders[VERTEX_SHADER] = 0;
	_shaders[FRAGMENT_SHADER] = 0;
	_shaders[GEOMETRY_SHADER] = 0;
	_attributeList.clear();
	_uniformLocationList.clear();
}

Renderer::~Renderer(void)
{
	_attributeList.clear();
	_uniformLocationList.clear();
} 

//Loads the shader from the External File
void Renderer::LoadShaderFromFile(GLenum whichShader, const string& filename) {
	//File stream 
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

	//ERROR CHECKING FOR THE SHADER
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

void Renderer::AddAttribute(const string& attribute) {
	_attributeList[attribute] = glGetAttribLocation(_program, attribute.c_str());
}

void Renderer::init()
{
	//Load the texture slicing shader
	LoadShaderFromFile(GL_VERTEX_SHADER, "shaders/textureSlicer.vert");
	LoadShaderFromFile(GL_FRAGMENT_SHADER, "shaders/textureSlicer.frag");

	//compile and link the shader
	CreateAndLinkProgram();
	UseProgram();

	//add attributes and uniforms
	AddAttribute("vVertex");
	AddUniform("MVP");
	AddUniform("volume");
	AddUniform("delta");

	UnUseProgram();


}
void Renderer::CreateAndLinkProgram() {
	_program = glCreateProgram();
	if (_shaders[VERTEX_SHADER] != 0) {
		glAttachShader(_program, _shaders[VERTEX_SHADER]);
	}
	if (_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader(_program, _shaders[FRAGMENT_SHADER]);
	}
	if (_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader(_program, _shaders[GEOMETRY_SHADER]);
	}

	//link and check whether the program links fine
	GLint status;
	glLinkProgram(_program);
	glGetProgramiv(_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;

		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* infoLog = new GLchar[infoLogLength];
		glGetProgramInfoLog(_program, infoLogLength, NULL, infoLog);
		cerr << "Link log: " << infoLog << endl;
		delete[] infoLog;
	}

	glDeleteShader(_shaders[VERTEX_SHADER]);
	glDeleteShader(_shaders[FRAGMENT_SHADER]);
	glDeleteShader(_shaders[GEOMETRY_SHADER]);
}

void Renderer::UseProgram() {
	glUseProgram(_program);
}

void Renderer::UnUseProgram() {
	glUseProgram(0);
}

GLuint Renderer::operator [](const string& attribute) {
	return _attributeList[attribute];
}

void Renderer::AddUniform(const string& uniform) {
	_uniformLocationList[uniform] = glGetUniformLocation(_program, uniform.c_str());
}
void Renderer::DeleteShaderProgram() {
	glDeleteProgram(_program);
}
GLuint Renderer::operator()(const string& uniform) {
	return _uniformLocationList[uniform];
}