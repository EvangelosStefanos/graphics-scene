#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

#define PI 3.14159265

GLfloat normalVectorsSquare[4][3]; // normal vectors of a square
GLfloat normalVectorsTriangle[3][3]; // normal vectors of a triangle
GLfloat sun_ambient[] = { 0.0, 0.0, 0.0, 1.0 },
sun_diffuse[] = { 0.3, 0.3, 0.3, 1.0 },
sun_specular[] = { 0.3, 0.3, 0.3, 1.0 },
sun_position[] = { -50.0, 0.0, 0.0, 1.0 };

GLfloat sphereSpecular[] = { 0.0, 0.0, 0.0, 1.0 },
sphereAmbientAndDiffuse[] = { 0.0, 0.0, 0.0, 1.0 },
sphereEmission[] = { 1.0, 0.4, 0.0, 0.0 }; // initially the color of the sphere is orange

GLfloat shinySpecular[] = { 1.0, 1.0, 1.0, 1.0 },
shinyAmbientAndDiffuse[] = { 0.4, 0.4, 0.4, 1.0 },
shinyShininess[] = { 128.0 };

GLfloat matSpecular[] = { 0.0, 0.0, 0.0 , 1.0 },
matAmbientAndDiffuse[] = { 0.35, 0.24, 0.20 , 1.0 };

GLfloat grassSpecular[] = { 0.0, 0.0, 0.0 , 1.0 },
grassAmbientAndDiffuse[] = { 0.0, 1.0, 0.20 , 1.0 };

GLfloat cameraX = 0.0,
cameraY = 0.0,
cameraZ = 0.0;

GLfloat spotlight_Ambient[] = { 0.0 , 0.0 , 0.0 , 1.0 },
spotlight_Diffuse[] = { 1.0 , 1.0 , 1.0 , 1.0 },
spotlight_Specular[] = { 1.0 , 1.0 , 1.0 , 1.0 },
spotlight_Position[] = { 5.0 , 5.0 , 11.0 , 1.0 },
spotlight_SpotCutoff = { 30 },
spotlight_SpotDirection[] = { 0.0 , -1.0 , 1.0 };


double rangle = 0.0; // camera rotation angle
double sunRangle = 0.0; // sun rotation angle
int grassX = 1;
int grassY = 1;
GLfloat shadowPlane[4];
GLfloat shadowMatrix[4][4];

/* Normalize a vector. */
void normalize(float v[3]) {
	GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (d == 0.0) {
		printf("zero length vector");
		return;
	}
	v[0] /= d; v[1] /= d; v[2] /= d;
}

/* Calculate the normal vector of the vertices given. */
void normcrossprod(float v1[3], float v2[3], float out[3])
{
	out[0] = v1[1] * v2[2] - v1[2] * v2[1];
	out[1] = v1[2] * v2[0] - v1[0] * v2[2];
	out[2] = v1[0] * v2[1] - v1[1] * v2[0];
	normalize(out);
}

/* Draw a triangle and set its normal vectors.
* Used by subdivide(). */
void drawtriangle(float *v1, float *v2, float *v3)
{
	glBegin(GL_TRIANGLES);
	glNormal3fv(v1);
	glVertex3fv(v1);
	glNormal3fv(v2);
	glVertex3fv(v2);
	glNormal3fv(v3);
	glVertex3fv(v3);
	glEnd();
}

/* Recursive subdivision: Create a sphere via recursive subdivision. */
void subdivide(float *v1, float *v2, float *v3, long depth)
{
	GLfloat v12[3], v23[3], v31[3];
	GLint i;

	if (depth == 0) {
		drawtriangle(v1, v2, v3);
		return;
	}
	for (i = 0; i < 3; i++) {
		v12[i] = v1[i] + v2[i];
		v23[i] = v2[i] + v3[i];
		v31[i] = v3[i] + v1[i];
	}
	normalize(v12);
	normalize(v23);
	normalize(v31);
	subdivide(v1, v12, v31, depth - 1);
	subdivide(v2, v23, v12, depth - 1);
	subdivide(v3, v31, v23, depth - 1);
	subdivide(v12, v23, v31, depth - 1);
}

