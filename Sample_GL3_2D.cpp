#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstring>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <FTGL/ftgl.h>

#include <SOIL/SOIL.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>



using namespace std;



struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;


GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void reshapeWindow (GLFWwindow* window, int width, int height);
GLFWwindow* initGLFW (int width, int height);

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);


	return glm::vec3(1,1,1);
	/*if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);*/
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float moverotate=0, movetranslate=0,rotatestatus=0;
float ballx=-2.5,bally=-2.7,g=0.1,ballx2=3.0,bally2=-3.7;
int incrementer=0;
/**************************
 * Customizable functions *
 **************************/
float rotateangle=0, xvelocity=1,yvelocity=1,theta=43.5,velocity=0.7,alpha=0.7;
float triangle_rot_dir = 1, friction=0.001,zoom=0,pan=0;
int limitv=7,limitflag=0,gflag=0;
float rectangle_rot_dir = 1,upy=6,downy=-6,drag=0,yvelocity2=1.4;
int shoot=0,flag=0,rect3flag=0,rollflag=0,hitflag=0,lives=3,readerflag=0,flag2=0;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
long long int score=0;
float timed=0.02;
double initialx,initialy,mousex,mousey;

int difficulty=1,arrow=0,level=1,gameover=0;
int width = 1366;
int height = 768;

GLFWwindow* window = initGLFW(width, height);


/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE&&gameover==0) {
        switch (key) {
            case GLFW_KEY_A:
            	if(rotateangle<9.5)
                rotateangle+=0.5;
		theta=45.0+(5*rotateangle);
                break;
            case GLFW_KEY_B:
            	if(rotateangle>-9)
                rotateangle-=0.5;
		theta=45.0+(5*rotateangle);
                break;
	    case GLFW_KEY_F:
            	if(velocity<2.0)
                {velocity+=0.1;
		limitv+=1;}
                break;
	    case GLFW_KEY_S:
            	if(velocity>0)
                {velocity-=0.1;
		limitv-=1;}
                break;
	    case GLFW_KEY_R: 
		//if(rollflag==1){                  //ready for reshoot
		shoot=0;
		flag=0;
		limitflag=0;gflag=0;
		rollflag=0;
		alpha=0.7;hitflag=0;readerflag=0;
		//friction=0.001;
		friction=(float)(rand()%30)/(float)1000;
		//cout<<friction<<endl;
		if(level==5)
		g=0.2545;
		else
		g=0.1;
		if(lives<=0)
		gameover=1;
		//}
                break;
	    case GLFW_KEY_ENTER:
		limitflag=1;
                break;

	    case GLFW_KEY_UP:
            	zoom+=0.5;
		reshapeWindow (window, width, height);	
                break;
	     case GLFW_KEY_DOWN:
            	zoom-=0.5;
		reshapeWindow (window, width, height);	
                break;

	    case GLFW_KEY_LEFT:
            	pan-=0.5;
		reshapeWindow (window, width, height);	
                break;
	     case GLFW_KEY_RIGHT:
            	pan+=0.5;
		reshapeWindow (window, width, height);	
                break;
	    
            case GLFW_KEY_SPACE:
                if(shoot==0){shoot=1;lives-=1;}
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom+=yoffset*0.5;
	reshapeWindow (window, width, height);
	
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS&&shoot==0&&gameover==0){
            	theta=atan((mousey+3.4)/(mousex+6.4));
		theta*=180.0f/M_PI;
		if(theta>90)theta=90;
		//cout<<((mousey+3.4)/(mousex+6.4))<<endl;
		//cout<<theta<<endl;;
		rotateangle=(theta-45)/5;
                xvelocity=velocity*cos(theta*M_PI/180.0f);
		yvelocity=velocity*sin(theta*M_PI/180.0f);
		ballx=(0.9*(cos(rotateangle*5*M_PI/180.0f)))-((0.7)*(sin(rotateangle*5*M_PI/180.0f)))-6.4;
		bally=(0.7*(cos(rotateangle*5*M_PI/180.0f)))+((0.9)*(sin(rotateangle*5*M_PI/180.0f)))-3.4;
		}
	     if (action == GLFW_RELEASE&&gameover==0) {
	     	if(shoot==0){shoot=1;lives-=1;}
	     }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS&&gameover==0) {
                 initialx=mousex;
		 
		 
            }
            if (action == GLFW_RELEASE&&gameover==0) {
            	drag=mousex-initialx;
                 pan-=drag;
		reshapeWindow (window, width, height);
		if(velocity<2.0)
                {velocity+=0.1;
		limitv+=1;}
		else if(velocity>=2.0)
		{
			velocity=0;
			limitv=0;
		}
            }
            break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
          
            if (action == GLFW_PRESS&&gameover==0) {
            	shoot=0;
		flag=0;
		limitflag=0;gflag=0;
		rollflag=0;
		alpha=0.7;hitflag=0;readerflag=0;
		//friction=0.001;
		friction=(float)(rand()%30)/(float)1000;
		//cout<<friction<<endl;
		if(level==5)
		g=0.2545;
		else
		g=0.1;
		if(lives<=0)
		gameover=1;
            }
            break;
        default:
            break;
    }


}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-8.0+zoom+pan, 8.0-zoom+pan, -4.0+zoom, 4.0-zoom, 0.1, 500.0);
}

