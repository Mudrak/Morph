/********************************************************************
* FILE: 	morph1.cpp
* NAME: 	Erik Mudrak
* DATE LAST MODIFIED:  02/20/2015
*
*  Illustrates using the vertex shader to perform basic morphing
*   - Passes 2 arrays to the shader to be used for morphing
*   - Uses Idle Function
*   - sets up elapsed time parameter for use by vertex shader
	- Preserves viewport upon window resizing
	- Assigns color to individual vertices
	- Loads vertice values from separate files
	- **** Takes number of frames from command line ****
*
*  Makes use of 
*	 - "subdata" to pack info about all the shapes into the same buffer.
*    - glutIdleFunc
*
* Assumes shaders vmorph.glsl and fmorph.glsl are in same directory
********************************************************************/
#include "Angel.h"
// Array of points in seperate files
#include "startPoints.h"
#include "middlePoints.h"
#include "destPoints.h"
#include <iostream>
using namespace std;
using namespace Angel;

//GLOBALS:
GLuint          time_location;
GLuint          vertices_A_location;
GLuint          vertices_B_location;
GLuint		   	vertices_C_location;
GLuint		   	program;
// Added globals:
GLuint 			frame_loc;
int win_width = 800;
int win_height = 600;
bool paused = false;
int num_frames = 500;	

//FUNCTIONS:
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void idle();
void SetAutoVP();
void reshape (int width, int height);
//void fillTween(vec4 startArray, vec4 destArray, vec4 tweenArray); 
//vec4 tween (vec4 A, vec4 B);

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );  //Note double buffering
    glutInitWindowSize(win_width, win_height);

	num_frames = atoi( argv[1] ); 		//convert the string argument to integer
  	glutCreateWindow( argv[1] );	   //use the string version for window title
	//glutCreateWindow("Pokemon");
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
	glutReshapeFunc( reshape );
    glutIdleFunc(idle);
    init();
    glutMainLoop();
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////
void init()
{	
	
	// Header file brings in vertices_A, vertices_B, and vertices_C

	GLuint vao[1];
    #ifdef __APPLE__       // For use with OS X
      glGenVertexArraysAPPLE(1, vao );
      glBindVertexArrayAPPLE(vao[0] );
    #else		       	// Other (Linux)
      glGenVertexArrays(1, vao );
      glBindVertexArray(vao[0] );       
    #endif

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices_A) + sizeof(vertices_B) + sizeof(vertices_C) , NULL, GL_STATIC_DRAW );

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_A), vertices_A);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_A),  sizeof(vertices_B), vertices_B);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_A)+ sizeof(vertices_B), sizeof(vertices_C),  vertices_C);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_A)+ sizeof(vertices_B), sizeof(vertices_C) +sizeof(paramArray),  paramArray);

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vmorph.glsl", "fmorph.glsl" );
    glUseProgram( program );

/////////// A ///////////

	// Initialize the vertex color attribute
	glPointSize(3.0);
	GLuint color_location = glGetAttribLocation( program, "vColor" );
	glEnableVertexAttribArray(color_location);
	glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(0) );

	// Initialize the vertex position attribute from the vertex shader
	GLuint vertices_A_location = glGetAttribLocation( program, "vertices1" );
    glEnableVertexAttribArray( vertices_A_location );
	glVertexAttribPointer( vertices_A_location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	glVertexAttribPointer(vertices_A_location, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof	(vertices_A[0].color)));

////////// B ////////////

	// Initialize the vertex position attribute from the vertex shader
	GLuint vertices_B_location = glGetAttribLocation( program, "vertices2" );
    glEnableVertexAttribArray( vertices_B_location );
	glVertexAttribPointer(vertices_B_location, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(vertices_A) + sizeof(vertices_B[0].color)));

////////// C ///////////

	// Initialize the vertex position attribute from the vertex shader
	GLuint vertices_C_location = glGetAttribLocation( program, "vertices3" );
	glEnableVertexAttribArray( vertices_C_location );
	glVertexAttribPointer(vertices_C_location, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), BUFFER_OFFSET(sizeof(vertices_A) + sizeof(vertices_B) + sizeof(vertices_C[0].color)));
	
	time_location = glGetUniformLocation(program, "time"); 
	
	// Sends value of frame_speed to the shader
	GLfloat speed = 1.0 / (num_frames - 1.0); // WHY is it always 0?
	cout << "Frame speed: " << speed << endl;
	frame_loc = glGetUniformLocation(program, "t");
	glUniform1f(frame_loc, speed);
	
    glClearColor(0.8, 0.9, 1.0, 1.0); /* blue-white background */
  	glLineWidth(2.0);
}

void display()
{
	if (!paused) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// send elapsed time to shaders 
		std::cout<<time_location<<"\n";  //testing purposes
		glUniform1f(time_location, glutGet(GLUT_ELAPSED_TIME));
		SetAutoVP();
		
		glDrawArrays(GL_LINE_STRIP, 0, numpoints);

		glutSwapBuffers();
	}
}

/*Preconditon: leftx, lefty are the x,y coordinates of the lower left corner of the viewport rectangle to be set; rightx and righty are the x,y coordinates of the upper right corner of the rectangle.  Both sets of coordinates are given in percentages of the screen window.  For example, if leftx=0.25 and lefty=0.5, the lower left corner of the viewport has been set to a pixel that is 25% from the left side of the window and 50% from the bottom.
 *Postcondition:  the appropriate viewport has been set. Automatically chooses a viewport to preserve the aspect ratio in your NDC coordinates every time the window is changed 
*/
	
// In form (L, B, R, T):
void SetAutoVP()
{

win_width = glutGet(GLUT_WINDOW_WIDTH);
win_height = glutGet(GLUT_WINDOW_HEIGHT);
double 			frame_width = 2.0;
double 			frame_height = 2.0;
double 			winAR = win_width / win_height;
double 			frameAR = frame_width / frame_height;
double 			VPx, VPy, VPw, VPh;
	
	if (frameAR > winAR) 
	{
		VPx = 0;
		VPw = win_width;
		VPh = win_width * (1.0 / frameAR); 
		VPy = (win_height - VPh) / 2.0;
		glViewport(VPx, VPy, VPw, VPh);
	}
	if (frameAR < winAR)
	{ 	
		VPy = 0;
		VPh = win_height;
		VPw = frameAR * VPh;
		VPx = (win_width - VPw) / 2.0;
		glViewport(VPx, VPy, VPw, VPh);
	}
}

// Accounts for changes in window size
void reshape (int width, int height) {
	glViewport(0, 0, width, height);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 27:
        exit(0);
        break;
    default:
        break;
	case 'g':
		paused = false;
		break;
	case 's':
		paused = true;
		break;
    }
}

void idle()
{
   glUniform1f(time_location, glutGet(GLUT_ELAPSED_TIME));
   glutPostRedisplay();
}