/* Create a sphere that represents the sun. */
void createSun()
{
	// calculate the color of the sphere
	// as the sphere gains height it's color changes to yellow
	// as the sphere loses height it's color changes to orange
	GLfloat x = 0.0,
		y = 0.0;

	x = (sun_diffuse[0] - 0.3f) / (1.0f - 0.3f); // normalize
	y = 1.2*x <= 1.0 ? 0.4 + 1.2*x : 1.0;

	sphereEmission[1] = y;

	// set the sphere's color
	glMaterialfv(GL_FRONT, GL_SPECULAR, sphereSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sphereAmbientAndDiffuse);
	glMaterialfv(GL_FRONT, GL_EMISSION, sphereEmission);

	// create the sphere
	GLfloat initialPoint[4][3] = { { 0.0, 0.0, 1.0 },{ 0.0, 0.942809, -0.33333 },
	{ -0.816497, -0.471405, -0.333333 },{ 0.816497, -0.471405, -0.333333 } };

	subdivide(initialPoint[0], initialPoint[1], initialPoint[2], 4);
	subdivide(initialPoint[3], initialPoint[2], initialPoint[1], 4);
	subdivide(initialPoint[0], initialPoint[3], initialPoint[1], 4);
	subdivide(initialPoint[0], initialPoint[2], initialPoint[3], 4);

	// reset emission color
	GLfloat noEmission[] = { 0.0, 0.0, 0.0, 0.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
}

/* Create a square, set its normal vectors and draw it. */
void createSquare()
{
	// calculate the square's normal vectors
	float points[4][3] = { { -1, -1, 0 },{ 1, -1, 0 },{ 1, 1, 0 },{ -1, 1, 0 } };
	for (int i = 0; i < 4; i++)
	{
		if (i >= 3)
		{
			normcrossprod(points[i], points[i - 3], normalVectorsSquare[i]);
		}
		else
		{
			normcrossprod(points[i], points[i + 1], normalVectorsSquare[i]);
		}
		//printf("%f %f %f \n", normalVectorsSquare[i][0], normalVectorsSquare[i][1], normalVectorsSquare[i][2]);
	}

	// set normal vectors and draw square
	glNewList(1, GL_COMPILE);
	glBegin(GL_POLYGON); // draw a square
	glNormal3fv(normalVectorsSquare[0]);
	glVertex3f(-1, -1, 0);
	glNormal3fv(normalVectorsSquare[1]);
	glVertex3f(1, -1, 0);
	glNormal3fv(normalVectorsSquare[2]);
	glVertex3f(1, 1, 0);
	glNormal3fv(normalVectorsSquare[3]);
	glVertex3f(-1, 1, 0);
	glEnd();
	glEndList();
}

/* Create a triangle, set its normal vectors and draw it. */
void createTriangle()
{
	// calculate the triangle's normal vectors
	float points[3][3] = { { -1, 0, 0 },{ 1, 0, 0 },{ 0, 1, 0 } };
	for (int i = 0; i < 3; i++)
	{
		if (i >= 2)
		{
			normcrossprod(points[i], points[i - 2], normalVectorsTriangle[i]);
		}
		else
		{
			normcrossprod(points[i], points[i + 1], normalVectorsTriangle[i]);
		}
		//printf("%f %f %f \n", normalVectorsTriangle[i][0], normalVectorsTriangle[i][1], normalVectorsTriangle[i][2]);
	}

	// set normal vectors and draw square
	glNewList(2, GL_COMPILE);
	glBegin(GL_TRIANGLES); // draw a square
	glNormal3fv(normalVectorsTriangle[0]);
	glVertex3f(-1, 0, 0);
	glNormal3fv(normalVectorsTriangle[1]);
	glVertex3f(1, 0, 0);
	glNormal3fv(normalVectorsTriangle[2]);
	glVertex3f(0, 1, 0);
	glEnd();
	glEndList();
}

/* Create a house. */
void createHouse()
{
	glPushMatrix();	// push initial

					// set the color of the house
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbientAndDiffuse);

	// draw house

	// front side
	glTranslatef(0, 0, 10);
	glScalef(5, 5, 1);
	glCallList(1); // draw front side at [-5, 5, -5, 5, 10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// back side
	glTranslatef(0, 0, -10);
	glRotatef(180, 0, 1, 0);
	glScalef(5, 5, 1);
	glCallList(1); // draw back side at [-5, 5, -5, 5, -10, -10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// top side
	glTranslatef(0, 5, 0);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw top side at [-5, 5, 10, 10, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// bottom side
	glTranslatef(0, -5, 0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw bottom side at [-5, 5, -10, -10, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial


					// left side
	glTranslatef(-5, 0, 0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glRotatef(-90, 0.0, 1.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw left side at [-5, -5, -5, 5, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// right side
	glTranslatef(5, 0, 0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glRotatef(90, 0.0, 1.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw right side at [5, 5, -5, 5, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// set the color of the roof
	glMaterialfv(GL_FRONT, GL_SPECULAR, shinySpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shinyAmbientAndDiffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, shinyShininess);

	// draw roof

	// roof left side
	glTranslatef(-5, 5, 0);
	glRotatef(60, 0.0, 0.0, 1.0);
	glTranslatef(5, 0, 0);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw right side at [-5, something, 5, something, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					// roof right side
	glTranslatef(5, 5, 0);
	glRotatef(120, 0.0, 0.0, 1.0);
	glTranslatef(5, 0, 0);
	glRotatef(90, 1.0, 0.0, 0.0);
	glScalef(5, 10, 1);
	glCallList(1); // draw right side at [something, 5, 5, something, -10, 10]
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					//roof front side
	glTranslatef(0, 5, 10);
	glScalef(5, 8.66, 1);
	glCallList(2);
	glPopMatrix(); // pop initial
	glPushMatrix(); // push initial

					//roof back side
	glTranslatef(0, 5, -10);
	glRotatef(180, 0, 1, 0);
	glScalef(5, 8.66, 1);
	glCallList(2);

	glPopMatrix(); // pop initial
}

/* Create a grid of squares with dimensions maxDimensionX x maxDimensionY that represents the grass.
* High parameter values have a performance cost but offer superior quality. */
void createGrassManyPolygons(int maxDimensionX, int maxDimensionY)
{
	// set the color of the grass
	glMaterialfv(GL_FRONT, GL_SPECULAR, grassSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grassAmbientAndDiffuse);

	// create a grid
	GLfloat points[110][110][3]; // array of points

	GLfloat baseValue = -1.0;
	GLfloat halfX = (maxDimensionX + 1) / 2;
	GLfloat halfY = (maxDimensionY + 1) / 2;
	GLfloat squareSide = abs(baseValue) + abs(-baseValue);
	double stepX = squareSide / (double)maxDimensionX;
	double stepY = squareSide / (double)maxDimensionY;


	// calculate the points of the first tetartimorio and mirror the rest
	for (int y = 0; y <= halfY; y++)
	{
		GLfloat valueY = baseValue + stepY*y;

		for (int x = 0; x <= halfX; x++)
		{
			GLfloat valueX = baseValue + stepX*x;

			points[y][x][0] = valueX;
			points[y][x][1] = valueY;
			points[y][x][2] = 0;

			// mirror on y = 0 axis
			if (x != halfX) // don't mirror at the halfway point(x = maxDimensionX - x)
			{
				points[y][maxDimensionX - x][0] = -valueX;
				points[y][maxDimensionX - x][1] = valueY;
				points[y][maxDimensionX - x][2] = 0;
			}

			// mirror on x = 0 axis
			if (y != halfY) // don't mirror at the halfway point(y = maxDimensionY - y)
			{
				points[maxDimensionY - y][x][0] = valueX;
				points[maxDimensionY - y][x][1] = -valueY;
				points[maxDimensionY - y][x][2] = 0;
			}

			// mirror on y = 0 and x = 0 axis
			if (x != halfX || y != halfY) // don't mirror at the halfway point(x = maxDimensionX - x || y = maxDimensionY - y || (x = maxDimensionX - x && y = maxDimensionY - y))
			{
				points[maxDimensionY - y][maxDimensionX - x][0] = -valueX;
				points[maxDimensionY - y][maxDimensionX - x][1] = -valueY;
				points[maxDimensionY - y][maxDimensionX - x][2] = 0;
			}
		}
	}

	// draw the grid
	for (int y = 0; y < maxDimensionY; y++)
	{
		for (int x = 0; x < maxDimensionX; x++)
		{
			glBegin(GL_POLYGON);
			{
				glNormal3f(0, 0, 1);
				glVertex3fv(points[y][x]);

				glNormal3f(0, 0, 1);
				glVertex3fv(points[y][x + 1]);

				glNormal3f(0, 0, 1);
				glVertex3fv(points[y + 1][x + 1]);

				glNormal3f(0, 0, 1);
				glVertex3fv(points[y + 1][x]);
			}
			glEnd();
		}
	}
}

/* Set the sun's new rotation angle and draw the next frame. */
void idle(void)
{
	GLfloat increase = sunRangle < 180 ? 0.01 : 0.10; // rotate faster at night
	if (sunRangle > 359)
	{
		sunRangle = 0;
	}
	if (grassX >= 100)
	{
		sunRangle += 2 * increase;
	}
	else
	{
		sunRangle += increase;
	}
	glutPostRedisplay();
}

/* Calculate the color of the sun's light. */
void sunIntensity()
{
	if (sunRangle > 180)
	{
		return;
	}
	GLfloat x = 0.0;
	GLfloat y = 0.0;

	if (sunRangle <= 90)
	{
		x = sunRangle / 90.0;
		y = 0.7*x + 0.3;
	}
	else if (sunRangle > 90)
	{
		x = (180.0 - sunRangle) / 90.0;
		y = 0.7*x + 0.3;
	}

	for (int i = 0; i < 3; i++)
	{
		sun_diffuse[i] = y;
		sun_specular[i] = y;
	}

	// set the new color of the sun
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, sun_specular);
}

/* Find the plane equation given 3 points. */
void findPlane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3])
{
	GLfloat vec0[3], vec1[3];
	int X = 0, Y = 1, Z = 2; // local indexes
	int A = 0, B = 1, C = 2, D = 3;

	/* Need 2 vectors to find cross product. */
	vec0[X] = v1[X] - v0[X];
	vec0[Y] = v1[Y] - v0[Y];
	vec0[Z] = v1[Z] - v0[Z];

	vec1[X] = v2[X] - v0[X];
	vec1[Y] = v2[Y] - v0[Y];
	vec1[Z] = v2[Z] - v0[Z];

	/* find cross product to get A, B, and C of plane equation */
	plane[A] = vec0[Y] * vec1[Z] - vec0[Z] * vec1[Y];
	plane[B] = -(vec0[X] * vec1[Z] - vec0[Z] * vec1[X]);
	plane[C] = vec0[X] * vec1[Y] - vec0[Y] * vec1[X];

	plane[D] = -(plane[A] * v0[X] + plane[B] * v0[Y] + plane[C] * v0[Z]);
}

/* create a matrix that will project the desired shadow */
void newShadowMatrix(GLfloat shadowMat[4][4],
					 GLfloat groundplane[4],
					 GLfloat lightpos[4])
{
	int X = 0, Y = 1, Z = 2, W = 3;
	GLfloat dot;

	/* find dot product between light position vector and ground plane normal */
	dot = groundplane[X] * lightpos[X] +
		groundplane[Y] * lightpos[Y] +
		groundplane[Z] * lightpos[Z] +
		groundplane[W] * lightpos[W];

	shadowMat[0][0] = dot - lightpos[X] * groundplane[X];
	shadowMat[1][0] = 0.f - lightpos[X] * groundplane[Y];
	shadowMat[2][0] = 0.f - lightpos[X] * groundplane[Z];
	shadowMat[3][0] = 0.f - lightpos[X] * groundplane[W];

	shadowMat[X][1] = 0.f - lightpos[Y] * groundplane[X];
	shadowMat[1][1] = dot - lightpos[Y] * groundplane[Y];
	shadowMat[2][1] = 0.f - lightpos[Y] * groundplane[Z];
	shadowMat[3][1] = 0.f - lightpos[Y] * groundplane[W];

	shadowMat[X][2] = 0.f - lightpos[Z] * groundplane[X];
	shadowMat[1][2] = 0.f - lightpos[Z] * groundplane[Y];
	shadowMat[2][2] = dot - lightpos[Z] * groundplane[Z];
	shadowMat[3][2] = 0.f - lightpos[Z] * groundplane[W];

	shadowMat[X][3] = 0.f - lightpos[W] * groundplane[X];
	shadowMat[1][3] = 0.f - lightpos[W] * groundplane[Y];
	shadowMat[2][3] = 0.f - lightpos[W] * groundplane[Z];
	shadowMat[3][3] = dot - lightpos[W] * groundplane[W];
}

/* Creates the shadow of the house. */
void houseShadow()
{
	// house shadow
	if (sunRangle > 180)
	{
		return;
	}

	// calculate shadow matrix
	newShadowMatrix(shadowMatrix, shadowPlane, sun_position);

	// calculate shadow color
	// the shadow color becomes darker when the sunlight becomes brighter
	GLfloat x = 0.0,
		y = 0.0;

	x = (sun_diffuse[0] - 0.3f) / (1.0 - 0.3f); // normalize
	y = x*1.1 < 0.8 ? 1.1*x : 0.8;

	// draw shadow

	// render only when stencil is greater than 2
	// when rendered update stencil to 2
	glStencilFunc(GL_LESS, 2, 0xffffffff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0, 0.0, 0.0, y);

	glPushMatrix(); // push initial
	glMultMatrixf((const GLfloat*)shadowMatrix);
	createHouse();
	glPopMatrix(); // pop initial

	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  /*clear the window */
	glLoadIdentity();

	// calculate new position of the sun
	sun_position[0] = -50.0 * cos(sunRangle*(PI / 180));
	sun_position[1] = 50.0 * sin(sunRangle*(PI / 180));
	sun_position[2] = 0;
	sun_position[3] = 1;

	// set the new position of the camera
	glPushMatrix(); // push initial
	cameraX = 70.0*sin(rangle*(PI / 180)) + 0.0*cos(rangle*(PI / 180)),
		cameraY = 40.0,
		cameraZ = 70.0*cos(rangle*(PI / 180)) - 0.0*sin(rangle*(PI / 180));

	gluLookAt(cameraX, 40.0, cameraZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // eye at [0, 40, 70] looking at [0, 0, 0] up vector [0, 1.0, 0.0]

																	 // light source and sphere
	glPushMatrix(); // push lookAt
	glLightfv(GL_LIGHT0, GL_POSITION, sun_position); // position the light source
	glRotatef(-sunRangle, 0, 0, 1); // rotate the sphere
	glTranslatef(-50, 0, 0);
	createSun(); // draw the sphere
	sunIntensity(); // set the sun's intensity
	glPopMatrix(); // pop lookAt

				   // spotlight
	glLightfv(GL_LIGHT1, GL_POSITION, spotlight_Position); // position the spotlight
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotlight_SpotDirection);

	// house
	createHouse(); // draw house

				   // grass
				   // draw the grass with stencil value 3
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 3, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glPushMatrix(); // push lookAt
	glTranslatef(0, -5, 0);
	glRotatef(-90, 1.0, 0.0, 0.0);
	glScalef(50, 50, 1);
	createGrassManyPolygons(grassX, grassY); // draw grass
	glPopMatrix(); // pop lookAt

				   // draw the shadow of the house
	glPushMatrix(); // push lookAt
	houseShadow();
	glPopMatrix(); // pop lookAt

	glPopMatrix(); // pop initial


	glutSwapBuffers();
	glFlush(); /* clear buffers */
}

void myinit(void)
{
	/* attributes */
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glClearColor(1.0, 1.0, 1.0, 0.0); /* white background */
	glColor3f(1.0, 0.0, 0.0); /* draw in red */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-20.0, 20.0, -20.0, 20.0, 20.0, 160.0);
	//glOrtho(-60, 60, -60, 60, -60, 60);
	glMatrixMode(GL_MODELVIEW);
	createSquare();
	createTriangle();

	// set the properties of the sun
	glLightfv(GL_LIGHT0, GL_AMBIENT, sun_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, sun_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, sun_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);
	glEnable(GL_LIGHT0);

	// set the properties of the spotlight
	glLightfv(GL_LIGHT1, GL_AMBIENT, spotlight_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, spotlight_Diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spotlight_Specular);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotlight_SpotCutoff);

	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);
	glEnable(GL_LIGHT1);

	// calculate the shadow plane
	GLfloat planePoints[3][3] = { { -20, -5, -20 },{ 20, -5, -20 },{ 20, -5, 20 } };
	findPlane(shadowPlane, planePoints[2], planePoints[1], planePoints[0]);
}

/* Handles menu events. */
void menuCallback(int value)
{
	switch (value)
	{
		case 1:
			grassX = 1;
			grassY = 1;
			break;
		case 2:
			grassX = 10;
			grassY = 10;
			break;
		case 3:
			// high performance cost but superior quality
			grassX = 100;
			grassY = 100;
			break;
		case 4:
			glEnable(GL_LIGHT1);
			break;
		case 5:
			glDisable(GL_LIGHT1);
			break;
		case 6:
			glShadeModel(GL_FLAT);
			break;
		case 7:
			glShadeModel(GL_SMOOTH);
			break;
	}
	glutPostRedisplay();
}

/* Set the new camera rotation angle and draw the next frame.
* Character 'a' rotates right and character 'd' rotates left. */
void keyboardCallback(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'a':
			rangle -= 1;
			break;
		case 'd':
			rangle += 1;
			break;
	}
	glutPostRedisplay();
}

void main(int argc, char** argv)
{
	/* Standard GLUT initialization */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL); /* not default, needed */
	glutInitWindowSize(500, 500); /* 500 x 500 pixel window */
	glutInitWindowPosition(0, 0); /* place window top left on display */
	glutCreateWindow("Little house in the field"); /* window title */
	glutDisplayFunc(display); /* display callback invoked when window opened */
	glutIdleFunc(idle);

	glutCreateMenu(menuCallback);
	glutAddMenuEntry("Grass 1 polygon", 1);
	glutAddMenuEntry("Grass 100 polygons", 2);
	glutAddMenuEntry("Grass 10000 polygons", 3);
	glutAddMenuEntry("Activate Spotlight", 4);
	glutAddMenuEntry("Deactivate Spotlight", 5);
	glutAddMenuEntry("Activate flat shading", 6);
	glutAddMenuEntry("Activate smooth shading", 7);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutKeyboardFunc(keyboardCallback);

	myinit(); /* set attributes */

	glutMainLoop(); /* enter event loop */
}

