/*
*
* Author: Anthony Spedaliere (student)
* Organization: Southern New Hampshire University
* Class: CS-330-T2833 Comp Graphic and Visualization 22EW2
* Date: December 7, 2022
* Assignment: 7-1 Final Project
*
*/

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GLEW/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// camera class
#include <learnOpengl/camera.h> 

// GLM Math Header inclusions
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

// Standard namespace
using namespace std;

// function prototypes
void PrintShaderCompileError(GLuint shader);
void PrintShaderLinkingError(int prog);
bool IsOpenGLError(); // error checking
void CreateVertices(); // initialize vertices, and create VBO, VAO, etc
void drawPencil(); // apply transforms to the barrel of the pencil object
void drawTip(); // apply transforms to the tip of the pencil object
void drawPlane(); // apply transforms to the plane object
void drawLamp(); // apply transforms to the lamp object
void drawBrick(); // apply transforms to the lamp object
void drawBaseball(); // apply transforms to the baseball object
void drawBook(); // apply transforms to the book object
void draw(GLFWwindow* window, double currentTime); // draw the objects
void initPositions(); // initialize object positions
static GLuint CompileShader(const string& source, GLuint shaderType);
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader);
void UProcessInput(GLFWwindow* window);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// global variables
int width = 640, height = 480; // window dimensions
GLuint triangleVBO, tipVBO, triangleVAO, tipVAO; // pencil VAO/VBO variables
GLuint planeVBO, planeVAO; // plane VAO/VBO variables
GLuint baseballVBO, baseballVAO; // plane VAO/VBO variables
GLuint lampVBO, lampEBO, lampVAO; // lighting variables for lamp object
GLuint bookVBO, bookVAO; // lighting variables for book object
GLuint brickVBO, brickVAO; // lighting variables for brick object
float triLocX, triLocY, triLocZ; // triangle location (x,y,z) for the pen tube; 
float tipLocX, tipLocY, tipLocZ; // location (x,y,z) for the tip of the pen;
float bookLocX, bookLocY, bookLocZ; // location (x,y,z) for the book;
float brickLocX, brickLocY, brickLocZ; // location (x,y,z) for the brick;
float planeLocX, planeLocY, planeLocZ; // location (x,y,z) for the plane;
float lampLocX, lampLocY, lampLocZ; // location (x,y,z) for the lamp object;
float baseballLocX, baseballLocY, baseballLocZ; // location (x,y,z) for the baseball object;
GLuint mvLoc, projLoc; // mvp uniform reference variables
GLuint lampMvLoc, lampProjLoc; // mvp uniform reference variables for lamp
GLuint objectColorLoc, lightColorLoc, lightPosLoc, viewPosLoc; // lighting object color variables
float aspectRatio; // for p matrix aspect ratio
glm::mat4 pMat, vMat, mMat, mvMat; // mvp variables
GLuint shaderProgram; // for compiled shaders
GLuint lampShaderProgram;

// Camera variables
Camera gCamera(glm::vec3(-2.0f, 8.0f, 20.0f));
float gLastX = width / 2.0f;
float gLastY = height / 2.0f;
bool gFirstMouse = true;

// timing
float gDeltaTime = 0.0f; // time between current frame and last frame
float gLastFrame = 0.0f;

// variables to convert radians to degrees
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

// TEXTURES
// plane texture
int planeWidth, planeHeight;
unsigned char* planeImage = SOIL_load_image("woolpng.png", &planeWidth, &planeHeight, 0, SOIL_LOAD_RGB);
GLuint planeTexture;

// pencil tube texture
int pencilTubeWidth, pencilTubeHeight;
unsigned char* pencilTubeImage = SOIL_load_image("pearl.png", &pencilTubeWidth, &pencilTubeHeight, 0, SOIL_LOAD_RGB);
GLuint pencilTubeTexture;

// pencil tip texture
int pencilTipWidth, pencilTipHeight;
unsigned char* pencilTipImage = SOIL_load_image("darkmetal2png.png", &pencilTipWidth, &pencilTipHeight, 0, SOIL_LOAD_RGB);
GLuint pencilTipTexture;

// baseball texture
int baseballWidth, baseballHeight;
unsigned char* baseballImage = SOIL_load_image("baseballtwo.png", &baseballWidth, &baseballHeight, 0, SOIL_LOAD_RGB);
GLuint baseballTexture;