VAO *triangle, *rectangle1, *triangle1, *rectangle2, *rectangle3, *triangle2;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, -0.5,0, // vertex 0
    -0.5,0.5,0, // vertex 1
    0.5,0.5,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 0
    1,1,1, // color 1
    1,1,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTriangle1 ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, -0.5,0, // vertex 0
    -0.5,0.5,0, // vertex 1
    0.5,0.5,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 0
    1,1,1, // color 1
    1,1,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle1 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createTriangle2 ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, -0.5,0, // vertex 0
    -0.5,0.5,0, // vertex 1
    0.5,0.5,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 0
    1,1,1, // color 1
    1,1,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle2 = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle2 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle3 ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */
  for (int i=0;i<360;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle = glm::scale (glm::vec3(0.6f, 0.6f, 0.6f));
  glm::mat4 translateTriangle = glm::translate (glm::vec3(-6.5+movetranslate,-3.7,0)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)((i+moverotate)*M_PI/180.0f), glm::vec3(0,-0.5,1));  // Cannon
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle * scaleTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);
  }
  moverotate+=5*rotatestatus;
  movetranslate+=0.01*rotatestatus;


  
  
  for (int i=0;i<360;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle = glm::scale (glm::vec3(0.3f, 0.3f, 0.3f));
  glm::mat4 translateTriangle = glm::translate (glm::vec3(movetranslate,0,0)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(i*M_PI/180.0f), glm::vec3(0,-0.5,1));  		// Ball
  glm::mat4 translateCircle1 = glm::translate (glm::vec3(ballx, bally, 0)); 
  //glm::mat4 translateCircle1 = glm::translate (glm::vec3(6.4-movetranslate, 3.4, 0)); 
  //glm::mat4 rotateCircle = glm::rotate((float)(5*rotateangle*M_PI/180.0f), glm::vec3(0,0,1));
  //glm::mat4 translateCircle2 = glm::translate (glm::vec3(-6.4+movetranslate, -3.4, 0)); 
  glm::mat4 triangleTransform =translateCircle1 * translateTriangle * rotateTriangle * scaleTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle1);
  }

if(rect3flag==2){
for (int i=0;i<360;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle20 = glm::scale (glm::vec3(0.5f, 0.5f, 0.5f));
  glm::mat4 translateTriangle20 = glm::translate (glm::vec3(0,0,0)); // glTranslatef
  glm::mat4 rotateTriangle20 = glm::rotate((float)(i*M_PI/180.0f), glm::vec3(0,-0.5,1));  		// Level 3 Ball
  glm::mat4 translateCircle20 = glm::translate (glm::vec3(ballx2, bally2, 0)); 
  //glm::mat4 translateCircle1 = glm::translate (glm::vec3(6.4-movetranslate, 3.4, 0)); 
  //glm::mat4 rotateCircle = glm::rotate((float)(5*rotateangle*M_PI/180.0f), glm::vec3(0,0,1));
  //glm::mat4 translateCircle2 = glm::translate (glm::vec3(-6.4+movetranslate, -3.4, 0)); 
  glm::mat4 triangleTransform20 =translateCircle20 * translateTriangle20 * rotateTriangle20 * scaleTriangle20;
  Matrices.model *= triangleTransform20; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(triangle1);
  }
  }



