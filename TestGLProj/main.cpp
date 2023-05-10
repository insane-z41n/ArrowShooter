#include <GL/glew.h>
#include <GL/freeglut.h>

//glm library
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
#include "Model.h"
#include "Shader.h"
#include "QuatCamera.h"
#include "Particle.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include "stb_image.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <dos.h>
#include <windows.h>
#include <playsoundapi.h>
#include <mmsystem.h>
using namespace std;

Shader shader;
Model *mesh;

Model *sphere;
Model *target;
Model *plane;
Model *weapon;
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;
bool drawTorus = true;
bool spin = false;
float targetPosX = 1.0f;
float targetPosY = 0.0f;
float targetSpeed = 0.01f;
int level = 1;
bool isTargetMovingRight;
bool isTargetMovingUp;
bool enableParticles;
glm::vec4 lightPosition = glm::vec4(0.0f,3.0f,0.0f,1.0f);

auto previousTime = std::chrono::high_resolution_clock::now();

std::vector<Particle> particles;
int NUM_PARTICLES = 10000;
QuatCamera * camera;

float xMinTarget = 0.086843f;
float xMaxTarget = 1.951883f;
float yMinTarget = -0.805112f;
float yMaxTarget = 0.927725f;

float zMinTarget = -1.0f;
float zMaxTarget = -0.923879;

int scoreCounter = 0;

/* report GL errors, if any, to stderr */
void checkError(const char *functionName)
{
	GLenum error;
	while (( error = glGetError() ) != GL_NO_ERROR) {
	  std::cerr << "GL error " << error << " detected in " << functionName << std::endl;
	}
}
void SortParticles() {
	std::sort(&particles[0], &particles[NUM_PARTICLES]);
}

void initParticles()
{
	for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle p;
		p.position = glm::vec3(0.0f, 0.0f, 0.0f);
		p.velocity = glm::vec3(0.0, 9.8f, 0.0f);
		p.color = glm::vec4(1.0f);
		p.size = 10.0f;
		p.life = 1.0f;
		particles.push_back(p);

	}
}

void updateParticles(float dt)
{
	/*for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle& p = particles[i];
		p.life -= dt;
		p.position += p.velocity * dt;
	}*/

	for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle& p = particles[i];

		if (p.life > 0.0f) {
			p.life -= dt;
			if (p.life > 0.0f) {
				p.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * dt * 0.5f;
				p.position += p.velocity* dt;
				p.cameradistance = glm::length(p.position - camera->GetPos());
			}
			else {
				p.cameradistance = -1.0f;
			}
		}
	}


}

void resetParticles() {
	for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle& p = particles[i];
		p.life = 1.0f;
	}
}

void renderParticles() {
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set the point size
	glPointSize(10.0f);

	// Enable vertex array and specify 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Particle), (void*)offsetof(Particle, color));

	for (int i = 0; i < NUM_PARTICLES; i++) {
		Particle& p = particles[i];
		glColor4f(p.color.r, p.color.g, p.color.b, p.life);
		glVertex3f(p.position.x, p.position.y, p.position.z);
	}

	// Draw particles
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

	// Disable vertex arrays
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void initShader(void)
{
	shader.InitializeFromFile("shaders/phong3.vert", "shaders/phong3.frag");
	//shader.AddAttribute("vertexPosition");
	//shader.AddAttribute("vertexNormal");

	checkError ("initShader");
}

void initRendering(void)
{
	glClearColor (0.117f, 0.565f, 1.0f, 0.0f); // Dodger Blue
	checkError ("initRendering");
}

void shootray(GLfloat mousePositionOnClickx, GLfloat mousePositionOnClicky) {
	GLint viewport[] = { 0,0,0,0 };
	glGetIntegerv(GL_VIEWPORT, viewport);


	//Holds Our x, y, and z coordinates
	GLfloat winX, winY, winZ;

	winX = mousePositionOnClickx;
	winY = mousePositionOnClicky;

	/* Now Windows coordinates start with(0, 0) being at the top left whereas
		OpenGL coords start at the lower left.To convert to OpenGL coordinates we do the following : */
		winY = viewport[3] - winY;

	//grabbing the Z coordinate
	glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	glm::vec3 pointOnNear = glm::unProject(glm::vec3(mousePositionOnClickx, viewport[3] - mousePositionOnClicky, winZ), view, projection,
		glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]));

	glm::mat4 sphereTranslation = glm::mat4();



	printf("Point on surface...\n");
	printf("%f %f %f\n", pointOnNear.x, pointOnNear.y, pointOnNear.z);

	//IF conditions for target collision

	if (pointOnNear.x > xMinTarget && pointOnNear.x < xMaxTarget) {

		if (pointOnNear.y > yMinTarget && pointOnNear.y < yMaxTarget) {


			std::cout << "-------------TARGET HIT------------ \n";
			scoreCounter += 1;
			std::cout << "SCORE : " << scoreCounter << " \n";


			PlaySound("mixkit-arcade-retro-game-over-213.wav", NULL, SND_SYNC | SND_FILENAME | SND_LOOP);
			PlaySound("mixkit-retro-game-emergency-alarm-1000.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
			level++;
			if (level > 3) {
				level = 1;
			}
		}
	}
	else {
		printf("MISS\n");
	}



}

