
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include <fstream>
#include <algorithm>
#include"Noise.h"

//for floating point inaccuracy
const float EPSILON = 0.0001f;

#ifdef _DEBUG 
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")//glew wrangler library dynamically linked
#pragma comment(lib, "freeglut_static_x86_d.lib") //linked statically
#pragma comment(lib, "SOIL_static_x86_d.lib") //linked statically
#else
#pragma comment(lib, "glew_static_x86.lib")
#pragma comment(lib, "freeglut_static_x86.lib")
#pragma comment(lib, "SOIL_static_x86.lib")
#endif

using namespace std;

//screen dimensions
const int WIDTH = 1280;
const int HEIGHT = 960;

//camera transform variables
int state = 0, oldX = 0, oldY = 0;
float rX = 4, rY = 50, dist = -2;



//modelview and projection matrices
glm::mat4 MV, P;



//3D texture slicing shader
Renderer render;

//maximum number of slices
const int MAX_SLICES = 512;

//background colour
glm::vec4 bg = glm::vec4(0.5, 0.5, 1, 1);

//dimensions of volume data
const int XDIM = 128;
const int YDIM = 128;
const int ZDIM = 128;

//total number of slices current used
int num_slices = 0;
GLuint delta;

//flag to see if the view is rotated
//volume is resliced if the view is rotated
bool bViewRotated = false;

//unit cube vertices
glm::vec3 vertexList[8] = { glm::vec3(-0.5,-0.5,-0.5),
						   glm::vec3(0.5,-0.5,-0.5),
						   glm::vec3(0.5, 0.5,-0.5),
						   glm::vec3(-0.5, 0.5,-0.5),
						   glm::vec3(-0.5,-0.5, 0.5),
						   glm::vec3(0.5,-0.5, 0.5),
						   glm::vec3(0.5, 0.5, 0.5),
						   glm::vec3(-0.5, 0.5, 0.5) };

//unit cube edges
int edgeList[8][12] = {
	{ 0,1,5,6,   4,8,11,9,  3,7,2,10 }, // v0 is front
	{ 0,4,3,11,  1,2,6,7,   5,9,8,10 }, // v1 is front
	{ 1,5,0,8,   2,3,7,4,   6,10,9,11}, // v2 is front
	{ 7,11,10,8, 2,6,1,9,   3,0,4,5  }, // v3 is front
	{ 8,5,9,1,   11,10,7,6, 4,3,0,2  }, // v4 is front
	{ 9,6,10,2,  8,11,4,7,  5,0,1,3  }, // v5 is front
	{ 9,8,5,4,   6,1,2,0,   10,7,11,3}, // v6 is front
	{ 10,9,6,5,  7,2,3,1,   11,4,8,0 }  // v7 is front
};
const int edges[12][2] = { {0,1},{1,2},{2,3},{3,0},{0,4},{1,5},{2,6},{3,7},{4,5},{5,6},{6,7},{7,4} };

//current viewing direction
glm::vec3 viewDir;


//mouse down event handler
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if (button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;

	if (s == GLUT_UP)
		bViewRotated = false;
}

//mouse move event handler
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist += (y - oldY) / 50.0f;
	}
	else {
		rX += (y - oldY) / 5.0f;
		rY += (x - oldX) / 5.0f;
		bViewRotated = true;
	}
	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

//function to get the max (abs) dimension of the given vertex v
int FindAbsMax(glm::vec3 v) {
	v = glm::abs(v);
	int max_dim = 0;
	float val = v.x;
	if (v.y > val) {
		val = v.y;
		max_dim = 1;
	}
	if (v.z > val) {
		val = v.z;
		max_dim = 2;
	}
	return max_dim;
}