if(rect3flag==3){
for (int i=0;i<360;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle40 = glm::scale (glm::vec3(0.5f, 0.5f, 0.5f));
  glm::mat4 translateTriangle40 = glm::translate (glm::vec3(0,0,0)); // glTranslatef
  glm::mat4 rotateTriangle40 = glm::rotate((float)(i*M_PI/180.0f), glm::vec3(0,-0.5,1));  		// Level 4 Ball
  glm::mat4 translateCircle40 = glm::translate (glm::vec3(ballx2, bally2, 0)); 
  //glm::mat4 translateCircle1 = glm::translate (glm::vec3(6.4-movetranslate, 3.4, 0)); 
  //glm::mat4 rotateCircle = glm::rotate((float)(5*rotateangle*M_PI/180.0f), glm::vec3(0,0,1));
  //glm::mat4 translateCircle2 = glm::translate (glm::vec3(-6.4+movetranslate, -3.4, 0)); 
  glm::mat4 triangleTransform40 =translateCircle40 * translateTriangle40 * rotateTriangle40 * scaleTriangle40;
  Matrices.model *= triangleTransform40; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(triangle1);
  }
  }

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle = glm::scale (glm::vec3(0.6f, 0.13f, 0.3f));
  glm::mat4 translateRectangle = glm::translate (glm::vec3(-6.05+movetranslate, -3.05, 0));        // Cannon
  glm::mat4 rotateRectangle = glm::rotate((float)(42*M_PI/180.0f), glm::vec3(0,0,1)); 
  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(6.4-movetranslate, 3.4, 0)); 
  glm::mat4 rotateRectangle3 = glm::rotate((float)(5*rotateangle*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 translateRectangle3 = glm::translate (glm::vec3(-6.4+movetranslate, -3.4, 0)); 
  Matrices.model *= (translateRectangle3 * rotateRectangle3 *translateRectangle1* translateRectangle * rotateRectangle * scaleRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle1);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle8 = glm::scale (glm::vec3(7.0f, 0.1f, 0.3f));
  glm::mat4 translateRectangle8 = glm::translate (glm::vec3(0, -4.0, 0));        // Land
  Matrices.model *= (translateRectangle8 * scaleRectangle8);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(rectangle1);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle2 = glm::shear (glm::vec3(0.6f, 0.13f, 0.3f));
  glm::mat4 translateRectangle2 = glm::translate (glm::vec3(-5.9+movetranslate, -3.2, 0));        // Cannon
  glm::mat4 rotateRectangle2 = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1));               
  glm::mat4 translateRectangle4 = glm::translate (glm::vec3(6.4-movetranslate, 3.4, 0)); 
  glm::mat4 rotateRectangle4 = glm::rotate((float)(5*rotateangle*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 translateRectangle5 = glm::translate (glm::vec3(-6.4+movetranslate, -3.4, 0)); 
  Matrices.model *= (translateRectangle5*rotateRectangle4 * translateRectangle4*translateRectangle2 * rotateRectangle2 * scaleRectangle2);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle2);


Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle14 = glm::scale (glm::vec3(0.3f, 0.3f, 0.3f));
  glm::mat4 translateRectangle14 = glm::translate (glm::vec3(3, 2, 0));
  Matrices.model *= translateRectangle14 * scaleRectangle14;                       // Target1
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 

if(rect3flag==0)
  draw3DObject(rectangle3);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle3 = glm::scale (glm::vec3(0.3f, 0.3f, 0.3f));
  glm::mat4 translateRectangle7 = glm::translate (glm::vec3(6, 2, 0));                         // Target2
  Matrices.model *= translateRectangle7 * scaleRectangle3;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 

if(rect3flag==1)
  draw3DObject(rectangle3);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle12 = glm::scale (glm::vec3(1.8f, 0.10f, 0.3f));
  glm::mat4 translateRectangle12 = glm::translate (glm::vec3(1, upy, 0));        // glTranslatef
  glm::mat4 rotateRectangle12 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // Obstacle 1
  Matrices.model *= translateRectangle12 * rotateRectangle12 * scaleRectangle12;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(rect3flag==1||rect3flag==3)
  draw3DObject(rectangle3);
  /*Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);*/
Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle13 = glm::scale (glm::vec3(1.8f, 0.10f, 0.3f));
  glm::mat4 translateRectangle13 = glm::translate (glm::vec3(1, downy, 0));        // glTranslatef
  glm::mat4 rotateRectangle13 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // Obstacle 2
  Matrices.model *= translateRectangle13 * rotateRectangle13 * scaleRectangle13;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(rect3flag==1||rect3flag==3)
  draw3DObject(rectangle3);


for(int i=0;i<limitv;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle6 = glm::scale (glm::vec3(0.2f, 0.05f, 0.3f));
  glm::mat4 translateRectangle6 = glm::translate (glm::vec3(-7.5, -1.0+i*0.2, 0));        // Power
  Matrices.model *= translateRectangle6 * scaleRectangle6;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle2);
 }

