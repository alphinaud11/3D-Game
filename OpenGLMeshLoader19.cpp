#define _CRT_SECURE_NO_WARNINGS
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <cmath>
#include<time.h>
using namespace std;

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 0.47f, float eyeY = 0.5f, float eyeZ = 2.0f, float centerX = 0.47f, float centerY = 0.5f, float centerZ = 0.5f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		Vector3f value = right.operator*(d);
		eye = eye.operator+(value);
		center = center.operator+(value);
	}

	void moveY(float d) {
		Vector3f value = up.unit().operator*(d);
		eye = eye.operator+(value);
		center = center.operator+(value);
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		Vector3f value = view.operator*(d);
		eye = eye.operator+(value);
		center = center.operator+(value);
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		Vector3f value1 = view.operator*(cos(DEG2RAD(a)));
		Vector3f value2 = up.operator*(sin(DEG2RAD(a)));
		view = value1.operator+(value2);
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		Vector3f value1 = view.operator*(cos(DEG2RAD(a)));
		Vector3f value2 = right.operator*(sin(DEG2RAD(a)));
		view = value1.operator+(value2);
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;
double zNearest = 0.7;        //set the z coordinates of the 3 gates
double zMid = 0.5;               //to control their spacing
double zFurthest = 0.3;

int cameraMode = 3; //  1:top view   2:side view    3:front view

double moveX = 0.4;
double moveZ = 0.85;

double rot = 0;
double goalX = 0.2;
double goalZ = 0.07;

int won = 0;
int timeRemaining = 60;


int firstRandom = rand() % 3;
int secondRandom = rand() % 3;
int thirdRandom = rand() % 3;
int fourthRandom = rand() % 3;
int fifthRandom = rand() % 3;
int firstPowerUpRandom = rand() % 6;
int secondPowerUpRandom = rand() % 6;

/*
	Wall thickness = 1.6;
	Gap size = 5;
*/

double firstX = 0;
double secondX = 0;
double thirdX = 0;
double fourthX = 0;
double fifthX = 0;

/*
	PowerUps positions updated in display according to powerUpRandom in Anim Timer
*/
double powerUpRadius = 2;
double firstPowerUpX = 0;  //Center
double firstPowerUpZ = 0;

double secondPowerUpX = 0;
double secondPowerUpZ = 0;

//Coins positions fixed
double coinRadius = 2;
double firstCoinX = 35;
double firstCoinZ = 3.5;

double secondCoinX = 15;
double secondCoinZ = 12.1;

double thirdCoinX = 45;
double thirdCoinZ = 20.7;

double fourthCoinX = 25;
double fourthCoinZ = 29.3;

double fifthCoinX = 40;
double fifthCoinZ = 37.9;

double sixthCoinX = 5;
double sixthCoinZ = 46.5;

/*
	goal size = 4 * 4 * 4;
*/
//double goalX = 47; //Center
//double goalZ = 3;

int level = 1;
double moveX2 = 0;
double moveZ2 = 0;
double speed = 0.5;
bool coin1Available2 = true;
bool coin2Available2 = true;
bool coin3Available2 = true;
bool coin4Available2 = true;
bool coin5Available2 = true;
bool coin6Available2 = true;
int coinsCollected = 0;
bool powerUp1Available = true;
bool powerUp2Available = true;
bool firstView = false;
bool thirdView = true;
bool coin1Available = true;
bool coin2Available = true;
bool coin3Available = true;
bool coin4Available = true;
bool gameOver = false;
GLTexture tex_wall;
GLTexture tex_fence;
GLTexture tex_ground;
GLTexture tex_wall2;
GLTexture tex_fence2;
GLTexture tex_goal2;
Model_3DS model_player;

void setupLights() {
	if (level == 1) {
		GLfloat lightIntensity[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat lightPosition[] = { 0.5f, 60.0f, 0.5f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);

		GLfloat lightIntensity1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
		GLfloat lightPosition1[] = { moveX, 0.21f, moveZ, 0.0f };
		GLfloat l0Direction[] = { 0.0, 0.0, -1.0 };
		glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightIntensity1);
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l0Direction);
	}
	else {
		GLfloat lightIntensity[] = { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat lightPosition[] = { 25.0f, 10.0f, 25.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);

		GLfloat lightIntensity1[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		GLfloat lightPosition1[] = { 41.2f + moveX2, 6.6f, 43.9f + moveZ2, true };
		GLfloat l0Direction[] = { 0.0, 0.0, -1.0 };
		glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightIntensity1);
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 90.0);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, l0Direction);
	}
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (level == 1) {
		if (firstView) {
			camera.eye.x = moveX;
			camera.eye.y = 0.2;
			camera.eye.z = moveZ - 0.02;
			camera.center.x = moveX;
			camera.center.y = 0.18;
			camera.center.z = moveZ - 0.1;
		}
		else if (thirdView) {
			camera.eye.x = moveX;
			camera.eye.y = 0.5;
			camera.eye.z = 0.4 + moveZ;
			camera.center.x = moveX;
			camera.center.y = 0.3;
			camera.center.z = moveZ;
		}
		else {
			camera.eye.x = 0.5;
			camera.eye.y = 1.3;
			camera.eye.z = 0.5;
			camera.center.x = 0.5;
			camera.center.y = 0;
			camera.center.z = 0.49;
		}
	}
	else {
		if (firstView) {
			camera.eye.x = 41.2 + moveX2;
			camera.eye.y = 6.5;
			camera.eye.z = 44 + moveZ2;
			camera.center.x = 41.2 + moveX2;
			camera.center.y = 5;
			camera.center.z = 40 + moveZ2;

		}
		else if (thirdView) {
			camera.eye.x = 41.2 + moveX2;
			camera.eye.y = 13;
			camera.eye.z = 55 + moveZ2;
			camera.center.x = 41.2 + moveX2;
			camera.center.y = 6.5;
			camera.center.z = 45.5 + moveZ2;
		}
		else {
			camera.eye.x = 25;
			camera.eye.y = 60;
			camera.eye.z = 25;
			camera.center.x = 25;
			camera.center.y = 0;
			camera.center.z = 24.9;
		}
	}
	camera.look();
}

void key(unsigned char k, int x, int y)//keyboard function, takes 3 parameters
									// k is the key pressed from the keyboard
									// x and y are mouse postion when the key was pressed.
{
	if (k == '1') {
		firstView = true;
		thirdView = false;
	}
	if (k == '2') {
		firstView = false;
		thirdView = false;
	}
	if (k == '3') {
		firstView = false;
		thirdView = true;
	}
	if (level == 1 && won == 1) {
		if (k == 'c') {
			level = 2;
			won = 0;
			coinsCollected = 0;
			timeRemaining = 60;
		}
	}
	glutPostRedisplay();//redisplay to update the screen with the changes
}

void print(int x, int y, char* string)
{
	int len, i;

	//set the position of the text in the window using the x, y and z coordinates
	glRasterPos3f(0.5, 0.8, 0.5);
	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void drawBorders() {


	glColor3f(0.1, 0.4, 0.2);       //green
	glPushMatrix();                 //ground
	glTranslated(0.5, 0.03, 0.5);
	glScaled(1, 0.06, 1);
	glutSolidCube(1);
	glPopMatrix();

	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);


	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 5);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0.06, 0.061, 0.06);
	glTexCoord2f(5, 5);
	glVertex3f(1 - 0.06, 0.061, 0.06);
	glTexCoord2f(5, 0);
	glVertex3f(1 - 0.06, 0.061, 1.001);
	glTexCoord2f(0, 0);
	glVertex3f(0.06, 0.061, 1.001);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);




	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_fence.texture[0]);





	//glColor3f(0.4, 0.2, 0.1); //brown

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(5, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 0.301, 0);
	glTexCoord2f(5, 2);
	glVertex3f(0.06, 0.301, 0);
	glTexCoord2f(5, 2);
	glVertex3f(0.06, 0.301, 1);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0.301, 1);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(5, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(1, 0.301, 0);
	glTexCoord2f(5, 2);
	glVertex3f(1 - 0.06, 0.301, 0);
	glTexCoord2f(5, 2);
	glVertex3f(1 - 0.06, 0.301, 1);
	glTexCoord2f(0, 0);
	glVertex3f(1, 0.301, 1);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 2);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 0.301, 0);
	glTexCoord2f(5, 2);
	glVertex3f(1, 0.301, 0);
	glTexCoord2f(5, 0);
	glVertex3f(1, 0.301, 0.06);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0.301, 0.06);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0.06, 0, 0.06);
	glTexCoord2f(5, 0);
	glVertex3f(1 - 0.06, 0, 0.06);
	glTexCoord2f(5, 0);
	glVertex3f(1 - 0.06, 0.301, 0.06);
	glTexCoord2f(0, 0);
	glVertex3f(0.06, 0.301, 0.06);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0.06, 0.06, 1);
	glTexCoord2f(5, 0);
	glVertex3f(0.06, 0.06, 0.06);
	glTexCoord2f(5, 2);
	glVertex3f(0.06, 0.301, 0.06);
	glTexCoord2f(0, 2);
	glVertex3f(0.06, 0.301, 1);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0, 0.06, 1);
	glTexCoord2f(2, 0);
	glVertex3f(0.06, 0.06, 1);
	glTexCoord2f(5, 2);
	glVertex3f(0.06, 0.301, 1);
	glTexCoord2f(0, 2);
	glVertex3f(0, 0.301, 1);
	glEnd();
	glPopMatrix();


	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(1 - 0.06, 0.06, 1);
	glTexCoord2f(5, 0);
	glVertex3f(1 - 0.06, 0.06, 0.06);
	glTexCoord2f(5, 2);
	glVertex3f(1 - 0.06, 0.301, 0.06);
	glTexCoord2f(0, 2);
	glVertex3f(1 - 0.06, 0.301, 1);
	glEnd();
	glPopMatrix();

	glPushMatrix();                 //
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(1 - 0, 0.06, 1);
	glTexCoord2f(2, 0);
	glVertex3f(1 - 0.06, 0.06, 1);
	glTexCoord2f(5, 2);
	glVertex3f(1 - 0.06, 0.301, 1);
	glTexCoord2f(0, 2);
	glVertex3f(1 - 0, 0.301, 1);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);

}