//main slicing function
void SliceVolume() {

	//get the max and min distance of each vertex of the unit cube in the viewing direction
	float max_dist = glm::dot(viewDir, vertexList[0]);
	float min_dist = max_dist;
	int max_index = 0;
	int count = 0;

	for (int i = 1; i < 8; i++) {
		//get the distance between the current unit cube vertex and 
		//the view vector by dot product
		float dist = glm::dot(viewDir, vertexList[i]);

		//if distance is > max_dist, store the value and index
		if (dist > max_dist) {
			max_dist = dist;
			max_index = i;
		}

		//if distance is < min_dist, store the value 
		if (dist < min_dist)
			min_dist = dist;
	}
	//find tha abs maximum of the view direction vector
	int max_dim = FindAbsMax(viewDir);

	//expand it a little bit
	min_dist -= EPSILON;
	max_dist += EPSILON;

	//local variables to store the start, direction vectors, 
	//lambda intersection values
	glm::vec3 vecStart[12];
	glm::vec3 vecDir[12];
	float lambda[12];
	float lambda_inc[12];
	float denom = 0;

	//set the minimum distance as the plane_dist
	//subtract the max and min distances and divide by the 
	//total number of slices to get the plane increment
	float plane_dist = min_dist;
	float plane_dist_inc = (max_dist - min_dist) / float(num_slices);

	//for all edges
	for (int i = 0; i < 12; i++) {
		//get the start position vertex by table lookup
		vecStart[i] = vertexList[edges[edgeList[max_index][i]][0]];

		//get the direction by table lookup
		vecDir[i] = vertexList[edges[edgeList[max_index][i]][1]] - vecStart[i];

		//do a dot of vecDir with the view direction vector
		denom = glm::dot(vecDir[i], viewDir);

		//determine the plane intersection parameter (lambda) and 
		//plane intersection parameter increment (lambda_inc)
		if (1.0 + denom != 1.0) {
			lambda_inc[i] = plane_dist_inc / denom;
			lambda[i] = (plane_dist - glm::dot(vecStart[i], viewDir)) / denom;
		}
		else {
			lambda[i] = -1.0;
			lambda_inc[i] = 0.0;
		}
	}

	//local variables to store the intesected points
	//note that for a plane and sub intersection, we can have 
	//a minimum of 3 and a maximum of 6 vertex polygon
	glm::vec3 intersection[6];
	float dL[12];

	//loop through all slices
	for (int i = num_slices - 1; i >= 0; i--) {

		//determine the lambda value for all edges
		for (int e = 0; e < 12; e++)
		{
			dL[e] = lambda[e] + i * lambda_inc[e];
		}

		//if the values are between 0-1, we have an intersection at the current edge
		//repeat the same for all 12 edges
		if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[0] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[0] = vecStart[1] + dL[1] * vecDir[1];
		}
		else if ((dL[3] >= 0.0) && (dL[3] < 1.0)) {
			intersection[0] = vecStart[3] + dL[3] * vecDir[3];
		}
		else continue;

		if ((dL[2] >= 0.0) && (dL[2] < 1.0)) {
			intersection[1] = vecStart[2] + dL[2] * vecDir[2];
		}
		else if ((dL[0] >= 0.0) && (dL[0] < 1.0)) {
			intersection[1] = vecStart[0] + dL[0] * vecDir[0];
		}
		else if ((dL[1] >= 0.0) && (dL[1] < 1.0)) {
			intersection[1] = vecStart[1] + dL[1] * vecDir[1];
		}
		else {
			intersection[1] = vecStart[3] + dL[3] * vecDir[3];
		}

		if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[2] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[2] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[2] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[6] >= 0.0) && (dL[6] < 1.0)) {
			intersection[3] = vecStart[6] + dL[6] * vecDir[6];
		}
		else if ((dL[4] >= 0.0) && (dL[4] < 1.0)) {
			intersection[3] = vecStart[4] + dL[4] * vecDir[4];
		}
		else if ((dL[5] >= 0.0) && (dL[5] < 1.0)) {
			intersection[3] = vecStart[5] + dL[5] * vecDir[5];
		}
		else {
			intersection[3] = vecStart[7] + dL[7] * vecDir[7];
		}
		if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[4] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[4] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[4] = vecStart[11] + dL[11] * vecDir[11];
		}

		if ((dL[10] >= 0.0) && (dL[10] < 1.0)) {
			intersection[5] = vecStart[10] + dL[10] * vecDir[10];
		}
		else if ((dL[8] >= 0.0) && (dL[8] < 1.0)) {
			intersection[5] = vecStart[8] + dL[8] * vecDir[8];
		}
		else if ((dL[9] >= 0.0) && (dL[9] < 1.0)) {
			intersection[5] = vecStart[9] + dL[9] * vecDir[9];
		}
		else {
			intersection[5] = vecStart[11] + dL[11] * vecDir[11];
		}

		//after all 6 possible intersection vertices are obtained,
		//we calculated the proper polygon indices by using indices of a triangular fan
		int indices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,5 };

		//Using the indices, pass the intersection vertices to the vTextureSlices vector
		for (int i = 0; i < 12; i++)
			render.vTextureSlices[count++] = intersection[indices[i]];
	}

	//update buffer object with the new vertices
	glBindBuffer(GL_ARRAY_BUFFER, render.volumeVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(render.vTextureSlices), &(render.vTextureSlices[0].x));
}

