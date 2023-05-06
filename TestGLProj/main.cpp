#include <GL/glew.h>
#include <GL/freeglut.h>

//glm library
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "Shader.h"
#include "QuatCamera.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


Shader shader;
Model *mesh;

Model *sphere;
Model *target;
Model *plane;
Model *bow;
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;
bool drawTorus = true;
bool spin = false;
float targetPosX = 1.0f;
float targetPosY = 0.0f;
float targetSpeed = 0.05f;
int level = 1;
bool isTargetMovingRight;
bool isTargetMovingUp;
glm::vec4 lightPosition = glm::vec4(0.0f,3.0f,0.0f,1.0f);

QuatCamera * camera;

/* report GL errors, if any, to stderr */
void checkError(const char *functionName)
{
	GLenum error;
	while (( error = glGetError() ) != GL_NO_ERROR) {
	  std::cerr << "GL error " << error << " detected in " << functionName << std::endl;
	}
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

void init(void) 
{	
	// View  positioned back -5 on the z axis, looking into the screen.
	glm::vec3 initpos = glm::vec3(0.0f, 0.0f, -10.0f);
	glm::vec3 initlookatpnt = glm::vec3(.0f, .0f, -1.0f);
	camera = new QuatCamera(800,600,initpos, initlookatpnt, glm::vec3(0.0f, 2.0f, 0.0f));
	// Perspective projection matrix.
	projection = glm::perspective(45.0f, 800.0f/600.0f, 1.0f, 1000.0f);


	

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

void renderTargets(bool useMat)
{

}

void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->OnRender();

	view = glm::lookAt(camera->GetPos(), camera->GetLookAtPoint(), camera->GetUp());
	
	
	
	 glm::vec4 lightPos = glm::translate(0.0f, 0.0f, 1.0f) * lightPosition;
	
	shader.Activate(); // Bind shader.
	shader.SetUniform("lightPosition", view*lightPos);
	shader.SetUniform("lightDiffuse", glm::vec4(1.0, 1.0, 1.0, 1.0));
	shader.SetUniform("lightSpecular", glm::vec4(1.0, 1.0, 1.0, 1.0));
	shader.SetUniform("lightAmbient", glm::vec4(1.0, 1.0, 1.0, 1.0));
	bool useMat = false;
	
	//mesh->render(view * model,projection, true); // Render current active model.
	//bow->setOverrideDiffuseMaterial( glm::vec4(.3, 0.3, 0.3, 1.0));
	//bow->setOverrideAmbientMaterial(  glm::vec4(0.0, 0.0, 0.0, 1.0));
	//bow->setOverrideSpecularShininessMaterial( 40.0f);
	//bow->setOverrideEmissiveMaterial(  glm::vec4(0.0, 0.0, 0.0, 1.0));
	bow->render(glm::translate(0.0f,0.0f,-3.0f)* glm::scale(0.05f,0.05f, 0.05f) , projection, false);

	
	plane->setOverrideSpecularMaterial( glm::vec4(.70, 0.70, 0.70, 1.0));
	plane->setOverrideDiffuseMaterial( glm::vec4(1.0, 0.0, 0.0, 1.0));
	plane->setOverrideAmbientMaterial(  glm::vec4(1.0 , 0.0, 0.0, 1.0));
	

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
	
	if (level == 2) {
		targetSpeed = 0.15f;
	}

	if (level >= 3) {
		if (isTargetMovingUp) {
			targetPosY += (1.0f * targetSpeed);
		}
		else {
			targetPosY -= (1.0f * targetSpeed);
		}
	}

	if (isTargetMovingRight) {
		
		targetPosX += (1.0f * targetSpeed);
		
	}
	else {
		targetPosX -= (1.0f * targetSpeed);
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
   }

}



static void passiveMouse(int x, int y)
{
   camera->OnMouse(x, y);
}

int main(int argc, char** argv)
{
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
	
	glEnable(GL_DEPTH_TEST);


	sphere = new Model(&shader,"models/sphere.obj", "models/");
	plane = new Model(&shader,"models/plane.obj",  "models/");
	bow = new Model(&shader, "models/bow.obj", "models/");
	target = new Model(&shader, "models/cylinder.obj", "models/");
	mesh = sphere;

	glutMainLoop();

   return 0;
}