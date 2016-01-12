//#pragma comment(linker, "/subsystem:\"windows\"   /entry:\"mainCRTStartup\"")

#include "stdafx.h"
#include "glut.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "openglConst.h"
#include "Constants.h"
#include "Game.h"

Game *game;

GLfloat look_eye_dis;
GLfloat look_eye[3]; // 视点位置
GLfloat look_up[3]; // 视点法向
GLfloat look_center[3]; // 相机中点
GLfloat lookAngle;

GLfloat debugX = 0, debugY = 0;
int mouseXrec = 0, mouseYrec = 0;

void Setup(); // 初始化设置openGL及各变量
void Display(); // 显示
void Reshape(int w, int h); // 窗口大小调整

void LookPosMaintain(GLfloat delta, GLfloat shift); // 调整视角和镜头位置
bool GetOGLPos(int x, int y, GLdouble &rx, GLdouble &ry); // 映射鼠标坐标至openGL坐标

void OnMouseMove(int x, int y); // 捕捉鼠标移动
void Mouse(int button, int state, int x, int y); // 捕捉鼠标按键信息

void Keyboard(unsigned char key, int x, int y); // 捕捉基本按键信息
void SpecialKeys(int key, int x, int y); // 捕捉特殊按键信息

void Update(int value); // 间隔刷新函数


int main(int argc, char **argv)
{
	game = new Game();

	/* init opengl & creat window */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(MYGL_WINDOW_W, MYGL_WINDOW_H);
	glutInitWindowPosition(MYGL_WINDOW_POS_X, MYGL_WINDOW_POS_Y);
	glutCreateWindow(MY_GL_WINDOW_NAME);
	Setup();

	/* register function */
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(OnMouseMove);

	glutTimerFunc(GAME_DELTA_TIME, Update, 0);

	/* main loop */
	glutMainLoop();
	return 0;
}


void Setup()
{
	// light on
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// light setup
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	// material setup
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emission);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	// other init
	lookAngle = look_init_angle;
	look_eye_dis = look_eye_dis_init;
	LookPosMaintain(0, 0);
}

void LookPosMaintain(GLfloat delta, GLfloat shift)
{
	lookAngle += delta;
	if (lookAngle < 0.01) lookAngle += 360;
	if (lookAngle > 359.99) lookAngle -= 360;
	look_eye_dis += shift;
	if (look_eye_dis < look_eye_dis_min) look_eye_dis = look_eye_dis_min;
	if (look_eye_dis > look_eye_dis_max) look_eye_dis = look_eye_dis_max;
	// 视点位置
	look_eye[0] = look_eye_dis * cos(look_dep_angle * PI / 180) * cos(lookAngle * PI / 180);
	look_eye[1] = look_eye_dis * cos(look_dep_angle * PI / 180) * sin(lookAngle * PI / 180);
	look_eye[2] = look_eye_dis * sin(look_dep_angle * PI / 180);
	// 视点法向
	look_up[0] = look_eye_dis * sin(look_dep_angle * PI / 180) * -cos(lookAngle * PI / 180);
	look_up[1] = look_eye_dis * sin(look_dep_angle * PI / 180) * -sin(lookAngle * PI / 180);
	look_up[2] = look_eye_dis * cos(look_dep_angle * PI / 180);
	// 相机中点
	GLfloat tmp = look_eye_dis - look_scn_dis;
	look_center[0] = tmp * cos(look_dep_angle * PI / 180) * cos(lookAngle * PI / 180);
	look_center[1] = tmp * cos(look_dep_angle * PI / 180) * sin(lookAngle * PI / 180);
	look_center[2] = tmp * sin(look_dep_angle * PI / 180);
	//look_center[0] = 0, look_center[1] = 0, look_center[2] = 0;
}

/* 绘制实心圆盘 */
void glutSolidCylinder(GLfloat red, GLfloat green, GLfloat blue,
	GLdouble x, GLdouble y, GLdouble z, GLdouble h, GLdouble r,
	GLint slices, GLint stacks)
{
	glPushMatrix();
	// Cylinder
	glColor3f(red - 0.02, green, blue);
	glTranslatef(x, y, z);
	GLUquadricObj *objCylinder = gluNewQuadric();
	gluCylinder(objCylinder, r, r, h, slices, stacks);
	// top
	glColor3f(red, green, blue);
	glTranslatef(0, 0, h);
	GLUquadricObj *objDisk = gluNewQuadric();
	gluDisk(objDisk, 0, r, slices, 10);
	glPopMatrix();
}

