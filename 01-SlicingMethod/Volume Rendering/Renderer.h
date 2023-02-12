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
	void LoadShaderFromFile(GLenum whichShader, const string& filename);
	void CompileShader(GLenum whichShader, const string& source);
	void CreateAndLinkProgram();


private:
	enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER };
	GLuint	_program;
	int _totalShaders;
	GLuint _shaders[3];//0->vertexshader, 1->fragmentshader, 2->geometryshader
	map<string, GLuint> _attributeList;
	map<string, GLuint> _uniformLocationList;
};