for(int i=0;i<(int)(friction*1000);i++){
	Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle10 = glm::scale (glm::vec3(0.1, 0.03f, 0.3f));
  glm::mat4 translateRectangle10 = glm::translate (glm::vec3(7.5-i*0.2, 3.0, 0));        // Windspeed
  Matrices.model *= translateRectangle10 * scaleRectangle10;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  if(rollflag!=1)
  draw3DObject(rectangle2);
}

if(rollflag!=1)
{
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle11 = glm::scale (glm::vec3(0.25f, 0.25f, 0.25f));
  glm::mat4 translateTriangle11 = glm::translate (glm::vec3(0+7.5-(friction*200),3.0,0)); // glTranslatef
  glm::mat4 rotateTriangle11 = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1));  // Windspeed arrow
  glm::mat4 triangleTransform =translateTriangle11 * rotateTriangle11 * scaleTriangle11;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle1);
}

for(int j=0;j<lives;j++)
{
	
for (int i=0;i<360;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle5 = glm::scale (glm::vec3(0.3f, 0.3f, 0.3f));
  glm::mat4 translateTriangle5 = glm::translate (glm::vec3(-7+j*0.5,3.5,0)); // glTranslatef
  glm::mat4 rotateTriangle5 = glm::rotate((float)(i*M_PI/180.0f), glm::vec3(0,-0.5,1));          // Lives
  glm::mat4 triangleTransform = translateTriangle5 * rotateTriangle5 * scaleTriangle5;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle1);
  }

}



Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle16 = glm::scale (glm::vec3(0.5, 0.3, 1.0f));
  glm::mat4 translateRectangle16 = glm::translate (glm::vec3(4.5 ,-3.8, 0));        // Blower platform`
  Matrices.model *= translateRectangle16 *scaleRectangle16;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(rect3flag==1)
  draw3DObject(rectangle2);



for(int i=0;i<arrow;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle15 = glm::scale (glm::vec3(0.03, 0.2, 1.0f));
  glm::mat4 translateRectangle15 = glm::translate (glm::vec3(4.1 ,-3.0+i*0.2, 0));        // Blower arrow1`
  Matrices.model *= translateRectangle15 *scaleRectangle15;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(rect3flag==1)  
draw3DObject(rectangle2);
}


for(int i=0;i<arrow;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle17 = glm::scale (glm::vec3(0.03, 0.2, 1.0f));
  glm::mat4 translateRectangle17 = glm::translate (glm::vec3(4.5 ,-3.0+i*0.2, 0));        // Blower arrow2`
  Matrices.model *= translateRectangle17 *scaleRectangle17;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(rect3flag==1) 
 draw3DObject(rectangle2);
}

for(int i=0;i<arrow;i++){
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle16 = glm::scale (glm::vec3(0.03, 0.2, 1.0f));
  glm::mat4 translateRectangle16 = glm::translate (glm::vec3(4.9 ,-3.0+i*0.2, 0));        // Blower arrow3
  Matrices.model *= translateRectangle16 *scaleRectangle16;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(rect3flag==1) 
 draw3DObject(rectangle2);
}

Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle13 = glm::scale (glm::vec3(0.25f, 0.25f, 0.25f));
  glm::mat4 translateTriangle13 = glm::translate (glm::vec3(4.1,-2.0,0)); // glTranslatef
  glm::mat4 rotateTriangle13 = glm::rotate((float)(-180*M_PI/180.0f), glm::vec3(0,0,1));  // Bolwer arrowhead1
  glm::mat4 triangleTransform3 =translateTriangle13*rotateTriangle13* scaleTriangle13;
  Matrices.model *= triangleTransform3; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(arrow==5&&rect3flag==1)  
