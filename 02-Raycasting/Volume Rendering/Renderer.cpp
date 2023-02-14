#include "Renderer.h"
#include <iostream>


Renderer::Renderer(void)
{
	_totalShaders=0;
	_shaders[VERTEX_SHADER]=0;
	_shaders[FRAGMENT_SHADER]=0;
	_shaders[GEOMETRY_SHADER]=0;
	_attributeList.clear();
	_uniformLocationList.clear();
}

Renderer::~Renderer(void)
{
	_attributeList.clear();	
	_uniformLocationList.clear();
}

void Renderer::init()
{
	//Load the raycasting shader
	LoadFromFile(GL_VERTEX_SHADER, "shaders/raycaster.vert");
	LoadFromFile(GL_FRAGMENT_SHADER, "shaders/raycaster.frag");

	//compile and link the shader
	CreateAndLinkProgram();
	UseProgram();
	//add attributes and uniforms
	AddAttribute("vVertex");
	AddUniform("MVP");
	AddUniform("volume");
	AddUniform("camPos");
	AddUniform("step_size");
	AddUniform("delta");
	AddUniform("MAX_SAMPLES");
	//pass constant uniforms at initialization


	UnUseProgram();
}

void Renderer::CreateVAOandVBO()
{

	//setup unit cube vertex array and vertex buffer objects
	glGenVertexArrays(1, &cubeVAOID);
	glGenBuffers(1, &cubeVBOID);
	glGenBuffers(1, &cubeIndicesID);

	//unit cube vertices 
	glm::vec3 vertices[8] = { glm::vec3(-0.5f,-0.5f,-0.5f),
							glm::vec3(0.5f,-0.5f,-0.5f),
							glm::vec3(0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f, 0.5f,-0.5f),
							glm::vec3(-0.5f,-0.5f, 0.5f),
							glm::vec3(0.5f,-0.5f, 0.5f),
							glm::vec3(0.5f, 0.5f, 0.5f),
							glm::vec3(-0.5f, 0.5f, 0.5f) };

	//unit cube indices
	GLushort cubeIndices[36] = { 0,5,4,
							  5,0,1,
							  3,7,6,
							  3,6,2,
							  7,4,6,
							  6,4,5,
							  2,1,3,
							  3,1,0,
							  3,0,7,
							  7,0,4,
							  6,5,2,
							  2,5,1 };
	glBindVertexArray(cubeVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBOID);
	//pass cube vertices to buffer object memory
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x), GL_STATIC_DRAW);

	//GL_CHECK_ERRORS
	//enable vertex attributre array for position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//pass indices to element array  buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndicesID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

}

void Renderer::DeleteShaderProgram() {	
	glDeleteProgram(_program);
}

void Renderer::DeleteBuffers()
{

	glDeleteVertexArrays(1, &cubeVAOID);
	glDeleteBuffers(1, &cubeVBOID);
	glDeleteBuffers(1, &cubeIndicesID);

}

void Renderer::CompileShader(GLenum type, const string& source) {	
	GLuint shader = glCreateShader (type);

	const char * ptmp = source.c_str();
	glShaderSource (shader, 1, &ptmp, NULL);
	
	//check whether the shader loads fine
	GLint status;
	glCompileShader (shader);
	glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;		
		glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog= new GLchar[infoLogLength];
		glGetShaderInfoLog (shader, infoLogLength, NULL, infoLog);
		cerr<<"Compile log: "<<infoLog<<endl;
		delete [] infoLog;
	}
	_shaders[_totalShaders++]=shader;
}


void Renderer::CreateAndLinkProgram() {
	_program = glCreateProgram ();
	if (_shaders[VERTEX_SHADER] != 0) {
		glAttachShader (_program, _shaders[VERTEX_SHADER]);
	}
	if (_shaders[FRAGMENT_SHADER] != 0) {
		glAttachShader (_program, _shaders[FRAGMENT_SHADER]);
	}
	if (_shaders[GEOMETRY_SHADER] != 0) {
		glAttachShader (_program, _shaders[GEOMETRY_SHADER]);
	}
	
	//link and check whether the program links fine
	GLint status;
	glLinkProgram (_program);
	glGetProgramiv (_program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint infoLogLength;
		
		glGetProgramiv (_program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *infoLog= new GLchar[infoLogLength];
		glGetProgramInfoLog (_program, infoLogLength, NULL, infoLog);
		cerr<<"Link log: "<<infoLog<<endl;
		delete [] infoLog;
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

void Renderer::AddAttribute(const string& attribute) {
	_attributeList[attribute]= glGetAttribLocation(_program, attribute.c_str());	
}

//An indexer that returns the location of the attribute
GLuint Renderer::operator [](const string& attribute) {
	return _attributeList[attribute];
}

void Renderer::AddUniform(const string& uniform) {
	_uniformLocationList[uniform] = glGetUniformLocation(_program, uniform.c_str());
}

void Renderer::ResizeViewport(int start, int end, float width, float height)
{
	//reset the viewport
	glViewport(start, end, (GLsizei)width, (GLsizei)height);
}

GLuint Renderer::operator()(const string& uniform){
	return _uniformLocationList[uniform];
}

#include <fstream>
#include "Renderer.h"
void Renderer::LoadFromFile(GLenum whichShader, const string& filename){
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if(fp) {		 
		string line, buffer;
		while(getline(fp, line)) {
			buffer.append(line);
			buffer.append("\r\n");
		}		 
		//copy to source
		CompileShader(whichShader, buffer);		
	} else {
		cerr<<"Error loading shader: "<<filename<<endl;
	}
}