void drawCharacter() {

	// Draw Player Model
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit
	glPushMatrix();
	glTranslatef(moveX, 0.05, moveZ);
	glRotatef(180.f, 0, 1, 0);
	glRotatef(90.f, 1, 0, 0);
	glScalef(0.007, 0.007, 0.007);
	model_player.Draw();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);
	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the texture.

}

void drawGoal() {
	glColor3f(0.9, 0.7, 0.2);
	glPushMatrix();
	glTranslatef(goalX, 0.1, goalZ);
	glutSolidSphere(0.03, 20, 20);
	glPopMatrix();
}

void drawCoins() {
	glColor3f(0.9, 0.7, 0.2);

	if (coin1Available) {
		glPushMatrix();
		glTranslatef(0.1, 0.1, 0.8);
		glRotated(rot, 0, 1, 0);
		glScaled(1, 1, 0.3);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();
	}

	if (coin2Available) {
		glPushMatrix();
		glTranslatef(0.8, 0.1, 0.6);
		glRotated(rot, 0, 1, 0);
		glScaled(1, 1, 0.3);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();
	}

	if (coin3Available) {
		glPushMatrix();
		glTranslatef(0.4, 0.1, 0.4);
		glRotated(rot, 0, 1, 0);
		glScaled(1, 1, 0.3);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();
	}

	if (coin4Available) {
		glPushMatrix();
		glTranslatef(0.85, 0.1, 0.15);
		glRotated(rot, 0, 1, 0);
		glScaled(1, 1, 0.3);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();
	}

}