draw3DObject(triangle1);

Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle14 = glm::scale (glm::vec3(0.25f, 0.25f, 0.25f));
  glm::mat4 translateTriangle14 = glm::translate (glm::vec3(4.5,-2.0,0)); // glTranslatef
  glm::mat4 rotateTriangle14 = glm::rotate((float)(-180*M_PI/180.0f), glm::vec3(0,0,1));  // Blower arrowhead2
  glm::mat4 triangleTransform2 =translateTriangle14*rotateTriangle14* scaleTriangle14;
  Matrices.model *= triangleTransform2; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(arrow==5&&rect3flag==1)  
draw3DObject(triangle1);

Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle12 = glm::scale (glm::vec3(0.25f, 0.25f, 0.25f));
  glm::mat4 translateTriangle12 = glm::translate (glm::vec3(4.9,-2.0,0)); // glTranslatef
  glm::mat4 rotateTriangle12 = glm::rotate((float)(-180*M_PI/180.0f), glm::vec3(0,0,1));  // Blower arrowhead3
  glm::mat4 triangleTransform =translateTriangle12*rotateTriangle12* scaleTriangle12;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(arrow==5&&rect3flag==1)  
draw3DObject(triangle1);


Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleRectangle21 = glm::scale (glm::vec3(0.3, 0.7, 1.0f));
  glm::mat4 translateRectangle21 = glm::translate (glm::vec3(3.0 ,-3.8, 0));        // Shooter platform`
  Matrices.model *= translateRectangle21 *scaleRectangle21;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(rect3flag==2||rect3flag==3)
  draw3DObject(rectangle2);


Matrices.model = glm::mat4(1.0f);
  glm::mat4 scaleTriangle30 = glm::scale (glm::vec3(1.0f, 1.0f, 1.0f));
  glm::mat4 translateTriangle30 = glm::translate (glm::vec3(3.0,-3.2,0)); // glTranslatef
  glm::mat4 rotateTriangle30 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));  // Shooter platform
  glm::mat4 triangleTransform30 =translateTriangle30*rotateTriangle30* scaleTriangle30;
  Matrices.model *= triangleTransform30; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
if(rect3flag==2||rect3flag==3)  
draw3DObject(triangle1);
















 // Increment angles
  float increments = 1;


// Render font on screen
	static int fontScale = 0;
	float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
	glm::vec3 fontColor = getRGBfromHue (fontScale);



	// Use font Shaders for next part of code
	glUseProgram(fontProgramID);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(5,3.5,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(0.6,0.6,0.6));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s[10];
	strcpy(s,"Score: ");
	char buffer[100];
	sprintf(buffer, "%lld", score);
	strcat(s, buffer);
	// Render font
	GL3Font.font->Render(s);


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText1 = glm::translate(glm::vec3(-7.2,3.0,0));
	glm::mat4 scaleText1 = glm::scale(glm::vec3(0.5,0.5,0.5));
	Matrices.model *= (translateText1 * scaleText1);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);

	if(readerflag==1)
		GL3Font.font->Render("Press R or Middle mouse click to Reload");


	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText2 = glm::translate(glm::vec3(7.0,3.15,0));
	glm::mat4 scaleText2 = glm::scale(glm::vec3(0.5,0.4,0.5));
	Matrices.model *= (translateText2 * scaleText2);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
if(rollflag!=1)
	GL3Font.font->Render("WIND");


if(rect3flag==1) {
Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText3 = glm::translate(glm::vec3(5.1,-2.0,0));
	glm::mat4 rotateText3 = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1));
	glm::mat4 scaleText3 = glm::scale(glm::vec3(0.5,0.4,0.5));
	Matrices.model *= (translateText3 *rotateText3* scaleText3);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Blowers");
}