void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(look_eye[0], look_eye[1], look_eye[2],
		look_center[0], look_center[1], look_center[2],
		look_up[0], look_up[1], look_up[2]);

	// floor plane
	{
		glPushMatrix();
		glBegin(GL_QUADS);
		glColor3f(0.7255, 0.3882, 0.0353);
		glNormal3f(0, 0, 1);
		glVertex3f(G_FloorPlaneSize, -G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glVertex3f(G_FloorPlaneSize, G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glVertex3f(-G_FloorPlaneSize, G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glVertex3f(-G_FloorPlaneSize, -G_FloorPlaneSize, -G_TableAltitude);
		glEnd();
		glPopMatrix();
	}

	// table plane
	{
		glPushMatrix();
		glBegin(GL_QUADS);
		//glColor3f(0.4392, 0.7255, 0.3137);
		glColor3f(0.2902, 0.498, 0.2);
		glNormal3f(0, 0, 1);
		glVertex3f(G_tableWidth / 2, -G_tableHeight / 2, 0);
		glNormal3f(0, 0, 1);
		glVertex3f(G_tableWidth / 2, G_tableHeight / 2, 0);
		glNormal3f(0, 0, 1);
		glVertex3f(-G_tableWidth / 2, G_tableHeight / 2, 0);
		glNormal3f(0, 0, 1);
		glVertex3f(-G_tableWidth / 2, -G_tableHeight / 2, 0);
		glEnd();
		glPopMatrix();
	}

	// table edge & side
	{
		glPushMatrix();
		glColor3f(0.93, 0.93, 0.93);
		// long side
		glPushMatrix();
		glTranslatef(0, G_tableHeight / 2 + G_TablePly / 2, -G_TableAltitude / 2 + G_TablePly / 2);
		glScalef(G_tableWidth + 2 * G_TablePly, G_TablePly, G_TableAltitude + G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0, -G_tableHeight / 2 - G_TablePly / 2, -G_TableAltitude / 2 + G_TablePly / 2);
		glScalef(G_tableWidth + 2 * G_TablePly, G_TablePly, G_TableAltitude + G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		// short side
		glPushMatrix();
		glTranslatef(G_tableWidth / 2 + G_TablePly / 2, 0, -G_TableAltitude / 2);
		glScalef(G_TablePly, G_tableHeight, G_TableAltitude);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-G_tableWidth / 2 - G_TablePly / 2, 0, -G_TableAltitude / 2);
		glScalef(G_TablePly, G_tableHeight, G_TableAltitude);
		glutSolidCube(1);
		glPopMatrix();
		// door edge
		glPushMatrix();
		glTranslatef(G_tableWidth / 2 + G_TablePly / 2, (G_tableHeight + G_goalWidth) / 4, G_TablePly / 2);
		glScalef(G_TablePly, (G_tableHeight - G_goalWidth) / 2, G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(G_tableWidth / 2 + G_TablePly / 2, -(G_tableHeight + G_goalWidth) / 4, G_TablePly / 2);
		glScalef(G_TablePly, (G_tableHeight - G_goalWidth) / 2, G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-G_tableWidth / 2 - G_TablePly / 2, (G_tableHeight + G_goalWidth) / 4, G_TablePly / 2);
		glScalef(G_TablePly, (G_tableHeight - G_goalWidth) / 2, G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(-G_tableWidth / 2 - G_TablePly / 2, -(G_tableHeight + G_goalWidth) / 4, G_TablePly / 2);
		glScalef(G_TablePly, (G_tableHeight - G_goalWidth) / 2, G_TablePly);
		glutSolidCube(1);
		glPopMatrix();
		glPopMatrix();
	}

	// puck & mallet
	Point pt;
	pt = game->GetPuckPosition();
	glutSolidCylinder(0.8157, 0.0157, 0.0157, pt.x(), pt.y(), 0, G_PuckHeight, G_PuckDiameter / 2, 32, 1);
	pt = game->GetPlayerMalletPosition();
	glutSolidCylinder(0.1765, 0.1765, 0.9765, pt.x(), pt.y(), 0, G_MalletHeight, G_MalletDiameter / 2, 32, 1);
	pt = game->GetOpponentMalletPosition();
	glutSolidCylinder(0.5686, 0.0039, 0.9490, pt.x(), pt.y(), 0, G_MalletHeight, G_MalletDiameter / 2, 32, 1);

	glFlush();
	glutSwapBuffers();
}

/* 映射鼠标坐标至openGL坐标
* 坐标位置为鼠标指向的物体表面点坐标
* copy from http://blog.csdn.net/augusdi/article/details/38597403
* 当鼠标对应点出界，返回false
*/
bool GetOGLPos(int x, int y, GLdouble &rx, GLdouble &ry)
{
	static GLint viewport[4];
	static GLdouble modelview[16];
	static GLdouble projection[16];
	static GLfloat winX, winY, winZ;
	static GLdouble posX, posY, posZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
	winX = (float)x, winY = (float)viewport[3] - (float)y;
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
	//std::cout << "get " << posX << " " << posY << " " << posZ << std::endl;

	// 第一次出界判定
	if (posX < -G_tableWidth / 2 - 0.01 || posX > G_tableWidth / 2 + 0.01
		|| posY < -G_tableHeight / 2 - 0.01 || posY > G_tableHeight / 2 + 0.01
		|| posZ < -0.01 || posZ > G_MalletHeight + 0.01) {
		// out of table
		//std::cout << "out1" << std::endl;
		return false;
	}
	// 修正点击在边界或圆盘上的情况
	if (posZ > 1e-5 || posZ < -1e-5) {
		// 不在桌面上，需要进行修整
		posX -= posZ / look_eye[2] * look_eye[0];
		posY -= posZ / look_eye[2] * look_eye[1];
		posZ = 0;
	}
	// 第二次出界判定
	if (posX < -G_tableWidth / 2 - 0.001 || posX > G_tableWidth / 2 + 0.001
		|| posY < -G_tableHeight / 2 - 0.001 || posY > G_tableHeight / 2 + 0.001) {
		// out of table
		//std::cout << "out2" << std::endl;
		return false;
	}
	//std::cout << "real " << posX << " " << posY << " " << posZ << std::endl;
	rx = posX, ry = posY;
	return true;
}

/* TODO 修正函数参数
 */
void Reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (GLfloat)w / h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

/* get mouse position
* top-left corner : (0, 0)
* bottom-right corner : (MYGL_WINDOW_W - 1, MYGL_WINDOW_H - 1)
*/
void OnMouseMove(int x, int y)
{
	mouseXrec = x, mouseYrec = y;
}

void Mouse(int button, int state, int x, int y)
{
	//std::cout << state << " " << button << std::endl;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// left mouse btn down
		//std::cout << "left btn down" << std::endl;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		// left mouse btn up
		//std::cout << "left btn down" << std::endl;
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	//std::cout << key << " " << x << " " << y << std::endl;
}

/* 捕获左右键消息：旋转视角
 * 捕获上下键消息：伸缩镜头
 */
void SpecialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		//std::cout << "LEFT" << std::endl;
		LookPosMaintain(-look_delta_angle, 0);
		break;
	case GLUT_KEY_RIGHT:
		//std::cout << "RIGHT" << std::endl;
		LookPosMaintain(look_delta_angle, 0);
		break;
	case GLUT_KEY_UP:
		//std::cout << "UP" << std::endl;
		LookPosMaintain(0, -look_eye_dis_delta);
		break;
	case GLUT_KEY_DOWN:
		//std::cout << "DOWN" << std::endl;
		LookPosMaintain(0, look_eye_dis_delta);
		break;
	}
}

void Update(int value)
{
	/* call game with player mallet position & delta-time
	 * the coord of mallets has to be transed into 2D
	 * the center of plant is (0, 0)
	 */
	GLdouble x, y;
	if (GetOGLPos(mouseXrec, mouseYrec, x, y)) {
		int ret = game->Run(GAME_DELTA_TIME, x, y);
		if (ret) std::cout << "game result " << ret << std::endl;
	}

	/* display */
	glutPostRedisplay();
	/* recursive call */
	glutTimerFunc(GAME_DELTA_TIME, Update, 0);
}