void init(void) 
{	
	// View  positioned back -5 on the z axis, looking into the screen.
	glm::vec3 initpos = glm::vec3(0.0f, 0.0f, -10.0f);
	glm::vec3 initlookatpnt = glm::vec3(.0f, .0f, -1.0f);
	camera = new QuatCamera(800,600,initpos, initlookatpnt, glm::vec3(0.0f, 2.0f, 0.0f));
	// Perspective projection matrix.
	projection = glm::perspective(45.0f, 800.0f/600.0f, 1.0f, 1000.0f);

	initParticles();
	

	// Load identity matrix into model matrix.
	model = glm::mat4();

	initShader ();
	initRendering ();
}

void dumpInfo(void)
{
	printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
	checkError ("dumpInfo");
}


void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->OnRender();

	auto now = std::chrono::high_resolution_clock::now();
	float delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime).count() / 1000.0f;
	previousTime = now;


	view = glm::lookAt(camera->GetPos(), camera->GetLookAtPoint(), camera->GetUp());
	
	
	
	 glm::vec4 lightPos = glm::translate(0.0f, 0.0f, 1.0f) * lightPosition;
	
	shader.Activate(); // Bind shader.
	shader.SetUniform("lightPosition", view*lightPos);
	shader.SetUniform("lightDiffuse", glm::vec4(1.0, 1.0, 1.0, 1.0));
	shader.SetUniform("lightSpecular", glm::vec4(1.0, 1.0, 1.0, 1.0));
	shader.SetUniform("lightAmbient", glm::vec4(1.0, 1.0, 1.0, 1.0));
	bool useMat = false;
	
	//mesh->render(view * model,projection, true); // Render current active model.


	weapon->setOverrideDiffuseMaterial( glm::vec4(.3, 0.3, 0.3, 1.0));
	weapon->setOverrideAmbientMaterial(  glm::vec4(0.0, 0.0, 0.0, 1.0));
	weapon->setOverrideSpecularShininessMaterial( 40.0f);
	weapon->setOverrideEmissiveMaterial(  glm::vec4(0.0, 0.0, 0.0, 1.0));
	weapon->render(glm::translate(1.0f, -1.0f, -2.0f) * glm::scale(.05f, .05f, .05f) * glm::rotate(-90.0f, 0.0f, 1.0f, 0.0f), projection, false);

	
	plane->setOverrideSpecularMaterial( glm::vec4(.70, 0.70, 0.70, 1.0));
	plane->setOverrideDiffuseMaterial( glm::vec4(1.0, 0.0, 0.0, 1.0));
	plane->setOverrideAmbientMaterial(  glm::vec4(1.0 , 0.0, 0.0, 1.0));
	
	if (enableParticles)
	{
		renderParticles();
		updateParticles(delta_time);
	}


	// Moving left and right.
	if (targetPosX >= 10.0f) {
		isTargetMovingRight = false;
	}
	else if (targetPosX <= -10.0f) {
		isTargetMovingRight = true;
	}

	// Moving up and down.
	if (targetPosY >= 5.0f) {
		isTargetMovingUp = false;
	}
	else if (targetPosY <= -1.0f) {
		isTargetMovingUp = true;
	}
	if (level == 1) {
		glClearColor(0.117f, 0.565f, 1.0f, 0.0f);

	}
	if (level == 2) {
		glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
		targetSpeed = 0.03f;
	}

	if (level >= 3) {
		glClearColor(0.5f, 0.0f, 0.0f, 0.0f);

		if (isTargetMovingUp) {
			targetPosY += (1.0f * targetSpeed);
			yMinTarget += (1.0f * targetSpeed);
			yMaxTarget += (1.0f * targetSpeed);
		}
		else {
			targetPosY -= (1.0f * targetSpeed);
			yMinTarget -= (1.0f * targetSpeed);
			yMaxTarget -= (1.0f * targetSpeed);
		}
	}

	if (isTargetMovingRight) {

		targetPosX += (1.0f * targetSpeed);
		xMinTarget += (1.0f * targetSpeed);
		xMaxTarget += (1.0f * targetSpeed);

	}
	else {
		targetPosX -= (1.0f * targetSpeed);
		xMinTarget -= (1.0f * targetSpeed);
		xMaxTarget -= (1.0f * targetSpeed);

	}



	target->setOverrideDiffuseMaterial(glm::vec4(1.0, 0.0, 0.0, 1.0));
	target->setOverrideAmbientMaterial(glm::vec4(0.0, 0.0, 0.0, 1.0));
	target->setOverrideSpecularMaterial(glm::vec4(1.0, 1.0, 1.0, 1.0));
	target->setOverrideEmissiveMaterial(glm::vec4(0.0, 0.0, 0.0, 1.0));
	target->render(view * glm::translate(targetPosX, targetPosY, 0.0f) * glm::rotate(90.0f, 1.0f, 0.0f, 0.0f) * glm::scale(1.0f, 0.05f, 1.0f), projection, useMat);


	//plane->setOverrideSpecularMaterial( glm::vec4(1.0, 1.0, 1.0, 1.0));
	//plane->setOverrideSpecularShininessMaterial( 90.0f);
	//plane->setOverrideEmissiveMaterial(  glm::vec4(0.0, 0.0, 0.0, 1.0));
	plane->render(view*glm::translate(0.0f,-2.0f,0.0f)*glm::scale(100.0f,100.0f,100.0f), projection, useMat);
	
	mesh->setOverrideEmissiveMaterial(  glm::vec4(1.0, 1.0, 1.0, 1.0));
	//mesh->render(view * glm::translate(lightPos.x,lightPos.y, lightPos.z)*glm::scale(.1f,.1f,.1f), projection, false);


	

	glutSwapBuffers(); // Swap the buffers.

	checkError ("display");
}

