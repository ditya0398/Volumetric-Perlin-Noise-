#pragma once
#include <GL/glew.h>
#include <map>
#include <string>

using namespace std;

class Renderer
{
public:
	Renderer(void);
	~Renderer(void);

	void init();
	void LoadShaderFromFile(GLenum whichShader, const string& filename);
	void CompileShader(GLenum whichShader, const string& source);
	void CreateAndLinkProgram();
	void UseProgram();
	void UnUseProgram();
	//An indexer that returns the location of the attribute/uniform
	GLuint operator[](const string& attribute);
	GLuint operator()(const string& uniform);
	void DeleteShaderProgram();
	void AddAttribute(const string& attribute);
	void AddUniform(const string& uniform);


private:
	enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER };
	GLuint	_program;
	int _totalShaders;
	GLuint _shaders[3];//0->vertexshader, 1->fragmentshader, 2->geometryshader
	map<string, GLuint> _attributeList;
	map<string, GLuint> _uniformLocationList;
};