if(rect3flag==2||rect3flag==3) {
Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText3 = glm::translate(glm::vec3(3.5,-2.75,0));
	glm::mat4 rotateText3 = glm::rotate((float)(-90*M_PI/180.0f), glm::vec3(0,0,1));
	glm::mat4 scaleText3 = glm::scale(glm::vec3(0.5,0.4,0.5));
	Matrices.model *= (translateText3 *rotateText3* scaleText3);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Shooter");
}



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText4 = glm::translate(glm::vec3(-2,3.5,0));
	glm::mat4 scaleText4 = glm::scale(glm::vec3(0.6,0.6,0.6));
	Matrices.model *= (translateText4 * scaleText4);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s1[10];
	strcpy(s1,"LEVEL ");
	char buffer1[100];
	sprintf(buffer1, "%d", level);
	strcat(s1, buffer1);
	// Render font
	GL3Font.font->Render(s1);



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(-1.6,-0.5,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(0.8,0.8,0.8));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s2[100];
	strcpy(s2,"Score: ");
	char buffer2[100];
	sprintf(buffer2, "%lld", score);
	strcat(s2, buffer2);
	// Render font
	if(lives<=0&&(rollflag==1||gameover>=1))
	GL3Font.font->Render(s2);



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText6 = glm::translate(glm::vec3(-2,0,0));
	glm::mat4 scaleText6 = glm::scale(glm::vec3(1.0,1.0,1.0));
	Matrices.model *= (translateText6 * scaleText6);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	// Render font
	if(lives<=0&&(rollflag==1||gameover>=1))
	GL3Font.font->Render("Game Over");



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText7 = glm::translate(glm::vec3(-3,1.0,0));
	glm::mat4 scaleText7 = glm::scale(glm::vec3(1.0,1.0,1.0));
	Matrices.model *= (translateText7 * scaleText7);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	// Render font
	if(level==5)
	GL3Font.font->Render("JUPITER");



	//camera_rotation_angle++; // Simulating camera rotation
	triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	// font size and color changes
	//fontScale = (fontScale + 1) % 360;

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

float distance(float x,float y,float a,float b)
{
	return sqrt(pow((x-a),2)+pow((y-b),2));
}

bool checkCollision(float xcor,float ycor, float radius)
{
	if(distance(ballx,bally,xcor,ycor)<=0.3354+radius)
		return true;
	else
		return false;
}





/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createRectangle2 ();
	createRectangle3 ();
	createTriangle1();
	createTriangle2();
	
	glActiveTexture(GL_TEXTURE0);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