void idle()
{
	glutPostRedisplay();
}


void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	checkError ("reshape");
}

void specialKeyboard(int Key, int x, int y)
{
    camera->OnKeyboard(Key);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;

	//TODO remove this when actual level is needed from increase
	case 'w':
		level += 1;
		break;
	case 'x':
		enableParticles = !enableParticles;
		break;

   }

}

void MouseButton(int button, int state, int x, int y)
{
	GLfloat mousePositionOnClickx = x;
	GLfloat mousePositionOnClicky = y;

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		resetParticles();

		printf("CLICK\n");
		printf("x position: %d\n", x);
		printf("y position: %d\n", y);
		shootray(mousePositionOnClickx, mousePositionOnClicky);
	}
}


//unsigned int loadCubemap(vector<std::string> faces)
//{
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
//
//	int width, height, nrChannels;
//	for (unsigned int i = 0; i < faces.size(); i++)
//	{
//		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
//		if (data)
//		{
//			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//			stbi_image_free(data);
//		}
//		else
//		{
//			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
//			stbi_image_free(data);
//		}
//	}
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//	return textureID;
//}

static void passiveMouse(int x, int y)
{
   camera->OnMouse(x, y);
}

int main(int argc, char** argv)
{

	//float skyboxVertices[] = {
	//	// positions          
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f, -1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,

	//	-1.0f, -1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f,
	//	-1.0f, -1.0f,  1.0f,

	//	-1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f, -1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	 1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f,  1.0f,
	//	-1.0f,  1.0f, -1.0f,

	//	-1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	 1.0f, -1.0f, -1.0f,
	//	-1.0f, -1.0f,  1.0f,
	//	 1.0f, -1.0f,  1.0f
	//};

	//unsigned int skyboxVAO, skyboxVBO;
	//glGenVertexArrays(1, &skyboxVAO);
	//glGenBuffers(1, &skyboxVBO);
	//glBindVertexArray(skyboxVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//vector<std::string> faces
	//{
	//	FileSystem::getPath("models/right.jpg"),
	//	FileSystem::getPath("models/left.jpg"),
	//	FileSystem::getPath("models/top.jpg"),
	//	FileSystem::getPath("models/bottom.jpg"),
	//	FileSystem::getPath("models/front.jpg"),
	//	FileSystem::getPath("models/back.jpg")
	//};
	//unsigned int cubemapTexture = loadCubemap(faces);



	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE| GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (800, 600); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("Lighting and Quaternion Camera Demo");

	glewInit();
	dumpInfo ();
	init ();

	glutDisplayFunc(display); 
	glutIdleFunc(idle); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc (keyboard);
	glutSpecialFunc(specialKeyboard);
    glutPassiveMotionFunc(passiveMouse);
	glutMouseFunc(MouseButton);

	glEnable(GL_DEPTH_TEST);

	//glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	//skyboxShader.use();
	//view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	//skyboxShader.setMat4("view", view);
	//skyboxShader.setMat4("projection", projection);
	//// skybox cube
	//glBindVertexArray(skyboxVAO);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	//glBindVertexArray(0);
	//glDepthFunc(GL_LESS); // set depth function back to default


	//Music
	PlaySound("mixkit-retro-game-emergency-alarm-1000.wav", NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

	sphere = new Model(&shader,"models/sphere.obj", "models/");
	plane = new Model(&shader,"models/plane.obj",  "models/");
	weapon = new Model(&shader, "models/m16_1.obj", "models/");
	target = new Model(&shader, "models/cylinder.obj", "models/");
	mesh = sphere;

	glutMainLoop();

   return 0;
}