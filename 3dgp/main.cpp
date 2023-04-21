#include <iostream>
#include <GL/glew.h>
#include <3dgl/3dgl.h>
#include <GL/glut.h>
#include <GL/freeglut_ext.h>

// Include GLM core features
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;
using namespace glm;

// 3D models
C3dglModel camera;
C3dglModel table;
C3dglModel vase;
C3dglModel teapot;
C3dglModel cat;
C3dglModel lamp;

// Textures 
GLuint idTexWood;
GLuint idFabric;
GLuint idTexNone;

// GLSL Program
C3dglProgram program;

// The View Matrix
mat4 matrixView;

// Camera & navigation
float maxspeed = 4.f;	// camera max speed
float accel = 4.f;		// camera acceleration
vec3 _acc(0), _vel(0);	// camera acceleration and velocity vectors
float _fov = 60.f;		// field of view (zoom)

// vertex array 
float vertices[] = {
	-4, 0, -4, 4, 0, -4, 0, 7, 0, -4, 0, 4, 4, 0, 4, 0, 7, 0,
	-4, 0, -4, -4, 0, 4, 0, 7, 0, 4, 0, -4, 4, 0, 4, 0, 7, 0,
	-4, 0, -4, -4, 0, 4, 4, 0, -4, 4, 0, 4 
};
float normals[] = {
	0, 4, -7, 0, 4, -7, 0, 4, -7, 0, 4, 7, 0, 4, 7, 0, 4, 7,
	-7, 4, 0, -7, 4, 0, -7, 4, 0, 7, 4, 0, 7, 4, 0, 7, 4, 0,
	0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0
};
unsigned indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13, 14, 15 };

// buffers names
unsigned vertexBuffer = 0;
unsigned normalBuffer = 0;
unsigned indexBuffer = 0;

int Lpoint1 = 1, Lpoint2 = 1;

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!

	// Initialise Shaders
	C3dglShader vertexShader;
	C3dglShader fragmentShader;

	if (!vertexShader.create(GL_VERTEX_SHADER)) return false;
	if (!vertexShader.loadFromFile("shaders/basic.vert")) return false;
	if (!vertexShader.compile()) return false;

	if (!fragmentShader.create(GL_FRAGMENT_SHADER)) return false;
	if (!fragmentShader.loadFromFile("shaders/basic.frag")) return false;
	if (!fragmentShader.compile()) return false;

	if (!program.create()) return false;
	if (!program.attach(vertexShader)) return false;
	if (!program.attach(fragmentShader)) return false;
	if (!program.link()) return false;
	if (!program.use(true)) return false;
	
	// prepare vertex data
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// prepare normal data
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	// prepare indices array
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// glut additional setup
	glutSetVertexAttribCoord3(program.getAttribLocation("aVertex"));
	glutSetVertexAttribNormal(program.getAttribLocation("aNormal"));

	// load your 3D models here!
	if (!camera.load("models\\camera.3ds")) return false;
	if (!table.load("models\\table.obj")) return false;
	if (!vase.load("models\\vase.obj")) return false;
	if (!teapot.load("models\\teapot.obj")) return false;
	if (!cat.load("models\\cat.obj")) return false;
	if (!lamp.load("models\\lamp.obj")) return false;

	//Setup Lights
	//Switches Light On
	program.sendUniform("lightAmbient.on", 1);
	////Setup Light Colour
	program.sendUniform("lightAmbient.color", vec3(0.02, 0.02, 0.02));
	//Setup The Emissive light
	program.sendUniform("lightEmissive.on", 0);
	program.sendUniform("lightEmissive.color", vec3(1.0, 1.0, 1.0));

	//Turn the Directional Light On
	program.sendUniform("lightDir.on", 1);
	program.sendUniform("lightDir.direction", vec3(1.0, 0.5, 1.0));
	program.sendUniform("lightDir.diffuse", vec3(0.4, 0.4, 0.4));

	//Point Light Diffuse + Specular Extension
	program.sendUniform("lightPoint1.on", Lpoint1);
	program.sendUniform("lightPoint1.position", vec3(17.0f, 19.3f, 4.0f));
	program.sendUniform("lightPoint1.diffuse", vec3(0.5, 0.5, 0.5));
	program.sendUniform("lightPoint1.specular", vec3(.5, .5, .5));

	program.sendUniform("lightPoint2.on", Lpoint2);
	program.sendUniform("lightPoint2.position", vec3(1.4f, 19.3f, -4.f));
	program.sendUniform("lightPoint2.diffuse", vec3(0.5, 0.5, 0.5));
	program.sendUniform("lightPoint2.specular", vec3(.5, .5, .5));

	program.sendUniform("materialSpecular", vec3(0.6, 0.6, 1.0));
	program.sendUniform("shininess", 3.0);

	// Textures
	C3dglBitmap bm;
	glActiveTexture(GL_TEXTURE0);

	//Oak texture
	bm.load("models/oak.bmp", GL_RGBA);
	if (!bm.getBits()) return false;
	glGenTextures(1, &idTexWood);
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());


	// Fabric texture
	bm.load("models/fabric.png", GL_RGBA);
	if (!bm.getBits()) return false;
	glGenTextures(1, &idFabric);
	glBindTexture(GL_TEXTURE_2D, idFabric);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits());
	program.sendUniform("texture0", 0);

	// Null texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// Initialise the View Matrix (initial position of the camera)
	matrixView = rotate(mat4(1), radians(12.f), vec3(1, 0, 0));
	matrixView *= lookAt(
		vec3(0.0, 18.0, 35.0),
		vec3(15.0, 18.0, 0.0),
		vec3(0.0, 1.0, 0.0));

	// setup the screen background colour
	glClearColor(0.18f, 0.25f, 0.22f, 1.0f);   // deep grey background

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Shift to speed up your movement" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	return true;
}