void RenderWall1(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_wall.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x1, y1, z1);
	glTexCoord2f(5, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(5, 5);
	glVertex3f(x3, y3, z3);
	glTexCoord2f(0, 5);
	glVertex3f(x4, y4, z4);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void drawGates() {
	//full side width = 0.88
	//gate width = 0.2
	//glColor3f(1, 1, 1);

	RenderWall1(0.06, 0.3, 0.27, 0.6, 0.3, 0.27, 0.6, 0, 0.27, 0.06, 0, 0.27);
	RenderWall1(0.06, 0.3, 0.27, 0.06, 0.3, 0.33, 0.6, 0.3, 0.33, 0.6, 0.3, 0.27);
	RenderWall1(0.06, 0.3, 0.33, 0.6, 0.3, 0.33, 0.6, 0, 0.33, 0.06, 0, 0.33);
	RenderWall1(0.6, 0.3, 0.33, 0.6, 0.3, 0.27, 0.6, 0, 0.27, 0.6, 0, 0.33);
	/*glPushMatrix();
	glTranslated(0.33, 0.15, zFurthest);           //furthest left
	glScaled(0.54, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/

	RenderWall1(0.8, 0.3, 0.27, 0.94, 0.3, 0.27, 0.94, 0, 0.27, 0.8, 0, 0.27);
	RenderWall1(0.8, 0.3, 0.27, 0.8, 0.3, 0.33, 0.94, 0.3, 0.33, 0.94, 0.3, 0.27);
	RenderWall1(0.8, 0.3, 0.33, 0.94, 0.3, 0.33, 0.94, 0, 0.33, 0.8, 0, 0.33);
	RenderWall1(0.8, 0.3, 0.33, 0.8, 0.3, 0.27, 0.8, 0, 0.27, 0.8, 0, 0.33);
	/*glPushMatrix();
	glTranslated(0.87, 0.15, zFurthest);           //furthest right
	glScaled(0.14, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/

	RenderWall1(0.06, 0.3, 0.47, 0.16, 0.3, 0.47, 0.16, 0, 0.47, 0.06, 0, 0.47);
	RenderWall1(0.06, 0.3, 0.47, 0.06, 0.3, 0.53, 0.16, 0.3, 0.53, 0.16, 0.3, 0.47);
	RenderWall1(0.06, 0.3, 0.53, 0.16, 0.3, 0.53, 0.16, 0, 0.53, 0.06, 0, 0.53);
	RenderWall1(0.16, 0.3, 0.53, 0.16, 0.3, 0.47, 0.16, 0, 0.47, 0.16, 0, 0.53);
	/*glPushMatrix();
	glTranslated(0.11, 0.15, zMid);       //mid left
	glScaled(0.1, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/

	RenderWall1(0.36, 0.3, 0.47, 0.94, 0.3, 0.47, 0.94, 0, 0.47, 0.36, 0, 0.47);
	RenderWall1(0.36, 0.3, 0.47, 0.36, 0.3, 0.53, 0.94, 0.3, 0.53, 0.94, 0.3, 0.47);
	RenderWall1(0.36, 0.3, 0.53, 0.94, 0.3, 0.53, 0.94, 0, 0.53, 0.36, 0, 0.53);
	RenderWall1(0.36, 0.3, 0.53, 0.36, 0.3, 0.47, 0.36, 0, 0.47, 0.36, 0, 0.53);
	/*glPushMatrix();
	glTranslated(0.65, 0.15, zMid);       //mid right
	glScaled(0.58, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/

	RenderWall1(0.06, 0.3, 0.67, 0.54, 0.3, 0.67, 0.54, 0, 0.67, 0.06, 0, 0.67);
	RenderWall1(0.06, 0.3, 0.67, 0.06, 0.3, 0.73, 0.54, 0.3, 0.73, 0.54, 0.3, 0.67);
	RenderWall1(0.06, 0.3, 0.73, 0.54, 0.3, 0.73, 0.54, 0, 0.73, 0.06, 0, 0.73);
	RenderWall1(0.54, 0.3, 0.73, 0.54, 0.3, 0.67, 0.54, 0, 0.67, 0.54, 0, 0.73);
	/*glPushMatrix();
	glTranslated(0.3, 0.15, zNearest);           //nearest left
	glScaled(0.48, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/

	RenderWall1(0.74, 0.3, 0.67, 0.94, 0.3, 0.67, 0.94, 0, 0.67, 0.74, 0, 0.67);
	RenderWall1(0.74, 0.3, 0.67, 0.74, 0.3, 0.73, 0.94, 0.3, 0.73, 0.94, 0.3, 0.67);
	RenderWall1(0.74, 0.3, 0.73, 0.94, 0.3, 0.73, 0.94, 0, 0.73, 0.74, 0, 0.73);
	RenderWall1(0.74, 0.3, 0.73, 0.74, 0.3, 0.67, 0.74, 0, 0.67, 0.74, 0, 0.73);
	/*glPushMatrix();
	glTranslated(0.84, 0.15, zNearest);           //nearest right
	glScaled(0.2, 0.3, 0.06);
	glutSolidCube(1);
	glPopMatrix();*/
}

void RenderGround(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x1, y1, z1);
	glTexCoord2f(5, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(5, 5);
	glVertex3f(x3, y3, z3);
	glTexCoord2f(0, 5);
	glVertex3f(x4, y4, z4);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderFence(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_fence2.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x1, y1, z1);
	glTexCoord2f(5, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(5, 5);
	glVertex3f(x3, y3, z3);
	glTexCoord2f(0, 5);
	glVertex3f(x4, y4, z4);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderWall(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_wall2.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x1, y1, z1);
	glTexCoord2f(5, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(5, 5);
	glVertex3f(x3, y3, z3);
	glTexCoord2f(0, 5);
	glVertex3f(x4, y4, z4);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderGoal(double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_goal2.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(x1, y1, z1);
	glTexCoord2f(5, 0);
	glVertex3f(x2, y2, z2);
	glTexCoord2f(5, 5);
	glVertex3f(x3, y3, z3);
	glTexCoord2f(0, 5);
	glVertex3f(x4, y4, z4);
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void Display() {
	setupLights();
	setupCamera();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (level == 1) {
		if (gameOver) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60, 640 / 480, 0.001, 100);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			camera.eye.x = 1.0f;
			camera.eye.y = 2.0f;
			camera.eye.z = 0.0f;
			camera.center.x = 0.0f;
			camera.center.y = 0.0f;
			camera.center.z = 0.0f;
			camera.look();
			glColor3f(1, 0, 0);
			char* p0s[20];
			sprintf((char*)p0s, "Time Up");
			print(0.5, 0.5, (char*)p0s);
		}

		if (won == 1) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60, 640 / 480, 0.001, 100);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			camera.eye.x = 1.0f;
			camera.eye.y = 2.0f;
			camera.eye.z = 0.0f;
			camera.center.x = 0.0f;
			camera.center.y = 0.0f;
			camera.center.z = 0.0f;
			camera.look();
			glColor3f(0, 0, 1);
			char* p0s[20];
			sprintf((char*)p0s, "Score: %d, Press C to continue", coinsCollected);
			print(0.5, 0.5, (char*)p0s);
		}

		if (won == 0 && !gameOver) {

			glPushMatrix();
			drawBorders();
			glPopMatrix();

			glPushMatrix();
			drawCharacter();
			glPopMatrix();

			glPushMatrix();
			drawGoal();
			glPopMatrix();

			glPushMatrix();
			drawGates();
			glPopMatrix();

			glPushMatrix();
			drawCoins();
			glPopMatrix();

		}
	}

	else {
		if (gameOver) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60, 640 / 480, 0.001, 100);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			camera.eye.x = 1.0f;
			camera.eye.y = 2.0f;
			camera.eye.z = 0.0f;
			camera.center.x = 0.0f;
			camera.center.y = 0.0f;
			camera.center.z = 0.0f;
			camera.look();
			glColor3f(1, 0, 0);
			char* p0s[20];
			sprintf((char*)p0s, "Time Up");
			print(0.5, 0.5, (char*)p0s);
		}

		if (won == 1) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60, 640 / 480, 0.001, 100);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			camera.eye.x = 1.0f;
			camera.eye.y = 2.0f;
			camera.eye.z = 0.0f;
			camera.center.x = 0.0f;
			camera.center.y = 0.0f;
			camera.center.z = 0.0f;
			camera.look();
			glColor3f(0, 0, 1);
			char* p0s[20];
			sprintf((char*)p0s, "Score: %d", coinsCollected);
			print(0, 0, (char*)p0s);
		}

		if (won == 0 & !gameOver) {

			RenderGround(0, 0.5, 0, 50, 0.5, 0, 50, 0.5, 50, 0, 0.5, 50);

			//Ground
			glColor3f(0.3, 0.9, 0);
			glPushMatrix();
			glTranslated(25, 0, 25);
			glScaled(50, 0, 50);
			glutSolidCube(1);
			glPopMatrix();



			//Walls
			RenderFence(0, 6.5, 0, 50, 6.5, 0, 50, 0, 0, 0, 0, 0);
			RenderFence(0, 6.5, 0, 0, 6.5, -2, 50, 6.5, -2, 50, 6.5, 0);


			RenderFence(0.01, 6.5, 0, 0.01, 6.5, 50, 0.01, 0, 50, 0.01, 0, 0);
			RenderFence(0, 6.51, 0, -2, 6.51, 0, -2, 6.51, 50, 0, 6.51, 50);
			RenderFence(0, 6.50, 50.01, -2, 6.50, 50.01, -2, 0, 50.01, 0, 0, 50.01);
			

			RenderFence(49.99, 6.5, 0, 49.99, 6.5, 50, 49.99, 0, 49.99, 50, 0, 0);
			RenderFence(50, 6.51, 0, 52, 6.51, 0, 52, 6.51, 50, 50, 6.51, 50);
			RenderFence(50, 6.5, 50.01, 52, 6.5, 50.01, 52, 0, 50.01, 50, 0, 50.01);
			

			//First
			glColor3f(0.3, 0.1, 0.1);

			if (firstRandom == 0)
			{
				RenderWall(0, 6.5, 8.6, 10, 6.5, 8.6, 10, 0, 8.6, 0, 0, 8.6);
				RenderWall(0, 6.5, 8.6, 0, 6.5, 7, 10, 6.5, 7, 10, 6.5, 8.6);
				RenderWall(10, 6.5, 8.6, 10, 6.5, 7, 10, 0, 7, 10, 0, 8.6);
				RenderWall(10, 6.5, 7, 0, 6.5, 7, 0, 0, 7, 10, 0, 7);
				firstX = 10;
				

				RenderWall(15, 6.5, 8.6, 50, 6.5, 8.6, 50, 0, 8.6, 15, 0, 8.6);
				RenderWall(15, 6.5, 8.6, 15, 6.5, 7, 50, 6.5, 7, 50, 6.5, 8.6);
				RenderWall(15, 6.5, 8.6, 15, 6.5, 7, 15, 0, 7, 15, 0, 8.6);
				RenderWall(50, 6.5, 7, 15, 6.5, 7, 15, 0, 7, 50, 0, 7);
				
			}

			if (firstRandom == 1)
			{
				RenderWall(0, 6.5, 8.6, 22, 6.5, 8.6, 22, 0, 8.6, 0, 0, 8.6);
				RenderWall(0, 6.5, 8.6, 0, 6.5, 7, 22, 6.5, 7, 22, 6.5, 8.6);
				RenderWall(22, 6.5, 8.6, 22, 6.5, 7, 22, 0, 7, 22, 0, 8.6);
				RenderWall(22, 6.5, 7, 0, 6.5, 7, 0, 0, 7, 22, 0, 7);
				firstX = 22;
				

				RenderWall(27, 6.5, 8.6, 50, 6.5, 8.6, 50, 0, 8.6, 27, 0, 8.6);
				RenderWall(27, 6.5, 8.6, 27, 6.5, 7, 50, 6.5, 7, 50, 6.5, 8.6);
				RenderWall(27, 6.5, 8.6, 27, 6.5, 7, 27, 0, 7, 27, 0, 8.6);
				RenderWall(50, 6.5, 7, 27, 6.5, 7, 27, 0, 7, 50, 0, 7);
				
			}

			if (firstRandom == 2)
			{
				RenderWall(0, 6.5, 8.6, 37, 6.5, 8.6, 37, 0, 8.6, 0, 0, 8.6);
				RenderWall(0, 6.5, 8.6, 0, 6.5, 7, 37, 6.5, 7, 37, 6.5, 8.6);
				RenderWall(37, 6.5, 8.6, 37, 6.5, 7, 37, 0, 7, 37, 0, 8.6);
				RenderWall(37, 6.5, 7, 0, 6.5, 7, 0, 0, 7, 37, 0, 7);
				firstX = 37;
				

				RenderWall(42, 6.5, 8.6, 50, 6.5, 8.6, 50, 0, 8.6, 42, 0, 8.6);
				RenderWall(42, 6.5, 8.6, 42, 6.5, 7, 50, 6.5, 7, 50, 6.5, 8.6);
				RenderWall(42, 6.5, 8.6, 42, 6.5, 7, 42, 0, 7, 42, 0, 8.6);
				RenderWall(50, 6.5, 7, 42, 6.5, 7, 42, 0, 7, 50, 0, 7);
				
			}

			//Second
			if (secondRandom == 0)
			{
				RenderWall(0, 6.5, 17.2, 10, 6.5, 17.2, 10, 0, 17.2, 0, 0, 17.2);
				RenderWall(0, 6.5, 17.2, 0, 6.5, 15.6, 10, 6.5, 15.6, 10, 6.5, 17.2);
				RenderWall(10, 6.5, 17.2, 10, 6.5, 15.6, 10, 0, 15.6, 10, 0, 17.2);
				RenderWall(10, 6.5, 15.6, 0, 6.5, 15.6, 0, 0, 15.6, 10, 0, 15.6);
				secondX = 10;
				

				RenderWall(15, 6.5, 17.2, 50, 6.5, 17.2, 50, 0, 17.2, 15, 0, 17.2);
				RenderWall(15, 6.5, 17.2, 15, 6.5, 15.6, 50, 6.5, 15.6, 50, 6.5, 17.2);
				RenderWall(15, 6.5, 17.2, 15, 6.5, 15.6, 15, 0, 15.6, 15, 0, 17.2);
				RenderWall(50, 6.5, 15.6, 15, 6.5, 15.6, 15, 0, 15.6, 50, 0, 15.6);
				
			}

			if (secondRandom == 1)
			{
				RenderWall(0, 6.5, 17.2, 22, 6.5, 17.2, 22, 0, 17.2, 0, 0, 17.2);
				RenderWall(0, 6.5, 17.2, 0, 6.5, 15.6, 22, 6.5, 15.6, 22, 6.5, 17.2);
				RenderWall(22, 6.5, 17.2, 22, 6.5, 15.6, 22, 0, 15.6, 22, 0, 17.2);
				RenderWall(22, 6.5, 15.6, 0, 6.5, 15.6, 0, 0, 15.6, 22, 0, 15.6);
				secondX = 22;
				
				RenderWall(27, 6.5, 17.2, 50, 6.5, 17.2, 50, 0, 17.2, 27, 0, 17.2);
				RenderWall(27, 6.5, 17.2, 27, 6.5, 15.6, 50, 6.5, 15.6, 50, 6.5, 17.2);
				RenderWall(27, 6.5, 17.2, 27, 6.5, 15.6, 27, 0, 15.6, 27, 0, 17.2);
				RenderWall(50, 6.5, 15.6, 27, 6.5, 15.6, 27, 0, 15.6, 50, 0, 15.6);
				
			}

			if (secondRandom == 2)
			{
				RenderWall(0, 6.5, 17.2, 37, 6.5, 17.2, 37, 0, 17.2, 0, 0, 17.2);
				RenderWall(0, 6.5, 17.2, 0, 6.5, 15.6, 37, 6.5, 15.6, 37, 6.5, 17.2);
				RenderWall(37, 6.5, 17.2, 37, 6.5, 15.6, 37, 0, 15.6, 37, 0, 17.2);
				RenderWall(37, 6.5, 15.6, 0, 6.5, 15.6, 0, 0, 15.6, 37, 0, 15.6);
				secondX = 37;
				
				RenderWall(42, 6.5, 17.2, 50, 6.5, 17.2, 50, 0, 17.2, 42, 0, 17.2);
				RenderWall(42, 6.5, 17.2, 42, 6.5, 15.6, 50, 6.5, 15.6, 50, 6.5, 17.2);
				RenderWall(42, 6.5, 17.2, 42, 6.5, 15.6, 42, 0, 15.6, 42, 0, 17.2);
				RenderWall(50, 6.5, 15.6, 42, 6.5, 15.6, 42, 0, 15.6, 50, 0, 15.6);
				
			}

			//Third
			if (thirdRandom == 0)
			{
				RenderWall(0, 6.5, 25.8, 10, 6.5, 25.8, 10, 0, 25.8, 0, 0, 25.8);
				RenderWall(0, 6.5, 25.8, 0, 6.5, 24.2, 10, 6.5, 24.2, 10, 6.5, 25.8);
				RenderWall(10, 6.5, 25.8, 10, 6.5, 24.2, 10, 0, 24.2, 10, 0, 25.8);
				RenderWall(10, 6.5, 24.2, 0, 6.5, 24.2, 0, 0, 24.2, 10, 0, 24.2);
				thirdX = 10;
				

				RenderWall(15, 6.5, 25.8, 50, 6.5, 25.8, 50, 0, 25.8, 15, 0, 25.8);
				RenderWall(15, 6.5, 25.8, 15, 6.5, 24.2, 50, 6.5, 24.2, 50, 6.5, 25.8);
				RenderWall(15, 6.5, 25.8, 15, 6.5, 24.2, 15, 0, 24.2, 15, 0, 25.8);
				RenderWall(50, 6.5, 24.2, 15, 6.5, 24.2, 15, 0, 24.2, 50, 0, 24.2);
				
			}

			if (thirdRandom == 1)
			{
				RenderWall(0, 6.5, 25.8, 22, 6.5, 25.8, 22, 0, 25.8, 0, 0, 25.8);
				RenderWall(0, 6.5, 25.8, 0, 6.5, 24.2, 22, 6.5, 24.2, 22, 6.5, 25.8);
				RenderWall(22, 6.5, 25.8, 22, 6.5, 24.2, 22, 0, 24.2, 22, 0, 25.8);
				RenderWall(22, 6.5, 24.2, 0, 6.5, 24.2, 0, 0, 24.2, 22, 0, 24.2);
				thirdX = 22;
				

				RenderWall(27, 6.5, 25.8, 50, 6.5, 25.8, 50, 0, 25.8, 27, 0, 25.8);
				RenderWall(27, 6.5, 25.8, 27, 6.5, 24.2, 50, 6.5, 24.2, 50, 6.5, 25.8);
				RenderWall(27, 6.5, 25.8, 27, 6.5, 24.2, 27, 0, 24.2, 27, 0, 25.8);
				RenderWall(50, 6.5, 24.2, 27, 6.5, 24.2, 27, 0, 24.2, 50, 0, 24.2);
				
			}

			if (thirdRandom == 2)
			{
				RenderWall(0, 6.5, 25.8, 37, 6.5, 25.8, 37, 0, 25.8, 0, 0, 25.8);
				RenderWall(0, 6.5, 25.8, 0, 6.5, 24.2, 37, 6.5, 24.2, 37, 6.5, 25.8);
				RenderWall(37, 6.5, 25.8, 37, 6.5, 24.2, 37, 0, 24.2, 37, 0, 25.8);
				RenderWall(37, 6.5, 24.2, 0, 6.5, 24.2, 0, 0, 24.2, 37, 0, 24.2);
				thirdX = 37;
				
				RenderWall(42, 6.5, 25.8, 50, 6.5, 25.8, 50, 0, 25.8, 42, 0, 25.8);
				RenderWall(42, 6.5, 25.8, 42, 6.5, 24.2, 50, 6.5, 24.2, 50, 6.5, 25.8);
				RenderWall(42, 6.5, 25.8, 42, 6.5, 24.2, 42, 0, 24.2, 42, 0, 25.8);
				RenderWall(50, 6.5, 24.2, 42, 6.5, 24.2, 42, 0, 24.2, 50, 0, 24.2);
				
			}

			//Fourth
			if (fourthRandom == 0)
			{
				RenderWall(0, 6.5, 34.4, 10, 6.5, 34.4, 10, 0, 34.4, 0, 0, 34.4);
				RenderWall(0, 6.5, 34.4, 0, 6.5, 32.8, 10, 6.5, 32.8, 10, 6.5, 34.4);
				RenderWall(10, 6.5, 34.4, 10, 6.5, 32.8, 10, 0, 32.8, 10, 0, 34.4);
				RenderWall(10, 6.5, 32.8, 0, 6.5, 32.8, 0, 0, 32.8, 10, 0, 32.8);
				fourthX = 10;
				

				RenderWall(15, 6.5, 34.4, 50, 6.5, 34.4, 50, 0, 34.4, 15, 0, 34.4);
				RenderWall(15, 6.5, 34.4, 15, 6.5, 32.8, 50, 6.5, 32.8, 50, 6.5, 34.4);
				RenderWall(15, 6.5, 34.4, 15, 6.5, 32.8, 15, 0, 32.8, 15, 0, 34.4);
				RenderWall(50, 6.5, 32.8, 15, 6.5, 32.8, 15, 0, 32.8, 50, 0, 32.8);
				
			}

			if (fourthRandom == 1)
			{
				RenderWall(0, 6.5, 34.4, 22, 6.5, 34.4, 22, 0, 34.4, 0, 0, 34.4);
				RenderWall(0, 6.5, 34.4, 0, 6.5, 32.8, 22, 6.5, 32.8, 22, 6.5, 34.4);
				RenderWall(22, 6.5, 34.4, 22, 6.5, 32.8, 22, 0, 32.8, 22, 0, 34.4);
				RenderWall(22, 6.5, 32.8, 0, 6.5, 32.8, 0, 0, 32.8, 22, 0, 32.8);
				fourthX = 22;
			

				RenderWall(27, 6.5, 34.4, 50, 6.5, 34.4, 50, 0, 34.4, 27, 0, 34.4);
				RenderWall(27, 6.5, 34.4, 27, 6.5, 32.8, 50, 6.5, 32.8, 50, 6.5, 34.4);
				RenderWall(27, 6.5, 34.4, 27, 6.5, 32.8, 27, 0, 32.8, 27, 0, 34.4);
				RenderWall(50, 6.5, 32.8, 27, 6.5, 32.8, 27, 0, 32.8, 50, 0, 32.8);
				
			}

			if (fourthRandom == 2)
			{
				RenderWall(0, 6.5, 34.4, 37, 6.5, 34.4, 37, 0, 34.4, 0, 0, 34.4);
				RenderWall(0, 6.5, 34.4, 0, 6.5, 32.8, 37, 6.5, 32.8, 37, 6.5, 34.4);
				RenderWall(37, 6.5, 34.4, 37, 6.5, 32.8, 37, 0, 32.8, 37, 0, 34.4);
				RenderWall(37, 6.5, 32.8, 0, 6.5, 32.8, 0, 0, 32.8, 37, 0, 32.8);
				fourthX = 37;
				
				RenderWall(42, 6.5, 34.4, 50, 6.5, 34.4, 50, 0, 34.4, 42, 0, 34.4);
				RenderWall(42, 6.5, 34.4, 42, 6.5, 32.8, 50, 6.5, 32.8, 50, 6.5, 34.4);
				RenderWall(42, 6.5, 34.4, 42, 6.5, 32.8, 42, 0, 32.8, 42, 0, 34.4);
				RenderWall(50, 6.5, 32.8, 42, 6.5, 32.8, 42, 0, 32.8, 50, 0, 32.8);
				
			}

			//Fifth
			if (fifthRandom == 0)
			{
				RenderWall(0, 6.5, 43, 10, 6.5, 43, 10, 0, 43, 0, 0, 43);
				RenderWall(0, 6.5, 43, 0, 6.5, 41.4, 10, 6.5, 41.4, 10, 6.5, 43);
				RenderWall(10, 6.5, 43, 10, 6.5, 41.4, 10, 0, 41.4, 10, 0, 43);
				RenderWall(10, 6.5, 41.4, 0, 6.5, 41.4, 0, 0, 41.4, 10, 0, 41.4);
				fifthX = 10;

				RenderWall(15, 6.5, 43, 50, 6.5, 43, 50, 0, 43, 15, 0, 43);
				RenderWall(15, 6.5, 43, 15, 6.5, 41.4, 50, 6.5, 41.4, 50, 6.5, 43);
				RenderWall(15, 6.5, 43, 15, 6.5, 41.4, 15, 0, 41.4, 15, 0, 43);
				RenderWall(50, 6.5, 41.4, 15, 6.5, 41.4, 15, 0, 41.4, 50, 0, 41.4);
			}

			if (fifthRandom == 1)
			{
				RenderWall(0, 6.5, 43, 22, 6.5, 43, 22, 0, 43, 0, 0, 43);
				RenderWall(0, 6.5, 43, 0, 6.5, 41.4, 22, 6.5, 41.4, 22, 6.5, 43);
				RenderWall(22, 6.5, 43, 22, 6.5, 41.4, 22, 0, 41.4, 22, 0, 43);
				RenderWall(22, 6.5, 41.4, 0, 6.5, 41.4, 0, 0, 41.4, 22, 0, 41.4);
				fifthX = 22;

				RenderWall(27, 6.5, 43, 50, 6.5, 43, 50, 0, 43, 27, 0, 43);
				RenderWall(27, 6.5, 43, 27, 6.5, 41.4, 50, 6.5, 41.4, 50, 6.5, 43);
				RenderWall(27, 6.5, 43, 27, 6.5, 41.4, 27, 0, 41.4, 27, 0, 43);
				RenderWall(50, 6.5, 41.4, 27, 6.5, 41.4, 27, 0, 41.4, 50, 0, 41.4);
				
			}

			if (fifthRandom == 2)
			{
				RenderWall(0, 6.5, 43, 37, 6.5, 43, 37, 0, 43, 0, 0, 43);
				RenderWall(0, 6.5, 43, 0, 6.5, 41.4, 37, 6.5, 41.4, 37, 6.5, 43);
				RenderWall(37, 6.5, 43, 37, 6.5, 41.4, 37, 0, 41.4, 37, 0, 43);
				RenderWall(37, 6.5, 41.4, 0, 6.5, 41.4, 0, 0, 41.4, 37, 0, 41.4);
				fifthX = 37;
				

				RenderWall(42, 6.5, 43, 50, 6.5, 43, 50, 0, 43, 42, 0, 43);
				RenderWall(42, 6.5, 43, 42, 6.5, 41.4, 50, 6.5, 41.4, 50, 6.5, 43);
				RenderWall(42, 6.5, 43, 42, 6.5, 41.4, 42, 0, 41.4, 42, 0, 43);
				RenderWall(50, 6.5, 41.4, 42, 6.5, 41.4, 42, 0, 41.4, 50, 0, 41.4);
				
			}

			//Coins

			//1
			if (coin1Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 35, 6, 1 + 3.5);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 35, 6, 1 + 3.5);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//2
			if (coin2Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 15, 6, 1 + 12.1);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 15, 6, 1 + 12.1);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//3
			if (coin3Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 45, 6, 1 + 20.7);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 45, 6, 1 + 20.7);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//4
			if (coin4Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 25, 6, 1 + 29.3);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 25, 6, 1 + 29.3);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//5
			if (coin5Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 40, 6, 1 + 37.9);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 40, 6, 1 + 37.9);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//6
			if (coin6Available2) {
				glColor3f(0.9, 0.5, 0.9);
				glPushMatrix();
				glTranslated(1 + 5, 6, 1 + 46.5);
				glRotated(-90, 1, 0, 0);
				glutSolidCone(2, 2, 4, 4);
				glPopMatrix();

				glPushMatrix();
				glTranslated(1 + 5, 6, 1 + 46.5);
				glRotated(90, 1, 0, 0);
				glutSolidCone(2, 6, 4, 4);
				glPopMatrix();
			}

			//Power-ups

			//1
			if (powerUp1Available) {
				if (firstPowerUpRandom == 0)
				{
					firstPowerUpX = 5;
					firstPowerUpZ = 3.5;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 5, 0, 1 + 3.5);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 5, 2, 1 + 3.5);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (firstPowerUpRandom == 1)
				{
					firstPowerUpX = 29;
					firstPowerUpZ = 12.1;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 29, 0, 1 + 12.1);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 29, 2, 1 + 12.1);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (firstPowerUpRandom == 2)
				{
					firstPowerUpX = 22;
					firstPowerUpZ = 20.7;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 22, 0, 1 + 20.7);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 22, 2, 1 + 20.7);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (firstPowerUpRandom == 3)
				{
					firstPowerUpX = 10;
					firstPowerUpZ = 29.3;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 10, 0, 1 + 29.3);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 10, 2, 1 + 29.3);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (firstPowerUpRandom == 4)
				{
					firstPowerUpX = 30;
					firstPowerUpZ = 37.9;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 30, 0, 1 + 37.9);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 30, 2, 1 + 37.9);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (firstPowerUpRandom == 5)
				{
					firstPowerUpX = 25;
					firstPowerUpZ = 46.5;
					glColor3f(0.9, 0.7, 0.1);
					glPushMatrix();
					glTranslated(1 + 25, 0, 1 + 46.5);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 25, 2, 1 + 46.5);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}
			}

			//2
			if (powerUp2Available) {
				if (secondPowerUpRandom == 0)
				{
					secondPowerUpX = 10;
					secondPowerUpZ = 3.5;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 10, 0, 1 + 3.5);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 5, 2, 1 + 3.5);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (secondPowerUpRandom == 1)
				{
					secondPowerUpX = 40;
					secondPowerUpZ = 12.1;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 40, 0, 1 + 12.1);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 40, 2, 1 + 12.1);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (secondPowerUpRandom == 2)
				{
					secondPowerUpX = 15;
					secondPowerUpZ = 20.7;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 15, 0, 1 + 20.7);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 15, 2, 1 + 20.7);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (secondPowerUpRandom == 3)
				{
					secondPowerUpX = 40;
					secondPowerUpZ = 29.3;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 40, 0, 1 + 29.3);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 40, 2, 1 + 29.3);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (secondPowerUpRandom == 4)
				{
					secondPowerUpX = 10;
					secondPowerUpZ = 37.9;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 10, 0, 1 + 37.9);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 10, 2, 1 + 37.9);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}

				if (secondPowerUpRandom == 5)
				{
					secondPowerUpX = 35;
					secondPowerUpZ = 46.5;
					glColor3f(0.6, 0.7, 0.9);
					glPushMatrix();
					glTranslated(1 + 35, 0, 1 + 46.5);
					glRotatef(-90, 1, 0, 0);
					glutSolidCone(2, 4, 64, 64);
					glPopMatrix();

					glPushMatrix();
					glTranslated(1 + 35, 2, 1 + 46.5);
					glRotatef(-90, 1, 0, 0);
					GLUquadricObj* qobj;
					qobj = gluNewQuadric();
					gluQuadricDrawStyle(qobj, GLU_LINE);
					gluCylinder(qobj, 0.8, 0.8, 3, 128, 128);
					glPopMatrix();
				}
			}

			//Goal
			RenderGoal(45, 7, 1, 45, 7, 5, 45, 0, 5, 45, 0, 1);
			RenderGoal(45, 7, 1, 49, 7, 1, 49, 7, 5, 45, 7, 5);
			RenderGoal(45, 7, 5, 49, 7, 5, 49, 0, 5, 45, 0, 5);
			


			//Character

			/*
				leftX(start) = 39;
				frontZ = 44;
				lowerY = 0;

				widthInXDirection = 43.4 - 39 = 4.4;
				depthInZDirection = 45.5 - 44 = 1.5;
				heightInYDirection = 6.5 - 0 = 6.5;
			*/


			// Draw Player Model
			glDisable(GL_LIGHTING);
			glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
			glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit
			glPushMatrix();
			glTranslatef(41+moveX2, 0.05, 45+moveZ2);
			glRotatef(180.f, 0, 1, 0);
			glRotatef(90.f, 1, 0, 0);
			glScalef(0.22, 0.22, 0.22);
			model_player.Draw();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);	// Disable 2D texturing
			glEnable(GL_LIGHTING);
			glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the texture.

			glutSwapBuffers();
		}
	}

	glFlush();
}