// book texture
int bookWidth, bookHeight;
unsigned char* bookImage = SOIL_load_image("leather.png", &bookWidth, &bookHeight, 0, SOIL_LOAD_RGB);
GLuint bookTexture;

// brick texture
int brickWidth, brickHeight;
unsigned char* brickImage = SOIL_load_image("brickred.png", &brickWidth, &brickHeight, 0, SOIL_LOAD_RGB);
GLuint brickTexture;

// light source position
glm::vec3 lightPosition(0.0f, -10.0f, -10.0f);

// vertex shader source code
string vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 vPosition;"
"layout(location = 1) in vec3 aColor;"
"layout(location = 2) in vec2 texCoord;"
"layout(location = 3) in vec3 normal;"
"out vec3 v_color;"
"out vec2 oTexCoord;"
"out vec3 oNormal;"
"out vec3 FragPos;"
"uniform mat4 mv_matrix;"
"uniform mat4 proj_matrix;"
"void main()\n"
"{\n"
"gl_Position = proj_matrix * mv_matrix * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
"v_color = aColor;"
"oTexCoord = texCoord;"
"oNormal = mat3(transpose(inverse(mv_matrix))) * normal;"
"FragPos = vec3(mv_matrix * vec4(vPosition, 1.0f));"
"}\n";

// fragment shader source code
string fragmentShaderSource =
"#version 330 core\n"
"in vec3 v_color;"
"in vec2 oTexCoord;"
"in vec3 oNormal;"
"in vec3 FragPos;"
"out vec4 fragColor;"
"uniform sampler2D myTexture;"
"uniform vec3 objectColor;"
"uniform vec3 lightColor;"
"uniform vec3 lightPos;"
"uniform vec3 viewPos;"
"uniform mat4 mv_matrix;"
"uniform mat4 proj_matrix;"
"void main()\n"
"{\n"
"//Ambient\n"
"float ambientStrength = 0.5f;"
"vec3 ambient = ambientStrength * lightColor;"
"//Diffuse\n"
"vec3 norm = normalize(oNormal);"
"vec3 lightDir = normalize(lightPos - FragPos);"
"float diff = max(dot(norm, lightDir), 0.0);"
"vec3 diffuse = diff * lightColor;"
"//Specularity\n"
"float specularStrength = 0.3;"
"vec3 viewDir = normalize(viewPos - FragPos);"
"vec3 reflectDir = reflect(-lightDir, norm);"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);"
"vec3 specular = specularStrength * spec * lightColor;"
"vec3 result = (ambient + diffuse + specular) * objectColor;"
"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"
"}\n";

// lamp vertex shader source code
string lampVertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 vPosition;"
"uniform mat4 mv_matrix;"
"uniform mat4 proj_matrix;"
"void main()\n"
"{\n"
"gl_Position = proj_matrix * mv_matrix * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
"}\n";