//OpenGL initialization
void InitializeOpenGL() {

	render.init();

	//Create 3D Noise and pass it as a Texture to the GPU
	make3DNoiseTexture();
	init3DNoiseTexture(Noise3DTexSize, Noise3DTexPtr);

	//setup the current camera transform and get the view direction vector
	glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(T, rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the current view direction vector
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//slice the volume dataset initially
	SliceVolume();

	std::cout << "Initialization successfull" << endl;
}

//release all allocated resources
void OnShutdown() {
	render.DeleteShaderProgram();
	render.DeleteBuffers();
	cout << "Shutdown successfull" << endl;
}

//resize event handler
void OnResize(int w, int h) {

	render.ResizeViewport(0, 0, w, h);
	//setup the projection matrix
	P = glm::perspective(glm::radians(60.0f), (float)w / h, 0.1f, 1000.0f);
}


//display function
void OnRender() {
	
	//setup the camera transform
	glm::mat4 Tr = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx = glm::rotate(Tr, rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the viewing direction
	viewDir = -glm::vec3(MV[0][2], MV[1][2], MV[2][2]);

	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
	glm::mat4 MVP = P * MV;


	//if view is rotated, reslice the volume
	if (bViewRotated)
	{
		SliceVolume();
	}

	//enable alpha blending (use over operator)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//bind volume vertex array object
	glBindVertexArray(render.volumeVAO);

	//use the volume shader
	render.UseProgram();
	//pass constant uniforms at initialization
	glUniform1i(render("volume"), 0);
	static float Delta;
	//pass the shader uniform
	glUniformMatrix4fv(render("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform1f(render("delta"), Delta);
	Delta += 0.001f;
	
	//draw the triangles
	glDrawArrays(GL_TRIANGLES, 0, sizeof(render.vTextureSlices) / sizeof(render.vTextureSlices[0]));
	
	//unbind the shader
	render.UnUseProgram();

	//disable blending
	glDisable(GL_BLEND);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
	glutPostRedisplay();
}

//keyboard function to change the number of slices
void OnKey(unsigned char key, int x, int y) {
	switch (key) {
	case '-':
		//if(num_slices > -10)
		num_slices = 0;
		break;

	case '+':
		num_slices = num_slices + 5;
		break;
	}

	//check the range of num_slices variable
	//num_slices = min(MAX_SLICES, max(num_slices,3));

	//slice the volume
	SliceVolume();

	//recall display function
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Volume Rendering using 3D Texture Slicing - OpenGL 3.3");

	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		cerr << "Error: " << glewGetErrorString(err) << endl;
	}
	else {
		if (GLEW_VERSION_3_3)
		{
			cout << "Driver supports OpenGL 3.3\nDetails:" << endl;
		}
	}
	err = glGetError();
	
	//output hardware information
	cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	//GL_CHECK_ERRORS

	//OpenGL initialization
	InitializeOpenGL();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}