const char* fontfile = "arial.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    double last_update_time3 = glfwGetTime();
    double last_update_time2 = glfwGetTime();
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)&&gameover!=20) {
	

	 glfwGetCursorPos(window,&mousex,&mousey);
	mousex=(mousex-1366/2)/85.375;mousey=(mousey-768/2)/96;mousey*=-1;
	//cout<<mousex<<" "<<mousey<<endl;

        // OpenGL Draw commands
        draw();
	if(shoot==0){
		/*if(theta>90)
		theta=90;
		if(theta<0)
		theta=0;*/
	//velocity=limitv*0.1;
        xvelocity=velocity*cos(theta*M_PI/180.0f);
	yvelocity=velocity*sin(theta*M_PI/180.0f);
	ballx=(0.9*(cos(rotateangle*5*M_PI/180.0f)))-((0.7)*(sin(rotateangle*5*M_PI/180.0f)))-6.4;
	bally=(0.7*(cos(rotateangle*5*M_PI/180.0f)))+((0.9)*(sin(rotateangle*5*M_PI/180.0f)))-3.4;
	
	flag=0;
	
	}
	
	/*cout<<"ballx:"<<ballx<<endl;
	  cout<<"bally:"<<bally<<endl;
	  cout<<"v:"<<velocity<<"x:"<<xvelocity<<"y:"<<yvelocity<<endl;*/

	//cout<<g<<endl;
	glfwSetScrollCallback(window, scroll_callback);

 	/*int xpos, ypos;
 	glfwGetMousePos(&xpos, &ypos);*/



	 

        if(checkCollision(3,2,0.4646)==true&&rect3flag==0)	//Level 1
		{
		rect3flag=1;
		score+=difficulty*5*(lives+1);
		difficulty++;
		lives=3;
		level++;
		}
		
	if(checkCollision(6,2,0.4646)==true&&rect3flag==1)	//Level 2
		{
		rect3flag=2;
		score+=difficulty*5*(lives+1);
		difficulty+=2;
		lives=5;
		g=0.1;
		level++;
		}

	if(checkCollision(ballx2,bally2,0.3535)==true&&rect3flag==2&&level==3)	//Level 3
		{
		rect3flag=3;
		score+=difficulty*5*(lives+1);
		difficulty+=3;
		lives=5;
		upy=6;downy=-6;
		flag2=0;ballx2=3.0;bally=-3.7;yvelocity2=1.8;
		level++;
		timed=0.01;
		}

	if(checkCollision(ballx2,bally2,0.3535)==true&&rect3flag==3)	//Level 4
		{
		rect3flag=1;
		score+=difficulty*5*(lives+1);
		difficulty+=4;
		lives=3;
		upy=6;downy=-6;
		level++;
		g=0.2545;
		yvelocity2=3.6;
		}


	if(checkCollision(6,2,0.4646)==true&&rect3flag==1)	//Level 5
		{
		rect3flag=1;
		score+=difficulty*5*(lives+1);
		difficulty+=4;
		gameover=1;
		}

	if(bally<=-3.75)
	{
		bally=-3.74;
		yvelocity=alpha*velocity*sin(theta*M_PI/180.0f);
		flag=0;
		alpha*=alpha;
		if(yvelocity<0.1)
			{yvelocity=0;rollflag=1;friction=0.03;readerflag=1;}
	}


	
	
	if(ballx>8.0||ballx<-8.0||bally>4.0||bally<-4.0)
		readerflag=1;

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.05s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.01) { // atleast 0.05s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;


		//score++;


	



	    if(flag==0 && yvelocity>0 && shoot==1 && rollflag==0)
	  {
	  	yvelocity-=g*0.2;
		bally=(bally+yvelocity*0.2)*shoot;

	  }
	  if(yvelocity<=0 && rollflag==0)
		{flag=1;}
		//yvelocity=0;	}
	  if(flag==1 && shoot==1 && rollflag==0)
	  {
		yvelocity+=g*0.2;
		bally=(bally-yvelocity*0.2)*shoot;	
	  }
	  
	  //cout<<zoom<<endl;
          if(shoot==1){
          	if(hitflag==0)
          	{xvelocity-=friction*0.2;
		if(xvelocity<0)
		xvelocity=0;
			
		}
		else
		{xvelocity+=friction*0.2;
		if(xvelocity>0)
		{
			xvelocity=0;
		}}
		if(xvelocity>0)
	     ballx=(ballx+xvelocity*0.2)*shoot;
		else if(xvelocity<0&&hitflag==1)
		ballx=(ballx+xvelocity*0.2)*shoot;
		if(ballx>=0.55&&ballx<=0.55+xvelocity*0.2&&(bally<=-1.0||bally>=0.07)&&(rect3flag==1||rect3flag==3)){
			xvelocity=-1*xvelocity;
			hitflag=1;}
		}
		if(gflag==1)
		{g=0.1;incrementer=0;gflag=0;}

		if((ballx>=2.0&&ballx<=2.0+xvelocity*0.2)&&gflag==0&&rect3flag==1)
		{
			g=-0.2;
			if(incrementer>=3)
			gflag=1;
			incrementer+=1;
		}
		
	  /*if(limitflag!=1)
		{
			if(limitv<20)
			limitv+=1;
			else
			limitv=0;
		}
	cout<<limitv<<endl;*/

		if(upy>2.1&&(rect3flag==1||rect3flag==3))
		upy-=0.05;
		if(downy<-3.5&&(rect3flag==1||rect3flag==3))
		downy+=0.04;
		
		
		
	  
		




        }
	if((current_time - last_update_time2) >= 0.2){
		 last_update_time2 = current_time;
		if(arrow<5)
		arrow++;
		else
		arrow=0;
	if(lives<=0&&(rollflag==1||gameover>=1))
	gameover+=1;
		if(bally2<=-3.71&& (rect3flag==2))
	{
		yvelocity2=1.4;
	}

if(bally2<=-3.71&& (rect3flag==3))
	{
		yvelocity2=1.8;
	}
}




if((current_time - last_update_time3) >= timed){
		 last_update_time3 = current_time;

		if(flag2==0 && yvelocity2>0 && (rect3flag==2||rect3flag==3))
	  {
	  	yvelocity2-=g*0.2;
		bally2=(bally2+yvelocity2*0.2);

	  }
	  if(yvelocity2<=0&& (rect3flag==2||rect3flag==3))
		{flag2=1;}
		//yvelocity=0;	}
	  if(flag2==1&& (rect3flag==2||rect3flag==3))
	  {
		yvelocity2+=g*0.2;
		bally2=(bally2-yvelocity2*0.2);	
		if(bally2<=-3.71&& (rect3flag==2||rect3flag==3))
	{
		bally2=-3.70;
		if(rect3flag==2)
		yvelocity2=1.4;
		else if(rect3flag==3)
		yvelocity2=1.8;
		flag2=0;
	}
	  }
	  /*cout<<"ballx:"<<ballx2<<endl;
	  cout<<"bally:"<<bally2<<endl;
	  cout<<"y:"<<yvelocity2<<endl;*/

	}

    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
