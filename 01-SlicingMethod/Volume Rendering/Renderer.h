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
	//volume vertex array and buffer objects
	GLuint volumeVBO;
	GLuint volumeVAO;
	

	//sliced vertices
	glm::vec3 vTextureSlices[512 * 12];
	


	Renderer(void);
	~Renderer(void);


	void init();
	void CreateVAOandVBO();
	void LoadShaderFromFile(GLenum whichShader, const string& filename);
	void CompileShader(GLenum whichShader, const string& source);
	void CreateAndLinkProgram();
	void SetBackgroundColor(float r, float g, float b);
	void UseProgram();
	void UnUseProgram();
	void ResizeViewport(int start, int end, float width, float height);

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
