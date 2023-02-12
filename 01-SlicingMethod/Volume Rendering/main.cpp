#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <algorithm>




#ifdef _DEBUG 
#pragma comment(lib,"opengl32.lib")//for linking
#pragma comment(lib,"glew32.lib")//for linking
#pragma comment(lib, "freeglut_static_x86_d.lib")
#else
#pragma comment(lib, "glew_static_x86.lib")
#endif
const int WIDTH = 1920;
const int HEIGHT = 1080;




void OnShutdown() {
	
}

//resize event handler
void OnResize(int w, int h) {

}

//display function
void display() {
	
}

//Keyboard Callback
void OnKey(unsigned char key, int x, int y) {
	
}
void OnMouseDown(int button, int s, int x, int y)
{
	
}

//mouse move event handler
void OnMouseMove(int x, int y)
{
	
}
void InitializeOpenGL()
{

}


int main(int argc, char** argv) {

	//Initialize the GLUT library
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
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	}
	else {
		if (GLEW_VERSION_3_3)
		{
			std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
		}
	}
	err = glGetError(); //this is to ignore INVALID ENUM error 1282
	//GL_CHECK_ERRORS

	//output hardware information
	std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	//GL_CHECK_ERRORS

	//OpenGL initialization
	InitializeOpenGL();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(display);
	glutReshapeFunc(OnResize);
	glutMouseFunc(OnMouseDown);
	glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);

	//main loop call
	glutMainLoop();

	return 0;
}