bool overlap(double l2Z, double l2X, double r2Z, double r2X) {
	//l1 : Top Left coordinate of first rectangle.
	//r1 : Bottom Right coordinate of first rectangle.
	//l2 : Top Left coordinate of second rectangle.
	//r2 : Bottom Right coordinate of second rectangle.

	double l1Z;
	double l1X;
	double r1Z;
	double r1X;
	if (level == 1) {
		l1Z = moveZ - 0.015;
		l1X = moveX + 0.045;
		r1Z = moveZ + 0.055;
		r1X = moveX - 0.045;
	}
	else {
		l1Z = 44 + moveZ2;
		l1X = 42.4 + moveX2;
		r1Z = 47.5 + moveZ2;
		r1X = 39 + moveX2;
	}

	// If one rectangle is on left side of other 
	if (l1Z >= r2Z || l2Z >= r1Z)
		return false;
	// If one rectangle is above other 
	if (l1X <= r2X || l2X <= r1X)
		return false;

	return true;
}

void spe(int k, int x, int y) {// keyboard special key function takes 3 parameters
							// int k: is the special key pressed such as the keyboard arrows the f1,2,3 and so on
	bool collision = false;
	if (level == 1) {
		if (k == GLUT_KEY_UP) {
			PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
			moveZ -= 0.01;
			if (overlap(0, 1, 0.06, 0)) { // front fence
				collision = true;
			}
			if (overlap(0.67, 0.54, 0.73, 0.06) || overlap(0.67, 0.94, 0.73, 0.74)) { // first wall
				collision = true;
			}
			if (overlap(0.47, 0.16, 0.53, 0.06) || overlap(0.47, 0.94, 0.53, 0.36)) { // second wall
				collision = true;
			}
			if (overlap(0.27, 0.6, 0.33, 0.06) || overlap(0.27, 0.94, 0.33, 0.8)) { // third wall
				collision = true;
			}
			if (collision) {
				PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ += 0.01;
			}
			if (coin1Available && overlap(0.75, 0.15, 0.85, 0.05)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin1Available = false;
				coinsCollected++;
			}
			if (coin2Available && overlap(0.55, 0.85, 0.65, 0.75)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin2Available = false;
				coinsCollected++;
			}
			if (coin3Available && overlap(0.35, 0.45, 0.45, 0.35)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin3Available = false;
				coinsCollected++;
			}
			if (coin4Available && overlap(0.1, 0.9, 0.2, 0.8)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin4Available = false;
				coinsCollected++;
			}
			if (won == 0 && overlap(0.04, 0.23, 0.1, 0.17)) {
				PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
				won = 1;
			}
		}
		if (k == GLUT_KEY_DOWN) {
			PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
			moveZ += 0.01;
			if (overlap(1, 1, 1.1, 0)) { // back fence
				collision = true;
			}
			if (overlap(0.67, 0.54, 0.73, 0.06) || overlap(0.67, 0.94, 0.73, 0.74)) { // first wall
				collision = true;
			}
			if (overlap(0.47, 0.16, 0.53, 0.06) || overlap(0.47, 0.94, 0.53, 0.36)) { // second wall
				collision = true;
			}
			if (overlap(0.27, 0.6, 0.33, 0.06) || overlap(0.27, 0.94, 0.33, 0.8)) { // third wall
				collision = true;
			}
			if (collision) {
				PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ -= 0.01;
			}
			if (coin1Available && overlap(0.75, 0.15, 0.85, 0.05)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin1Available = false;
				coinsCollected++;
			}
			if (coin2Available && overlap(0.55, 0.85, 0.65, 0.75)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin2Available = false;
				coinsCollected++;
			}
			if (coin3Available && overlap(0.35, 0.45, 0.45, 0.35)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin3Available = false;
				coinsCollected++;
			}
			if (coin4Available && overlap(0.1, 0.9, 0.2, 0.8)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin4Available = false;
				coinsCollected++;
			}
			if (won == 0 && overlap(0.04, 0.23, 0.1, 0.17)) {
				PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
				won = 1;
			}
		}
	}
	else {
		if (k == GLUT_KEY_UP) {
			PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
			moveZ2 -= speed;
			if (overlap(-1, 50, 0, 0)) { // front fence
				collision = true;
			}
			if (fifthRandom == 0 && (overlap(41.4, 10, 43, 0) || overlap(41.4, 50, 43, 15))) { // first wall (gap left)
				collision = true;
			}
			if (fifthRandom == 1 && (overlap(41.4, 22, 43, 0) || overlap(41.4, 50, 43, 27))) { // first wall (gap middle)
				collision = true;
			}
			if (fifthRandom == 2 && (overlap(41.4, 37, 43, 0) || overlap(41.4, 50, 43, 42))) { // first wall (gap right)
				collision = true;
			}
			if (fourthRandom == 0 && (overlap(32.8, 10, 34.4, 0) || overlap(32.8, 50, 34.4, 15))) { // second wall (gap left)
				collision = true;
			}
			if (fourthRandom == 1 && (overlap(32.8, 22, 34.4, 0) || overlap(32.8, 50, 34.4, 27))) { // second wall (gap middle)
				collision = true;
			}
			if (fourthRandom == 2 && (overlap(32.8, 37, 34.4, 0) || overlap(32.8, 50, 34.4, 42))) { // second wall (gap right)
				collision = true;
			}
			if (thirdRandom == 0 && (overlap(24.2, 10, 25.8, 0) || overlap(24.2, 50, 25.8, 15))) { // third wall (gap left)
				collision = true;
			}
			if (thirdRandom == 1 && (overlap(24.2, 22, 25.8, 0) || overlap(24.2, 50, 25.8, 27))) { // third wall (gap middle)
				collision = true;
			}
			if (thirdRandom == 2 && (overlap(24.2, 37, 25.8, 0) || overlap(24.2, 50, 25.8, 42))) { // third wall (gap right)
				collision = true;
			}
			if (secondRandom == 0 && (overlap(15.6, 10, 17.2, 0) || overlap(15.6, 50, 17.2, 15))) { // fourth wall (gap left)
				collision = true;
			}
			if (secondRandom == 1 && (overlap(15.6, 22, 17.2, 0) || overlap(15.6, 50, 17.2, 27))) { // fourth wall (gap middle)
				collision = true;
			}
			if (secondRandom == 2 && (overlap(15.6, 37, 17.2, 0) || overlap(15.6, 50, 17.2, 42))) { // fourth wall (gap right)
				collision = true;
			}
			if (firstRandom == 0 && (overlap(7, 10, 8.6, 0) || overlap(7, 50, 8.6, 15))) { // fifth wall (gap left)
				collision = true;
			}
			if (firstRandom == 1 && (overlap(7, 22, 8.6, 0) || overlap(7, 50, 8.6, 27))) { // fifth wall (gap middle)
				collision = true;
			}
			if (firstRandom == 2 && (overlap(7, 37, 8.6, 0) || overlap(7, 50, 8.6, 42))) { // fifth wall (gap right)
				collision = true;
			}
			if (collision) {
				PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 += speed;
			}
			if (coin1Available2 && overlap(1.5, 37, 5.5, 33)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin1Available2 = false;
				coinsCollected++;
			}
			if (coin2Available2 && overlap(10.1, 17, 14.1, 13)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin2Available2 = false;
				coinsCollected++;
			}
			if (coin3Available2 && overlap(18.7, 47, 22.7, 43)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin3Available2 = false;
				coinsCollected++;
			}
			if (coin4Available2 && overlap(27.3, 27, 31.3, 23)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin4Available2 = false;
				coinsCollected++;
			}
			if (coin5Available2 && overlap(35.9, 42, 39.9, 38)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin5Available2 = false;
				coinsCollected++;
			}
			if (coin6Available2 && overlap(44.5, 7, 48.5, 3)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin6Available2 = false;
				coinsCollected++;
			}
			if (powerUp1Available && firstPowerUpRandom == 0 && overlap(1.5, 7, 5.5, 3)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 1 && overlap(10.1, 31, 14.1, 27)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 2 && overlap(18.7, 24, 22.7, 20)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 3 && overlap(27.3, 12, 31.3, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 4 && overlap(35.9, 32, 39.9, 28)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 5 && overlap(44.5, 27, 48.5, 23)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 0 && overlap(1.5, 12, 5.5, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 1 && overlap(10.1, 42, 14.1, 38)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 2 && overlap(18.7, 17, 22.7, 13)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 3 && overlap(27.3, 42, 31.3, 38)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 4 && overlap(35.9, 12, 39.9, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 5 && overlap(44.5, 37, 48.5, 33)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (won == 0 && overlap(1, 49, 5, 45)) {
				PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
				won = 1;
			}
		}
		if (k == GLUT_KEY_DOWN) {
			PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
			moveZ2 += speed;
			if (overlap(50, 50, 51, 0)) { // back fence
				collision = true;
			}
			if (fifthRandom == 0 && (overlap(41.4, 10, 43, 0) || overlap(41.4, 50, 43, 15))) { // first wall (gap left)
				collision = true;
			}
			if (fifthRandom == 1 && (overlap(41.4, 22, 43, 0) || overlap(41.4, 50, 43, 27))) { // first wall (gap middle)
				collision = true;
			}
			if (fifthRandom == 2 && (overlap(41.4, 37, 43, 0) || overlap(41.4, 50, 43, 42))) { // first wall (gap right)
				collision = true;
			}
			if (fourthRandom == 0 && (overlap(32.8, 10, 34.4, 0) || overlap(32.8, 50, 34.4, 15))) { // second wall (gap left)
				collision = true;
			}
			if (fourthRandom == 1 && (overlap(32.8, 22, 34.4, 0) || overlap(32.8, 50, 34.4, 27))) { // second wall (gap middle)
				collision = true;
			}
			if (fourthRandom == 2 && (overlap(32.8, 37, 34.4, 0) || overlap(32.8, 50, 34.4, 42))) { // second wall (gap right)
				collision = true;
			}
			if (thirdRandom == 0 && (overlap(24.2, 10, 25.8, 0) || overlap(24.2, 50, 25.8, 15))) { // third wall (gap left)
				collision = true;
			}
			if (thirdRandom == 1 && (overlap(24.2, 22, 25.8, 0) || overlap(24.2, 50, 25.8, 27))) { // third wall (gap middle)
				collision = true;
			}
			if (thirdRandom == 2 && (overlap(24.2, 37, 25.8, 0) || overlap(24.2, 50, 25.8, 42))) { // third wall (gap right)
				collision = true;
			}
			if (secondRandom == 0 && (overlap(15.6, 10, 17.2, 0) || overlap(15.6, 50, 17.2, 15))) { // fourth wall (gap left)
				collision = true;
			}
			if (secondRandom == 1 && (overlap(15.6, 22, 17.2, 0) || overlap(15.6, 50, 17.2, 27))) { // fourth wall (gap middle)
				collision = true;
			}
			if (secondRandom == 2 && (overlap(15.6, 37, 17.2, 0) || overlap(15.6, 50, 17.2, 42))) { // fourth wall (gap right)
				collision = true;
			}
			if (firstRandom == 0 && (overlap(7, 10, 8.6, 0) || overlap(7, 50, 8.6, 15))) { // fifth wall (gap left)
				collision = true;
			}
			if (firstRandom == 1 && (overlap(7, 22, 8.6, 0) || overlap(7, 50, 8.6, 27))) { // fifth wall (gap middle)
				collision = true;
			}
			if (firstRandom == 2 && (overlap(7, 37, 8.6, 0) || overlap(7, 50, 8.6, 42))) { // fifth wall (gap right)
				collision = true;
			}
			if (collision) {
				PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 -= speed;
			}
			if (coin1Available2 && overlap(1.5, 37, 5.5, 33)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin1Available2 = false;
				coinsCollected++;
			}
			if (coin2Available2 && overlap(10.1, 17, 14.1, 13)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin2Available2 = false;
				coinsCollected++;
			}
			if (coin3Available2 && overlap(18.7, 47, 22.7, 43)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin3Available2 = false;
				coinsCollected++;
			}
			if (coin4Available2 && overlap(27.3, 27, 31.3, 23)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin4Available2 = false;
				coinsCollected++;
			}
			if (coin5Available2 && overlap(35.9, 42, 39.9, 38)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin5Available2 = false;
				coinsCollected++;
			}
			if (coin6Available2 && overlap(44.5, 7, 48.5, 3)) {
				PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
				coin6Available2 = false;
				coinsCollected++;
			}
			if (powerUp1Available && firstPowerUpRandom == 0 && overlap(1.5, 7, 5.5, 3)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 1 && overlap(10.1, 31, 14.1, 27)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 2 && overlap(18.7, 24, 22.7, 20)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 3 && overlap(27.3, 12, 31.3, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 4 && overlap(35.9, 32, 39.9, 28)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp1Available && firstPowerUpRandom == 5 && overlap(44.5, 27, 48.5, 23)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				speed = 1;
				powerUp1Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 0 && overlap(1.5, 12, 5.5, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 1 && overlap(10.1, 42, 14.1, 38)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 2 && overlap(18.7, 17, 22.7, 13)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 3 && overlap(27.3, 42, 31.3, 38)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 4 && overlap(35.9, 12, 39.9, 8)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (powerUp2Available && secondPowerUpRandom == 5 && overlap(44.5, 37, 48.5, 33)) {
				PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveZ2 = -41;
				moveX2 = -35;
				powerUp2Available = false;
			}
			if (won == 0 && overlap(1, 49, 5, 45)) {
				PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
				won = 1;
			}
		}
	}
	glutPostRedisplay();  //redisplay to update the screen with the changes
}

void time1(int val) {
	if (won == 0) {
		timeRemaining--;
		if (timeRemaining == 0) {
			PlaySound((LPCSTR)"lose.wav", NULL, SND_ASYNC | SND_FILENAME);
			gameOver = true;
		}
		glutPostRedisplay();// redraw
	}
	glutTimerFunc(1000, time1, 0);
}

void timeRotation(int val) {
	rot++;

	glutPostRedisplay();// redraw
	glutTimerFunc(1, timeRotation, 0);
}


void AnimTimer(int val)
{
	bool overlapping1 = false;
	bool overlapping2 = false;
	bool overlapping3 = false;
	bool overlapping4 = false;
	bool overlapping5 = false;
	// first wall gaps
	overlapping1 = overlapping1 || overlap(41.4, 15, 43, 10);
	overlapping1 = overlapping1 || overlap(41.4, 27, 43, 22);
	overlapping1 = overlapping1 || overlap(41.4, 42, 43, 37);
	// second wall gaps
	overlapping2 = overlapping2 || overlap(32.8, 15, 34.4, 10);
	overlapping2 = overlapping2 || overlap(32.8, 27, 34.4, 22);
	overlapping2 = overlapping2 || overlap(32.8, 42, 34.4, 37);
	// third wall gaps
	overlapping3 = overlapping3 || overlap(24.2, 15, 25.8, 10);
	overlapping3 = overlapping3 || overlap(24.2, 27, 25.8, 22);
	overlapping3 = overlapping3 || overlap(24.2, 42, 25.8, 37);
	// fourth wall gaps
	overlapping4 = overlapping4 || overlap(15.6, 15, 17.2, 10);
	overlapping4 = overlapping4 || overlap(15.6, 27, 17.2, 22);
	overlapping4 = overlapping4 || overlap(15.6, 42, 17.2, 37);
	// fifth wall gaps
	overlapping5 = overlapping5 || overlap(7, 15, 8.6, 10);
	overlapping5 = overlapping5 || overlap(7, 27, 8.6, 22);
	overlapping5 = overlapping5 || overlap(7, 42, 8.6, 37);

	firstRandom = overlapping5 ? firstRandom : (rand() % 3);
	secondRandom = overlapping4 ? secondRandom : (rand() % 3);
	thirdRandom = overlapping3 ? thirdRandom : (rand() % 3);
	fourthRandom = overlapping2 ? fourthRandom : (rand() % 3);
	fifthRandom = overlapping1 ? fifthRandom : (rand() % 3);
	firstPowerUpRandom = rand() % 6;
	secondPowerUpRandom = rand() % 6;

	glutPostRedisplay();
	glutTimerFunc(10000, AnimTimer, 0);
}

void mouseClick(int btn, int state, int x, int y) {
	bool collision = false;
	if (state == GLUT_DOWN) {
		if (level == 1) {
			switch (btn) {
			case GLUT_LEFT_BUTTON:
				PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveX -= 0.01;
				if (overlap(0, 0.06, 1, 0)) { // left fence
					collision = true;
				}
				if (overlap(0.67, 0.54, 0.73, 0.06) || overlap(0.67, 0.94, 0.73, 0.74)) { // first wall
					collision = true;
				}
				if (overlap(0.47, 0.16, 0.53, 0.06) || overlap(0.47, 0.94, 0.53, 0.36)) { // second wall
					collision = true;
				}
				if (overlap(0.27, 0.6, 0.33, 0.06) || overlap(0.27, 0.94, 0.33, 0.8)) { // third wall
					collision = true;
				}
				if (collision) {
					PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveX += 0.01;
				}
				if (coin1Available && overlap(0.75, 0.15, 0.85, 0.05)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin1Available = false;
					coinsCollected++;
				}
				if (coin2Available && overlap(0.55, 0.85, 0.65, 0.75)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin2Available = false;
					coinsCollected++;
				}
				if (coin3Available && overlap(0.35, 0.45, 0.45, 0.35)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin3Available = false;
					coinsCollected++;
				}
				if (coin4Available && overlap(0.1, 0.9, 0.2, 0.8)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin4Available = false;
					coinsCollected++;
				}
				if (won == 0 && overlap(0.04, 0.23, 0.1, 0.17)) {
					PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
					won = 1;
				}
				break;
			case GLUT_RIGHT_BUTTON:
				PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveX += 0.01;
				if (overlap(0, 1, 1, 0.94)) { // right fence
					collision = true;
				}
				if (overlap(0.67, 0.54, 0.73, 0.06) || overlap(0.67, 0.94, 0.73, 0.74)) { // first wall
					collision = true;
				}
				if (overlap(0.47, 0.16, 0.53, 0.06) || overlap(0.47, 0.94, 0.53, 0.36)) { // second wall
					collision = true;
				}
				if (overlap(0.27, 0.6, 0.33, 0.06) || overlap(0.27, 0.94, 0.33, 0.8)) { // third wall
					collision = true;
				}
				if (collision) {
					PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveX -= 0.01;
				}
				if (coin1Available && overlap(0.75, 0.15, 0.85, 0.05)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin1Available = false;
					coinsCollected++;
				}
				if (coin2Available && overlap(0.55, 0.85, 0.65, 0.75)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin2Available = false;
					coinsCollected++;
				}
				if (coin3Available && overlap(0.35, 0.45, 0.45, 0.35)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin3Available = false;
					coinsCollected++;
				}
				if (coin4Available && overlap(0.1, 0.9, 0.2, 0.8)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin4Available = false;
					coinsCollected++;
				}
				if (won == 0 && overlap(0.04, 0.23, 0.1, 0.17)) {
					PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
					won = 1;
				}
				break;
			}
		}
		else {
			switch (btn) {
			case GLUT_LEFT_BUTTON:
				PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveX2 -= speed;
				if (overlap(0, 0, 50, -1)) { // left fence
					collision = true;
				}
				if (fifthRandom == 0 && (overlap(41.4, 10, 43, 0) || overlap(41.4, 50, 43, 15))) { // first wall (gap left)
					collision = true;
				}
				if (fifthRandom == 1 && (overlap(41.4, 22, 43, 0) || overlap(41.4, 50, 43, 27))) { // first wall (gap middle)
					collision = true;
				}
				if (fifthRandom == 2 && (overlap(41.4, 37, 43, 0) || overlap(41.4, 50, 43, 42))) { // first wall (gap right)
					collision = true;
				}
				if (fourthRandom == 0 && (overlap(32.8, 10, 34.4, 0) || overlap(32.8, 50, 34.4, 15))) { // second wall (gap left)
					collision = true;
				}
				if (fourthRandom == 1 && (overlap(32.8, 22, 34.4, 0) || overlap(32.8, 50, 34.4, 27))) { // second wall (gap middle)
					collision = true;
				}
				if (fourthRandom == 2 && (overlap(32.8, 37, 34.4, 0) || overlap(32.8, 50, 34.4, 42))) { // second wall (gap right)
					collision = true;
				}
				if (thirdRandom == 0 && (overlap(24.2, 10, 25.8, 0) || overlap(24.2, 50, 25.8, 15))) { // third wall (gap left)
					collision = true;
				}
				if (thirdRandom == 1 && (overlap(24.2, 22, 25.8, 0) || overlap(24.2, 50, 25.8, 27))) { // third wall (gap middle)
					collision = true;
				}
				if (thirdRandom == 2 && (overlap(24.2, 37, 25.8, 0) || overlap(24.2, 50, 25.8, 42))) { // third wall (gap right)
					collision = true;
				}
				if (secondRandom == 0 && (overlap(15.6, 10, 17.2, 0) || overlap(15.6, 50, 17.2, 15))) { // fourth wall (gap left)
					collision = true;
				}
				if (secondRandom == 1 && (overlap(15.6, 22, 17.2, 0) || overlap(15.6, 50, 17.2, 27))) { // fourth wall (gap middle)
					collision = true;
				}
				if (secondRandom == 2 && (overlap(15.6, 37, 17.2, 0) || overlap(15.6, 50, 17.2, 42))) { // fourth wall (gap right)
					collision = true;
				}
				if (firstRandom == 0 && (overlap(7, 10, 8.6, 0) || overlap(7, 50, 8.6, 15))) { // fifth wall (gap left)
					collision = true;
				}
				if (firstRandom == 1 && (overlap(7, 22, 8.6, 0) || overlap(7, 50, 8.6, 27))) { // fifth wall (gap middle)
					collision = true;
				}
				if (firstRandom == 2 && (overlap(7, 37, 8.6, 0) || overlap(7, 50, 8.6, 42))) { // fifth wall (gap right)
					collision = true;
				}
				if (collision) {
					PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveX2 += speed;
				}
				if (coin1Available2 && overlap(1.5, 37, 5.5, 33)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin1Available2 = false;
					coinsCollected++;
				}
				if (coin2Available2 && overlap(10.1, 17, 14.1, 13)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin2Available2 = false;
					coinsCollected++;
				}
				if (coin3Available2 && overlap(18.7, 47, 22.7, 43)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin3Available2 = false;
					coinsCollected++;
				}
				if (coin4Available2 && overlap(27.3, 27, 31.3, 23)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin4Available2 = false;
					coinsCollected++;
				}
				if (coin5Available2 && overlap(35.9, 42, 39.9, 38)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin5Available2 = false;
					coinsCollected++;
				}
				if (coin6Available2 && overlap(44.5, 7, 48.5, 3)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin6Available2 = false;
					coinsCollected++;
				}
				if (powerUp1Available && firstPowerUpRandom == 0 && overlap(1.5, 7, 5.5, 3)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 1 && overlap(10.1, 31, 14.1, 27)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 2 && overlap(18.7, 24, 22.7, 20)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 3 && overlap(27.3, 12, 31.3, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 4 && overlap(35.9, 32, 39.9, 28)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 5 && overlap(44.5, 27, 48.5, 23)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 0 && overlap(1.5, 12, 5.5, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 1 && overlap(10.1, 42, 14.1, 38)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 2 && overlap(18.7, 17, 22.7, 13)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 3 && overlap(27.3, 42, 31.3, 38)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 4 && overlap(35.9, 12, 39.9, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 5 && overlap(44.5, 37, 48.5, 33)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (won == 0 && overlap(1, 49, 5, 45)) {
					PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
					won = 1;
				}
				if (firstView) {
					camera.eye.x = 41.2 + moveX2;
					camera.eye.z = 44 + moveZ2;
					camera.center.x = 41.2 + moveX2;
					camera.center.z = 40 + moveZ2;
				}
				if (thirdView) {
					camera.eye.x = 41.2 + moveX2;
					camera.eye.z = 49.5 + moveZ2;
					camera.center.x = 41.2 + moveX2;
					camera.center.z = 45.5 + moveZ2;
				}
				break;
			case GLUT_RIGHT_BUTTON:
				PlaySound((LPCSTR)"movement.wav", NULL, SND_ASYNC | SND_FILENAME);
				moveX2 += speed;
				if (overlap(0, 51, 50, 50)) { // right fence
					collision = true;
				}
				if (fifthRandom == 0 && (overlap(41.4, 10, 43, 0) || overlap(41.4, 50, 43, 15))) { // first wall (gap left)
					collision = true;
				}
				if (fifthRandom == 1 && (overlap(41.4, 22, 43, 0) || overlap(41.4, 50, 43, 27))) { // first wall (gap middle)
					collision = true;
				}
				if (fifthRandom == 2 && (overlap(41.4, 37, 43, 0) || overlap(41.4, 50, 43, 42))) { // first wall (gap right)
					collision = true;
				}
				if (fourthRandom == 0 && (overlap(32.8, 10, 34.4, 0) || overlap(32.8, 50, 34.4, 15))) { // second wall (gap left)
					collision = true;
				}
				if (fourthRandom == 1 && (overlap(32.8, 22, 34.4, 0) || overlap(32.8, 50, 34.4, 27))) { // second wall (gap middle)
					collision = true;
				}
				if (fourthRandom == 2 && (overlap(32.8, 37, 34.4, 0) || overlap(32.8, 50, 34.4, 42))) { // second wall (gap right)
					collision = true;
				}
				if (thirdRandom == 0 && (overlap(24.2, 10, 25.8, 0) || overlap(24.2, 50, 25.8, 15))) { // third wall (gap left)
					collision = true;
				}
				if (thirdRandom == 1 && (overlap(24.2, 22, 25.8, 0) || overlap(24.2, 50, 25.8, 27))) { // third wall (gap middle)
					collision = true;
				}
				if (thirdRandom == 2 && (overlap(24.2, 37, 25.8, 0) || overlap(24.2, 50, 25.8, 42))) { // third wall (gap right)
					collision = true;
				}
				if (secondRandom == 0 && (overlap(15.6, 10, 17.2, 0) || overlap(15.6, 50, 17.2, 15))) { // fourth wall (gap left)
					collision = true;
				}
				if (secondRandom == 1 && (overlap(15.6, 22, 17.2, 0) || overlap(15.6, 50, 17.2, 27))) { // fourth wall (gap middle)
					collision = true;
				}
				if (secondRandom == 2 && (overlap(15.6, 37, 17.2, 0) || overlap(15.6, 50, 17.2, 42))) { // fourth wall (gap right)
					collision = true;
				}
				if (firstRandom == 0 && (overlap(7, 10, 8.6, 0) || overlap(7, 50, 8.6, 15))) { // fifth wall (gap left)
					collision = true;
				}
				if (firstRandom == 1 && (overlap(7, 22, 8.6, 0) || overlap(7, 50, 8.6, 27))) { // fifth wall (gap middle)
					collision = true;
				}
				if (firstRandom == 2 && (overlap(7, 37, 8.6, 0) || overlap(7, 50, 8.6, 42))) { // fifth wall (gap right)
					collision = true;
				}
				if (collision) {
					PlaySound((LPCSTR)"collision.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveX2 -= speed;
				}
				if (coin1Available2 && overlap(1.5, 37, 5.5, 33)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin1Available2 = false;
					coinsCollected++;
				}
				if (coin2Available2 && overlap(10.1, 17, 14.1, 13)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin2Available2 = false;
					coinsCollected++;
				}
				if (coin3Available2 && overlap(18.7, 47, 22.7, 43)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin3Available2 = false;
					coinsCollected++;
				}
				if (coin4Available2 && overlap(27.3, 27, 31.3, 23)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin4Available2 = false;
					coinsCollected++;
				}
				if (coin5Available2 && overlap(35.9, 42, 39.9, 38)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin5Available2 = false;
					coinsCollected++;
				}
				if (coin6Available2 && overlap(44.5, 7, 48.5, 3)) {
					PlaySound((LPCSTR)"coin.wav", NULL, SND_ASYNC | SND_FILENAME);
					coin6Available2 = false;
					coinsCollected++;
				}
				if (powerUp1Available && firstPowerUpRandom == 0 && overlap(1.5, 7, 5.5, 3)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 1 && overlap(10.1, 31, 14.1, 27)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 2 && overlap(18.7, 24, 22.7, 20)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 3 && overlap(27.3, 12, 31.3, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 4 && overlap(35.9, 32, 39.9, 28)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp1Available && firstPowerUpRandom == 5 && overlap(44.5, 27, 48.5, 23)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					speed = 1;
					powerUp1Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 0 && overlap(1.5, 12, 5.5, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 1 && overlap(10.1, 42, 14.1, 38)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 2 && overlap(18.7, 17, 22.7, 13)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 3 && overlap(27.3, 42, 31.3, 38)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 4 && overlap(35.9, 12, 39.9, 8)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (powerUp2Available && secondPowerUpRandom == 5 && overlap(44.5, 37, 48.5, 33)) {
					PlaySound((LPCSTR)"powerUp.wav", NULL, SND_ASYNC | SND_FILENAME);
					moveZ2 = -41;
					moveX2 = -35;
					powerUp2Available = false;
				}
				if (won == 0 && overlap(1, 49, 5, 45)) {
					PlaySound((LPCSTR)"win.wav", NULL, SND_ASYNC | SND_FILENAME);
					won = 1;
				}
				if (firstView) {
					camera.eye.x = 41.2 + moveX2;
					camera.eye.z = 44 + moveZ2;
					camera.center.x = 41.2 + moveX2;
					camera.center.z = 40 + moveZ2;
				}
				if (thirdView) {
					camera.eye.x = 41.2 + moveX2;
					camera.eye.z = 49.5 + moveZ2;
					camera.center.x = 41.2 + moveX2;
					camera.center.z = 45.5 + moveZ2;
				}
				break;
			}
		}
	}
	glutPostRedisplay();
}

int main(int argc, char** argv) {
	srand(time(0));
	glutInit(&argc, argv);

	glutInitWindowSize(640, 480);
	glutInitWindowPosition(50, 50);

	glutCreateWindow("Maze runner");
	glutDisplayFunc(Display);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glutSpecialFunc(spe);            //call the keyboard special keys function
	glutKeyboardFunc(key);            //call the keyboard function
	glutMouseFunc(mouseClick);
	glutTimerFunc(0, time1, 0);        //call the main timer function
	glutTimerFunc(0, timeRotation, 0);        //call the timer function for rotation
	glutTimerFunc(0, AnimTimer, 0); //For random gates and powerups
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	tex_fence.Load("Textures/wood.bmp");
	tex_wall.Load("Textures/wall.bmp");
	tex_wall2.Load("Textures/wall2.bmp");
	tex_fence2.Load("Textures/fence.bmp");
	tex_goal2.Load("Textures/goal.bmp");
	tex_ground.Load("Textures/grass.bmp");
	model_player.Load("Models/player/test.3DS");

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}
