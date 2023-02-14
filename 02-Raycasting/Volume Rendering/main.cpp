#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer.h"
#include <fstream>
#include"Noise.h"

#ifdef _DEBUG 
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glew32.lib")//glew wrangler library dynamically linked
#pragma comment(lib, "freeglut_static_x86_d.lib")
#pragma comment(lib, "SOIL_static_x86_d.lib")
#else
#pragma comment(lib, "glew_static_x86.lib")
#pragma comment(lib, "freeglut_static_x86.lib")
#pragma comment(lib, "SOIL_static_x86.lib")
#endif

using namespace std;

//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;

//camera transform variables
int state = 0, oldX=0, oldY=0;
float rX=4, rY=50, dist = -2;


//modelview projection matrices
glm::mat4 MV,P;

//ray casting shader
Renderer render;

//background colour
glm::vec4 bg=glm::vec4(0.5,0.5,1,1);

int MAX_SAMPLES = 0;
//volume dataset filename  
const std::string volume_file = "Engine256.raw";

//volume dimensions
const int XDIM = 128;
const int YDIM = 128;
const int ZDIM = 128;

//volume texture ID
GLuint textureID;

//function that load a volume from the given raw data file and 
//generates an OpenGL 3D texture from it
bool LoadVolume() {
	std::ifstream infile(volume_file.c_str(), std::ios_base::binary);

	if(infile.good()) {
		//read the volume data file
		GLubyte* pData = new GLubyte[XDIM*YDIM*ZDIM];
		infile.read(reinterpret_cast<char*>(pData), XDIM*YDIM*ZDIM*sizeof(GLubyte));
		infile.close();

		//generate OpenGL texture
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_3D, textureID);

		// set the texture parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//set the mipmap levels (base and max)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
				
		//allocate data with internal format and foramt as (GL_RED)	
		glTexImage3D(GL_TEXTURE_3D,0,GL_R8,XDIM,YDIM,ZDIM,0,GL_RED,GL_UNSIGNED_BYTE,pData);
		//GL_CHECK_ERRORS

		//generate mipmaps
		glGenerateMipmap(GL_TEXTURE_3D);

		//delete the volume data allocated on heap
		delete [] pData;

		return true;
	} else {
		return false;
	}
}

//mouse down event handler
void OnMouseDown(int button, int s, int x, int y)
{
	if (s == GLUT_DOWN)
	{
		oldX = x;
		oldY = y;
	}

	if(button == GLUT_MIDDLE_BUTTON)
		state = 0;
	else
		state = 1;
}

//mouse move event handler
void OnMouseMove(int x, int y)
{
	if (state == 0) {
		dist += (y - oldY)/50.0f;
	} else {
		rX += (y - oldY)/5.0f;
		rY += (x - oldX)/5.0f;
	}
	oldX = x;
	oldY = y;
	
	glutPostRedisplay();
}

//keyboard function to change the number of slices
void OnKey(unsigned char key, int x, int y) {
	switch (key) {
	case '-':
		//if(num_slices > -10)
		MAX_SAMPLES -= 1;
		break;

	case '+':
		MAX_SAMPLES += 1;
		break;
	}
	
	render.UseProgram();
	glUniform1i(render("MAX_SAMPLES"), MAX_SAMPLES);
	
	//recall display function
	glutPostRedisplay();
}


//OpenGL initialization
void InitOpenGL() {
	
	render.init();
	
	//load volume data
	if(LoadVolume()) {
		std::cout<<"Volume data loaded successfully."<<std::endl; 
	} else {
		std::cout<<"Cannot load volume data."<<std::endl;
		exit(EXIT_FAILURE);
	}

	glEnable(GL_TEXTURE_3D);
	//Create 3D Noise and pass it as a Texture to the GPU
	make3DNoiseTexture();
	init3DNoiseTexture(Noise3DTexSize, Noise3DTexPtr);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//set background colour
	glClearColor(bg.r, bg.g, bg.b, bg.a);
	
	render.CreateVAOandVBO();
	//enable depth test
	glEnable(GL_DEPTH_TEST);

	//set the over blending function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	cout<<"Initialization successfull"<<endl;
}

//release all allocated resources
void OnShutdown() {
	render.DeleteShaderProgram();
	render.DeleteBuffers();

	glDeleteTextures(1, &textureID);
	
	cout<<"Shutdown successfull"<<endl;
}


//resize event handler
void OnResize(int w, int h) {
	//reset the viewport
	render.ResizeViewport(0, 0, (GLsizei)w, (GLsizei)h);
	//setup the projection matrix
	P = glm::perspective(glm::radians(60.0f),(float)w/h, 0.1f,1000.0f);
}

//display callback function
void OnRender() {
	
	//set the camera transform
	glm::mat4 Tr	= glm::translate(glm::mat4(1.0f),glm::vec3(0.0f, 0.0f, dist));
	glm::mat4 Rx	= glm::rotate(Tr,  rX, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 MV    = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

	//get the camera position
	glm::vec3 camPos = glm::vec3(glm::inverse(MV)*glm::vec4(0,0,0,1));

	//clear colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//get the combined modelview projection matrix
    glm::mat4 MVP	= P*MV;


	//enable blending and bind the cube vertex array object
	glEnable(GL_BLEND);
	glBindVertexArray(render.cubeVAOID);
		//bind the raycasting shader
		render.UseProgram();
		glUniform3f(render("step_size"), 1.0f / XDIM, 1.0f / YDIM, 1.0f / ZDIM);
		glUniform1i(render("volume"), 0);
			//pass shader uniforms
			glUniformMatrix4fv(render("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
			glUniform3fv(render("camPos"), 1, &(camPos.x));
			static float Delta = 0.0f;
			glUniform1f(render("delta"), Delta);
			Delta += 0.001f;

				//render the cube
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//unbind the raycasting shader
		render.UnUseProgram();
	//disable blending
	glDisable(GL_BLEND);

	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	//freeglut initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Volume Rendering using GPU Ray Casting - OpenGL 3.3");

	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
		cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
			cout<<"Driver supports OpenGL 3.3\nDetails:"<<endl;
		}
	}
	err = glGetError(); 
	

	//output hardware information
	cout<<"\tUsing GLEW "<<glewGetString(GLEW_VERSION)<<endl;
	cout<<"\tVendor: "<<glGetString (GL_VENDOR)<<endl;
	cout<<"\tRenderer: "<<glGetString (GL_RENDERER)<<endl;
	cout<<"\tVersion: "<<glGetString (GL_VERSION)<<endl;
	cout<<"\tGLSL: "<<glGetString (GL_SHADING_LANGUAGE_VERSION)<<endl;



	//OpenGL initialization
	InitOpenGL();

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