// lamp fragment shader source code
string lampFragmentShaderSource =
"#version 330 core\n"
"out vec4 fragColor;"
"void main()\n"
"{\n"
"fragColor = vec4(1.0f);"
"}\n";

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSwapInterval(1); // VSync operation

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, UResizeWindow);
	glfwSetCursorPosCallback(window, UMousePositionCallback);
	glfwSetScrollCallback(window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	initPositions();// Set up view (camera) and triangle initial positions
	CreateVertices(); // Generate vertices, create VAO, create buffers, associate VAO with VBO, load vertices to VBOs, associate VBOs with VAs (Vertex Attributes)
	// initialize the perspective matrix
	pMat = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	// Create shader (program object)
	shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	// create lamp shader 
	lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);

	// generate plane textures
	glGenTextures(1, &planeTexture);
	glBindTexture(GL_TEXTURE_2D, planeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, planeWidth, planeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, planeImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(planeImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// generate pencil tube textures
	glGenTextures(1, &pencilTubeTexture);
	glBindTexture(GL_TEXTURE_2D, pencilTubeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pencilTubeWidth, pencilTubeHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pencilTubeImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(pencilTubeImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// generate pencil tip textures
	glGenTextures(1, &pencilTipTexture);
	glBindTexture(GL_TEXTURE_2D, pencilTipTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pencilTipWidth, pencilTipHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pencilTipImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(pencilTipImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// generate baseball textures
	glGenTextures(1, &baseballTexture);
	glBindTexture(GL_TEXTURE_2D, baseballTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, baseballWidth, baseballHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, baseballImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(baseballImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// generate baseball textures
	glGenTextures(1, &bookTexture);
	glBindTexture(GL_TEXTURE_2D, bookTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bookWidth, bookHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bookImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(bookImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	// generate baseball textures
	glGenTextures(1, &brickTexture);
	glBindTexture(GL_TEXTURE_2D, brickTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, brickWidth, brickHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, brickImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(brickImage);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// per-frame timing	
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// process input
		UProcessInput(window);

		// Draw primitives
		draw(window, glfwGetTime());

		/* Swap front and back buffers */
		glfwSwapBuffers(window); // VSync operation

		/* Poll for and process events */
		glfwPollEvents(); // Detect keyboard and mouse input
	}

	glfwTerminate();
	return 0;
}


//============================================================================
//============================================================================
//============================================================================

void CreateVertices()
{
	GLfloat brickVertices[]{
		0.0f, 1.0f, 0.5f, //vertice 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, -0.5f, // vertice 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.5f, // vertice 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, -0.5f, // vertice 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.5f, // vertice 5
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.5f, // vertice 6
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, -0.5f, // vertice 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.5f, // vertice 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.5f, //vertice 1
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.5f, //vertice 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.5f, // vertice 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.5f, // vertice 3
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.5f, //vertice 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.5f, // vertice 3
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.5f, // vertice 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, -0.5f, // vertice 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.5f, // vertice 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.5f, // vertice 6
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,
	};

	// Defeine vertex data for book
	GLfloat bookVertices[] = {
		-1.0f, 0.0f, -0.375f, // vertex 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.375f, // vertex 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.375f, // vertex 3
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.375f, // vertex 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.375f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.375f, // vertex 3
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.375f, // vertex 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.375f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, 0.375f, // vertex 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.375f, // vertex 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, 0.375f, // vertex 5
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, -0.375f, // vertex 6
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.375f, // vertex 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.375f, // vertex 3
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, -0.1f, 0.375f, // vertex 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.375f, // vertex 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, -0.1f, 0.375f, // vertex 7
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, -0.1f, -0.375f, // vertex 8
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.375f, // vertex 3
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.375f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, 0.375f, // vertex 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.375f, // vertex 3
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, 0.375f, // vertex 5
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, -0.1f, 0.375f, // vertex 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.375f, // vertex 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, -0.375f, // vertex 6
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, -0.1f, -0.375f, // vertex 8
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, -0.375f, // vertex 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, -0.375f, // vertex 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		1.0f, -0.1f, -0.375f, // vertex 6
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,
	};

	// Define vertex data for pencil tube 
	GLfloat baseballVertices[] = {
		0.0f, 0.0f, 1.0f,  // vert 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, 0.86603f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, 0.86603f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,
		
		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 8
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 10
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, -0.86603f, // vert 11
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 10
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, -0.86603f, // vert 11
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, -1.0f, // vert 13
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,


	};

	// Define vertex data for pencil tube 
	GLfloat planeVertices[] = {
		// positon attributes (x,y,z)
		-5.0f, -1.0f, -5.0f,  // vert 1
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0, // UV lower left
		0.0f, 0.0f, 1.0f, // normal positive z

		5.0f, -1.0f, -5.0f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0, // UV lower right
		0.0f, 0.0f, 1.0f, // normal positive z

		5.0f, -1.0f, 5.0f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		1.0, 1.0, // UV upper right
		0.0f, 0.0f, 1.0f, // normal positive z

		5.0f, -1.0f, 5.0f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		1.0, 1.0, // UV upper right
		0.0f, 0.0f, 1.0f, // normal positive z

		-5.0f, -1.0f, 5.0f, // vert 4
		1.0f, 1.0f, 1.0f, // white	
		0.0, 1.0, // UV upper left
		0.0f, 0.0f, 1.0f, // normal positive z

		-5.0f, -1.0f, -5.0f, // vert 1
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0, // UV lower left
		0.0f, 0.0f, 1.0f // normal positive z

	};

	// Define vertex data for pencil tube 
	GLfloat triangleVertices[] = {
		// positon attributes (x,y,z)
		0.0f, .0f, 0.0f,  // vert 1
		1.0f, 0.0f, 0.0f, // red
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.25f, 0.433f, 0.0f, // vert 2
		0.0f, 1.0f, 0.0f, // green
		0.0, 0.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.5f, 0.0f, 0.0f, // vert 3
		0.0f, 0.0f, 1.0f, // blue
		1.0, 0.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.25f, 0.433f, 0.0f, // vert 4
		0.0f, 1.0f, 0.0f, // green
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.25f, 0.433f, -1.0f, // vert 5
		0.0f, 1.0f, 0.0f, // green	
		0.0, 0.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.5f, 0.0f, 0.0f, // vert 6
		0.0f, 0.0f, 1.0f, // blue
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.5f, 0.0f, 0.0f, // vert 7
		0.0f, 0.0f, 1.0f, // blue
		0.0, 1.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.5f, 0.0f, -1.0f, // vert 8
		0.0f, 0.0f, 1.0f, // blue
		0.0, 0.0,
		0.0f, 0.0f, 1.0f, // normal positive z

		0.25f, 0.433f, -1.0f, // vert 9
		0.0f, 1.0f, 0.0f, // green
		1.0, 0.0,
		0.0f, 0.0f, 1.0f // normal positive z
	};

	// Define vertex data for tip of pencil 
	GLfloat tipVertices[] = {

		0.0f, 0.0f, 1.0f,  // vert 1
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, 0.86603f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, 0.86603f, // vert 3
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, 0.86603f, // vert 2
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, 0.5f, // vert 5
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, 0.5f, // vert 4
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 8
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -1.0f, 0.0f, // vert 7
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.86603f, -0.5f, 0.0f, // vert 6
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 10
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, -0.86603f, // vert 11
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.86603f, -0.5f, // vert 9
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		0.75f, -0.43301f, -0.5f, // vert 10
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.43301f, -0.25f, -0.86603f, // vert 11
		1.0f, 1.0f, 1.0f, // white
		0.5, 1.0,
		0.0f, 0.0f, 1.0f,

		-0.0f, -0.5f, -0.86603f, // vert 12
		1.0f, 1.0f, 1.0f, // white
		1.0, 0.0,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, -1.0f, // vert 13
		1.0f, 1.0f, 1.0f, // white
		0.0, 0.0,
		0.0f, 0.0f, 1.0f,

	};

	// Define vertex data for lamp light source
	GLfloat lampVertices[] = {

		// positon attributes (x,y,z)
		// base of pyramid
		-0.25f, -0.25f, -0.25f,  // vert 4 - back left vertice

		-0.25f, -0.25f, 0.25f,  // vert 3 - front left vertice

		0.25f, -0.25f, -0.25f,  // vert 1 - back right vertice

		0.25f, -0.25f, -0.25f,  // vert 1 - back right vertice

		0.25f, -0.25f, 0.25f,  // vert 2 - front right vertice

		-0.25f, -0.25f, 0.25f,  // vert 3 - front left vertice 

		// pyramid sides
		0.0f, 0.25f, 0.0f,  // vert 0 - point of pyramid

		0.25f, -0.25f, -0.25f,  // vert 1 - back right vertice 

		0.25f, -0.25f, 0.25f,  // vert 2 - front right vertice 

		0.0f, 0.25f, 0.0f,  // vert 0 - point of pyramid

		0.25f, -0.25f, 0.25f,  // vert 2 - front right vertice 

		-0.25f, -0.25f, 0.25f,  // vert 3 - front left vertice 

		0.0f, 0.25f, 0.0f,  // vert 0 - point of pyramid

		-0.25f, -0.25f, 0.25f,  // vert 3 - front left vertice 

		-0.25f, -0.25f, -0.25f,  // vert 4 - back left vertice 

		-0.25f, -0.25f, -0.25f,  // vert 4 - back left vertice 

		0.0f, 0.25f, 0.0f,  // vert 0 - point of pyramid

		0.25f, -0.25f, -0.25f,  // vert 1 - back right vertice 
	};

	// plane
	glGenVertexArrays(1, &planeVAO); // Create VAO
	glGenBuffers(1, &planeVBO); // Create VBO
	glBindVertexArray(planeVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA - color
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)	

	// pencil tube
	glGenVertexArrays(1, &triangleVAO); // Create VAO
	glGenBuffers(1, &triangleVBO); // Create VBO
	glBindVertexArray(triangleVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VAO (Optional but recommended)	

	// pencil tip
	glGenVertexArrays(1, &tipVAO); // Create VAO
	glGenBuffers(1, &tipVBO); // Create VBO
	glBindVertexArray(tipVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, tipVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(tipVertices), tipVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)	

	// baseball
	glGenVertexArrays(1, &baseballVAO); // Create VAO
	glGenBuffers(1, &baseballVBO); // Create VBO
	glBindVertexArray(baseballVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, baseballVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(baseballVertices), baseballVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)

	// baseball
	glGenVertexArrays(1, &bookVAO); // Create VAO
	glGenBuffers(1, &bookVBO); // Create VBO
	glBindVertexArray(bookVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, bookVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(bookVertices), bookVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)	

	// brick
	glGenVertexArrays(1, &brickVAO); // Create VAO
	glGenBuffers(1, &brickVBO); // Create VBO
	glBindVertexArray(brickVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, brickVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(brickVertices), brickVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // Associate VBO with VA
	glEnableVertexAttribArray(1); // Enable VA
	// texture
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // Associate VBO with VA - texture
	glEnableVertexAttribArray(2); // Enable texture VA
	// normals
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)	

	// lamp
	glGenVertexArrays(1, &lampVAO); // Create VAO
	glGenBuffers(1, &lampVBO); // Create VBO
	glGenBuffers(1, &lampEBO); // Create EBO
	glBindVertexArray(lampVAO); // Activate VAO for VBO association
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Enable VBO	
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Copy Vertex data to VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0); // Associate VBO with VA (Vertex Attribute)
	glEnableVertexAttribArray(0); // Enable VA
	glBindVertexArray(0); // Unbind VAO (Optional but recommended)

}

// function to draw the scene objects - plane, pencil,
void draw(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT); // Z-buffer operation (hsr removal)
	glClear(GL_COLOR_BUFFER_BIT); // remove animation trails

	glUseProgram(shaderProgram); // load shaders to GPU

	// Reference matrix uniform variables in shader
	mvLoc = glGetUniformLocation(shaderProgram, "mv_matrix");
	projLoc = glGetUniformLocation(shaderProgram, "proj_matrix");

	// get lighting object color location
	objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
	lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
	lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

	// assign light and object colors sampled from texture (get RGB value from texture of object) 
	glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

	// Set light position
	glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);

	// Set light position
	glUniform3f(viewPosLoc, gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

	// Build Perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspectRatio = (float)width / (float)height;

	//pMat = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);


	//Build View matrix
	vMat = gCamera.GetViewMatrix();

	glBindTexture(GL_TEXTURE_2D, planeTexture);
	drawPlane();
	glBindTexture(GL_TEXTURE_2D, pencilTubeTexture);
	drawPencil();
	glBindTexture(GL_TEXTURE_2D, pencilTipTexture);
	drawTip();
	glBindTexture(GL_TEXTURE_2D, baseballTexture);
	drawBaseball();
	glBindTexture(GL_TEXTURE_2D, bookTexture);
	drawBook();
	glBindTexture(GL_TEXTURE_2D, brickTexture);
	drawBrick();
	glUseProgram(0);

	// switch shader for lamp object
	glUseProgram(lampShaderProgram);
	// Reference matrix uniform variables in shader for lamp
	lampMvLoc = glGetUniformLocation(lampShaderProgram, "mv_matrix");
	lampProjLoc = glGetUniformLocation(lampShaderProgram, "proj_matrix");

	drawLamp();

}

void drawLamp() {
	glBindVertexArray(lampVAO);

	// Apply Transform to model // Build model matrix for tri
	glm::mat4 scale = glm::scale(glm::vec3(0.85f, 2.0f, 0.85f));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(lampLocX, lampLocY, lampLocZ)); // Position lamp object
	//mMat = glm::rotate(mMat, 90 * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	mvMat = vMat * mMat * scale; // view and model matrix multiplied here

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(lampMvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 18); // Render primitive or execute shader per draw

	glBindVertexArray(0);
}

// function to draw the tip of the pencil
void drawPlane() {

	glBindVertexArray(planeVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()

	// Apply Transform to model // Build model matrix for tri
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(planeLocX, planeLocY, planeLocZ)); // Position pencil tip at end of the pencil tube
	mMat = glm::rotate(mMat, 0 * toRadians, glm::vec3(1.0f, 0.3f, 0.5f));
	mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 9); // Render primitive or execute shader per draw

	glBindVertexArray(0);// Optional unbinding but recommended

}

// function to draw the book object
void drawBook() {

	glBindVertexArray(bookVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()

	// Apply Transform to model // Build model matrix for tri
	glm::mat4 scale = glm::scale(glm::vec3(8.0f, 8.0f, 12.0f));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(bookLocX, bookLocY, bookLocZ)); // Position book
	mMat = glm::rotate(mMat, 0 * toRadians, glm::vec3(1.0f, 0.3f, 0.5f));
	mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 36); // Render primitive or execute shader per draw

	glBindVertexArray(0);// Optional unbinding but recommended

}


// function to draw the brick object
void drawBrick() {

	glBindVertexArray(brickVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()

	// Apply Transform to model // Build model matrix for tri
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(brickLocX, brickLocY, brickLocZ)); // Position brick
	mMat = glm::rotate(mMat, 0 * toRadians, glm::vec3(1.0f, 0.3f, 0.5f));
	mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

	//Copy perspective and MV matrices to uniforms
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glEnable(GL_DEPTH_TEST); // Z-buffer operation
	glDepthFunc(GL_LEQUAL); // Used with Depth test
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
	glDrawArrays(GL_TRIANGLES, 0, 36); // Render primitive or execute shader per draw

	glBindVertexArray(0);// Optional unbinding but recommended

}

// function to draw the tip of the pencil
void drawTip() {

	// roatation using 30 degree angles
	glm::float32 triRotations2[] = { 0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };

	glBindVertexArray(tipVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()
	// Use loop to build Model matrix for pencil tube
	for (int i = 0; i < 6; i++) {
		// Apply Transform to model 
		glm::mat4 scale = glm::scale(glm::vec3(0.2f, 0.2f, 1.8f));
		//scale = glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(tipLocX, tipLocY, tipLocZ)); // Position pencil tube
		mMat = glm::rotate(mMat, -90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, 15 * toRadians, glm::vec3(0.0f, 0.1f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, glm::radians(triRotations2[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array		
		mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

		//Copy perspective and MV matrices to uniforms
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

		glEnable(GL_DEPTH_TEST); // Z-buffer operation
		glDepthFunc(GL_LEQUAL); // Used with Depth test
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
		glDrawArrays(GL_TRIANGLES, 0, 9); // Render primitive or execute shader per draw

	}
	// Optional unbinding but recommended
	glBindVertexArray(0);

}

// function to draw the baseball
void drawBaseball() {

	// roatation using 30 degree angles
	glm::float32 triRotations2[] = { 0.0f, 60.0f, 120.0f, 180.0f, 240.0f, 300.0f };

	glBindVertexArray(baseballVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()
	// Use loop to build Model matrix for pencil tube
	for (int i = 0; i < 6; i++) {
		// Apply Transform to model 
		glm::mat4 scale = glm::scale(glm::vec3(1.5f, 1.5f, 1.5f));
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(baseballLocX, baseballLocY, baseballLocZ)); // Position pencil tube
		mMat = glm::rotate(mMat, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, 15 * toRadians, glm::vec3(0.0f, 0.1f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, glm::radians(triRotations2[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array		
		mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

		//Copy perspective and MV matrices to uniforms
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

		glEnable(GL_DEPTH_TEST); // Z-buffer operation
		glDepthFunc(GL_LEQUAL); // Used with Depth test
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
		glDrawArrays(GL_TRIANGLES, 0, 30); // Render primitive or execute shader per draw

	}
	// Optional unbinding but recommended
	glBindVertexArray(0);

}

// function to draw the tube of the pencil
void drawPencil() {

	// roatation using 30 degree angles
	glm::float32 triRotations2[] = { 0.0f, 30.f, 60.0f, 90.0f, 120.0f, 150.0f, 180.0f,
	210.0f, 240.0f, 270.f, 300.0f, 330.0f };

	glBindVertexArray(triangleVAO); // Activate VAO (now references VBO and VA association) Can be placed anywhere before glDrawArrays()
	// Use loop to build Model matrix for pencil tube
	for (int i = 0; i < 12; i++) {
		// Apply Transform to model 
		glm::mat4 scale = glm::scale(glm::vec3(0.4f, 0.4f, 3.6f));
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(triLocX, triLocY, triLocZ)); // Position pencil tube
		mMat = glm::rotate(mMat, 90 * toRadians, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, 15 * toRadians, glm::vec3(0.0f, 0.1f, 0.0f)); // Rotate pencil tube to orient it in the right direction
		mMat = glm::rotate(mMat, glm::radians(triRotations2[i]), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate strip on z by increments in array		
		mvMat = vMat * mMat * scale; // view and model matrix multiplied here (instead of in shader for better performance)

		//Copy perspective and MV matrices to uniforms
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

		glEnable(GL_DEPTH_TEST); // Z-buffer operation
		glDepthFunc(GL_LEQUAL); // Used with Depth test
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
		glDrawArrays(GL_TRIANGLES, 0, 9); // Render primitive or execute shader per draw

	}
	// Optional unbinding but recommended
	glBindVertexArray(0);
}


// set up object locations
void initPositions()
{
	// location variables for the plane
	planeLocX = 0.0f;
	planeLocY = 0.0f;
	planeLocZ = 0.0f;

	// location variables for pencil tube
	triLocX = 6.0f;
	triLocY = -1.0f;
	triLocZ = 1.04f;

	// variables for tip of pencil
	tipLocX = 3.45f;
	tipLocY = -0.99f;
	tipLocZ = 1.710f;

	// variables for lamp
	lampLocX = 0.0f;
	lampLocY = 10.0f;
	lampLocZ = 12.0f;

	// variables for baseball position
	baseballLocX = -5.0f;
	baseballLocY = 0.0f;
	baseballLocZ = 0.0f;

	// variables for book position
	bookLocX = 0.0f;
	bookLocY = -1.24f;
	bookLocZ = 0.0f;

	brickLocX = 0.0f;
	brickLocY = -1.25f;
	brickLocZ = 0.0f;
}


//============================================================================
/*================= GLSL Error Checking Definitions ===================*/
//============================================================================
void PrintShaderCompileError(GLuint shader)
{
	int len = 0;
	int chWritten = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWritten, log);
		cout << "Shader Compile Error: " << log << endl;
		free(log);
	}
}

void PrintShaderLinkingError(int prog)
{
	int len = 0;
	int chWritten = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0)
	{
		log = (char*)malloc(len);
		glGetShaderInfoLog(prog, len, &chWritten, log);
		cout << "Shader Linking Error: " << log << endl;
		free(log);
	}
}

bool IsOpenGLError()
{
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}


//============================================================================
/*================= Shader Functions ===================*/
//============================================================================


// compile Shaders 
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// create vertexShader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// attach source code
	glShaderSource(shaderID, 1, &src, nullptr);

	// compile shader
	glCompileShader(shaderID);


	// shader Compliation Error Check 
	GLint shaderCompiled;
	IsOpenGLError();
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != 1)
	{
		cout << "Shader Compilation Failed!" << endl;
		PrintShaderCompileError(shaderID);
	}

	// return ID
	return shaderID;
}

// create Shader Program
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// compile Vertex Shader
	GLint vertShaderCompiled = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// compile Fragment Shader
	GLint fragShaderCompiled = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// create program object
	GLuint shaderProgram = glCreateProgram();

	// attach shaders
	glAttachShader(shaderProgram, vertShaderCompiled);
	glAttachShader(shaderProgram, fragShaderCompiled);

	// link shaders to create full shader program
	glLinkProgram(shaderProgram);

	// shader Linking Error Check
	GLint linked;
	IsOpenGLError();
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
	if (linked != 1)
	{
		cout << "Shader Linking Failed!" << endl;
		PrintShaderLinkingError(shaderProgram);
	}

	glValidateProgram(shaderProgram);

	// delete intermediates
	glDeleteShader(vertShaderCompiled);
	glDeleteShader(fragShaderCompiled);

	return shaderProgram;
}


//============================================================================
/*================= Movement Functions ===================*/
//============================================================================

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	// process closing window using escape key
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// process switching from perspective mode to ortho mode using P and O
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		pMat = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
		pMat = glm::ortho(-glm::radians(gCamera.Zoom), 5.0f, -glm::radians(gCamera.Zoom), 5.0f, 0.1f, 100.0f);
	}
	// process movement with keys WASDQE
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}

// glfw: whenever the mouse moves, this callback is called
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	// reversed since y-coordinates go from bottom to top
	float yoffset = gLastY - ypos;

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}

