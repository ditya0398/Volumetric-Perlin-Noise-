#pragma once
#include <GL/glew.h>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;

class Renderer
{
public:
	//cube vertex array and vertex buffer object IDs
	GLuint cubeVBOID;
	GLuint cubeVAOID;
	GLuint cubeIndicesID;


	Renderer(void);
	~Renderer(void);	
	void init();
	void CreateVAOandVBO();
	void CompileShader(GLenum whichShader, const string& source);
	void LoadFromFile(GLenum whichShader, const string& filename);
	void CreateAndLinkProgram();
	void UseProgram();
	void UnUseProgram();
	void AddAttribute(const string& attribute);
	void AddUniform(const string& uniform);
	void ResizeViewport(int start, int end, float width, float height);

	//An indexer that returns the location of the attribute/uniform
	GLuint operator[](const string& attribute);
	GLuint operator()(const string& uniform);
	void DeleteShaderProgram();
	void DeleteBuffers();
private:
	enum ShaderType {VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER};
	GLuint	_program;
	int _totalShaders;
	GLuint _shaders[3];//0->vertexshader, 1->fragmentshader, 2->geometryshader
	map<string,GLuint> _attributeList;
	map<string,GLuint> _uniformLocationList;
};	