void renderScene(mat4& matrixView, float time, float deltaTime)
{
	mat4 m;

	// setup materials - brown
	program.sendUniform("material", vec3(0.784f, 0.545f, 0.317f));

	// table
	m = matrixView;
	m = translate(m, vec3(15.0f, 0, 0.0f));
	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.020f, 0.020f, 0.020f));
	program.sendUniform("materialDiffuse", vec3(0.784f, 0.545f, 0.317f));
	program.sendUniform("materialAmbient", vec3(0.784f, 0.545f, 0.317f));
	glBindTexture(GL_TEXTURE_2D, idTexWood);
	table.render(1, m);

	// setup materials - grey
	program.sendUniform("material", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialDiffuse", vec3(0.6f, 0.6f, 0.6f));
	program.sendUniform("materialAmbient", vec3(0.6f, 0.6f, 0.6f));
	
	m = rotate(m, radians(90.f), vec3(0.0f, 1.0f, 0.0f));
	glBindTexture(GL_TEXTURE_2D, idFabric);
	table.render(0, m);

	m = rotate(m, radians(180.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);
	
	m = rotate(m, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);

	m = rotate(m, radians(-180.f), vec3(0.0f, 1.0f, 0.0f));
	table.render(0, m);

	// setup materials - blue
	program.sendUniform("material", vec3(0.2f, 0.2f, 0.8f));
	glBindTexture(GL_TEXTURE_2D, idTexNone);

	// vase
	m = matrixView;
	m = translate(m, vec3(15.0f, 15.2f, .0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(0.4f, 0.4f, 0.4f));
	program.sendUniform("materialDiffuse", vec3(0.2f, 0.2f, 0.8f));
	program.sendUniform("materialAmbient", vec3(0.2f, 0.2f, 0.8f));
	vase.render(m);
	
	// setup materials - green
	program.sendUniform("material", vec3(0.164f, 0.898f, 0.121f));

	//// teapot
	m = matrixView;
	m = translate(m, vec3(21.0f, 15.f, .0f));
	m = rotate(m, radians(-90.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(2.f, 2.f, 2.f));
	program.sendUniform("materialDiffuse", vec3(0.164f, 0.898f, 0.121f));
	program.sendUniform("materialAmbient", vec3(0.164f, 0.898f, 0.121f));
	teapot.render(m);

	// setup materials - red
	program.sendUniform("material", vec3(0.89f, 0.05f, 0.07f));

	// Get Attribute Locations
	GLuint attribVertex = program.getAttribLocation("aVertex");
	GLuint attribNormal = program.getAttribLocation("aNormal");

	// Enable vertex attribute arrays
	glEnableVertexAttribArray(attribVertex);
	glEnableVertexAttribArray(attribNormal);
	// Bind (activate) the vertex buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Bind (activate) the normal buffer and set the pointer to it
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Draw triangles – using index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	// Transforming the Pyramid
	program.sendUniform("materialDiffuse", vec3(0.89f, 0.05f, 0.07f));
	program.sendUniform("materialAmbient", vec3(0.89f, 0.05f, 0.07f));
	m = translate(m, vec3(-3.f, 1.85f, 5.f));
	m = scale(m, vec3(0.25f, 0.25f, 0.25f));
	m = rotate(m, radians(180.f), vec3(0.f, 0.f, 1.f));
	// Y axis pyramid rotation
	m = rotate(m, radians(time * 150), vec3(0.f, 1.f, 0.f));
	program.sendUniform("matrixModelView", m);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	m = matrixView;

	// setup materials - orange
	program.sendUniform("material", vec3(0.941f, 0.454f, 0.078f));

	// Transforming the cat
	program.sendUniform("materialDiffuse", vec3(0.941f, 0.454f, 0.078f));
	program.sendUniform("materialAmbient", vec3(0.941f, 0.454f, 0.078f));
	m = translate(m, vec3(10.f, 18.65f, -4.7f));
	m = scale(m, vec3(0.05f, 0.05f, 0.05f));
	m = rotate(m, radians(45.0f), vec3(0.f, 1.f, 0.f));
	// Y axis cat rotation 
	m = rotate(m, -radians(time * 15), vec3(0.f, 1.f, 0.f));
	cat.render(m);
	m = matrixView;

	// Disable arrays
	glDisableVertexAttribArray(attribVertex);
	glDisableVertexAttribArray(attribNormal);

	// setup materials - yellow
	program.sendUniform("material", vec3(0.9f, 0.8f, 0.4f));

	// lamp
	program.sendUniform("materialDiffuse", vec3(0.9f, 0.8f, 0.4f));
	program.sendUniform("materialAmbient", vec3(0.9f, 0.8f, 0.4f));
	m = matrixView;
	m = translate(m, vec3(4.0f, 15.2f, -4.0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(.07f, .07f, .07f));
	lamp.render(m);

	// lamp
	m = matrixView;
	m = translate(m, vec3(21.0f, 15.2f, 4.0f));
	m = rotate(m, radians(0.f), vec3(0.0f, 1.0f, 0.0f));
	m = scale(m, vec3(.07f, .07f, .07f));
	lamp.render(m);

	// Lightbulbs
	// The 1st lighbulb
	m = translate(m, vec3(-36.8f, 59.f, .0f));
	m = scale(m, vec3(5.2f, 5.2f, 5.2f));
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("lightEmissive.on", Lpoint1);
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	m = matrixView;
	
	// The 2nd lightbulb
	m = translate(m, vec3(1.4f, 19.3f, -4.f));
	m = scale(m, vec3(0.35f, 0.35f, 0.35f));
	program.sendUniform("materialDiffuse", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialAmbient", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("materialSpecular", vec3(1.0f, 1.0f, 1.0f));
	program.sendUniform("lightEmissive.on", Lpoint2);
	program.sendUniform("matrixModelView", m);
	glutSolidSphere(1, 32, 32);
	m = matrixView;

	program.sendUniform("lightEmissive.on", 0);
	m = matrixView;

}

void onRender()
{
	// these variables control time & animation
	static float prev = 0;
	float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;	// time since start in seconds
	float deltaTime = time - prev;						// time since last frame
	prev = time;										// framerate is 1/deltaTime

	// clear screen and buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	_vel = clamp(_vel + _acc * deltaTime, -vec3(maxspeed), vec3(maxspeed));
	float pitch = getPitch(matrixView);
	matrixView = rotate(translate(rotate(mat4(1),
		pitch, vec3(1, 0, 0)),	// switch the pitch off
		_vel * deltaTime),		// animate camera motion (controlled by WASD keys)
		-pitch, vec3(1, 0, 0))	// switch the pitch on
		* matrixView;

	// setup View Matrix
	program.sendUniform("matrixView", matrixView);

	// render the scene objects
	renderScene(matrixView, time, deltaTime);

	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void onReshape(int w, int h)
{
	float ratio = w * 1.0f / h;      // we hope that h is not zero
	glViewport(0, 0, w, h);
	mat4 matrixProjection = perspective(radians(_fov), ratio, 0.02f, 1000.f);

	// Setup the Projection Matrix
	program.sendUniform("matrixProjection", matrixProjection);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': _acc.z = accel; break;
	case 's': _acc.z = -accel; break;
	case 'a': _acc.x = accel; break;
	case 'd': _acc.x = -accel; break;
	case 'e': _acc.y = accel; break;
	case 'q': _acc.y = -accel; break;
	case '1': Lpoint1 = 1 - Lpoint1; program.sendUniform("lightPoint1.on", Lpoint1); break;
	case '2': Lpoint2 = 1 - Lpoint2; program.sendUniform("lightPoint2.on", Lpoint2); break;
	}
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': _acc.z = _vel.z = 0; break;
	case 'a':
	case 'd': _acc.x = _vel.x = 0; break;
	case 'q':
	case 'e': _acc.y = _vel.y = 0; break;
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	maxspeed = glutGetModifiers() & GLUT_ACTIVE_SHIFT ? 20.f : 4.f;
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	glutSetCursor(state == GLUT_DOWN ? GLUT_CURSOR_CROSSHAIR : GLUT_CURSOR_INHERIT);
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);
	if (button == 1)
	{
		_fov = 60.0f;
		onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}
}

// handle mouse move
void onMotion(int x, int y)
{
	glutWarpPointer(glutGet(GLUT_WINDOW_WIDTH) / 2, glutGet(GLUT_WINDOW_HEIGHT) / 2);

	// find delta (change to) pan & pitch
	float deltaYaw = 0.005f * (x - glutGet(GLUT_WINDOW_WIDTH) / 2);
	float deltaPitch = 0.005f * (y - glutGet(GLUT_WINDOW_HEIGHT) / 2);

	if (abs(deltaYaw) > 0.3f || abs(deltaPitch) > 0.3f)
		return;	// avoid warping side-effects

	// View = Pitch * DeltaPitch * DeltaYaw * Pitch^-1 * View;
	constexpr float maxPitch = radians(80.f);
	float pitch = getPitch(matrixView);
	float newPitch = glm::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
	matrixView = rotate(rotate(rotate(mat4(1.f),
		newPitch, vec3(1.f, 0.f, 0.f)),
		deltaYaw, vec3(0.f, 1.f, 0.f)), 
		-pitch, vec3(1.f, 0.f, 0.f)) 
		* matrixView;
}

void onMouseWheel(int button, int dir, int x, int y)
{
	_fov = glm::clamp(_fov - dir * 5.f, 5.0f, 175.f);
	onReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("3DGL Scene Level 1: Khrystyna Tkachuk");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		C3dglLogger::log("GLEW Error {}", (const char*)glewGetErrorString(err));
		return 0;
	}
	C3dglLogger::log("Using GLEW {}", (const char*)glewGetString(GLEW_VERSION));

	// register callbacks
	glutDisplayFunc(onRender);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutMouseWheelFunc(onMouseWheel);

	C3dglLogger::log("Vendor: {}", (const char *)glGetString(GL_VENDOR));
	C3dglLogger::log("Renderer: {}", (const char *)glGetString(GL_RENDERER));
	C3dglLogger::log("Version: {}", (const char*)glGetString(GL_VERSION));
	C3dglLogger::log("");

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		C3dglLogger::log("Application failed to initialise\r\n");
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}

