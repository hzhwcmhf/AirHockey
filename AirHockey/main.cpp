//#pragma comment(linker, "/subsystem:\"windows\"   /entry:\"mainCRTStartup\"")

#include "stdafx.h"
#include "glut.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "openglConst.h"
#include "Constants.h"
#include "Game.h"

/* 游戏当前模式 */
enum GameModeType {
	MainWindow, // 主界面
	Gaming, // 游戏界面
	Goal, // 得分界面
	Win, // 胜利界面
	Pause // 暂停界面
};

Game *game; // 游戏逻辑模块
GameModeType gameMode; // 当前游戏模式
int score1, score2; // 双方得分，1玩家，2AI
int flashCount; // 用于主界面和暂停界面的闪烁
int waitingTime; // 每轮游戏开始延迟等待时间剩余
int roundSet; // 胜利所需轮数
int aiLevel; // AI难度
int winner; // 胜利者

GLuint floorTexcture; // 地板贴图编号

GLfloat look_eye_dis; // 视点距离原点距离
GLfloat look_eye[3]; // 视点位置
GLfloat look_up[3]; // 视点法向
GLfloat look_center[3]; // 相机中点
GLfloat lookAngle; // 视角

int mouseXrec, mouseYrec; // 鼠标位置

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
	/* 初始化openGL */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(MYGL_WINDOW_W, MYGL_WINDOW_H);
	glutInitWindowPosition(MYGL_WINDOW_POS_X, MYGL_WINDOW_POS_Y);
	glutCreateWindow(MY_GL_WINDOW_NAME);
	Setup();

	/* 注册回调函数 */
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(OnMouseMove);
	glutMotionFunc(OnMouseMove);

	/* 间隔刷新函数 */
	glutTimerFunc(GAME_DELTA_TIME, Update, 0);

	/* main loop */
	glutMainLoop();
	return 0;
}

/* 使用windowAPI读取bmp图片并创建贴图
 * 读取bmp部分可以使用其他函数代替
 */
bool LoadTexture(LPTSTR szFileName, GLuint &texid)   // Creates Texture From A Bitmap File
{
	HBITMAP hBMP;       // Handle Of The Bitmap
	BITMAP BMP;       // Bitmap Structure
	glGenTextures(1, &texid);      // Create The Texture
	hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
	if (!hBMP)        // Does The Bitmap Exist?
		return FALSE;       // If Not Return False
	GetObject(hBMP, sizeof(BMP), &BMP);     // Get The Object
											// hBMP:    Handle To Graphics Object
											// sizeof(BMP): Size Of Buffer For Object Information
											// &BMP:    Buffer For Object Information
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);    // Pixel Storage Mode (Word Alignment / 4 Bytes)
											  // Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);     // Bind To The Texture ID
											 //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Mag Filter
																	  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
	DeleteObject(hBMP);       // Delete The Object
	return TRUE;       // Loading Was Successful
}

GLuint WordShowLists = 0; // openGL显示字符列表，selectFont选择字体时修改，drawString时使用

/* 设置字体 */
void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_BOLD, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);

	// 申请MAX_CHAR个连续的显示列表编号
	if (WordShowLists == 0)
		WordShowLists = glGenLists(128);
	// 把每个字符的绘制命令都装到对应的显示列表中
	wglUseFontBitmaps(wglGetCurrentDC(), 0, 128, WordShowLists);
}

/* 绘制字符串 */
void drawString(const char* str)
{
	// 调用每个字符对应的显示列表，绘制每个字符
	for (; *str != '\0'; ++str)
		glCallList(WordShowLists + *str);
}


void Setup()
{
	game = new Game();
	gameMode = MainWindow;
	flashCount = 0;
	roundSet = Game_Round_Init;
	aiLevel = Game_AI_INIT;
	mouseXrec = mouseYrec = 0;
	waitingTime = 0;

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

	// texcture
	LoadTexture("floor.bmp", floorTexcture);

	// other init
	lookAngle = look_init_angle;
	look_eye_dis = look_eye_dis_init;
	LookPosMaintain(0, 0);

	glutGet(GLUT_ELAPSED_TIME);
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
	// delete
	gluDeleteQuadric(objDisk);
	gluDeleteQuadric(objCylinder);
	glPopMatrix();
}

/* 绘制曲棍 */
void glutSolidMallet(GLfloat red, GLfloat green, GLfloat blue,
	GLdouble x, GLdouble y, GLdouble z, GLdouble h, GLdouble r,
	GLint slices, GLint stacks)
{
	glPushMatrix();
	// Cylinder
	glColor3f(red - 0.02, green, blue);
	glTranslatef(x, y, z);
	GLUquadricObj *objCylinder = gluNewQuadric();
	gluCylinder(objCylinder, r, r, h, slices, stacks);
	// ball
	glColor3f(red, green, blue);
	glTranslatef(0, 0, h);
	GLUquadricObj *objSphere = gluNewQuadric();
	gluSphere(objSphere, r / 2, slices, slices);
	// top
	glColor3f(red, green, blue);
	GLUquadricObj *objDisk = gluNewQuadric();
	gluDisk(objDisk, 0, r, slices, 10);
	// delete
	gluDeleteQuadric(objSphere);
	gluDeleteQuadric(objDisk);
	gluDeleteQuadric(objCylinder);
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

	// 绘制地面并贴图
	{
		//glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glColor3f(1, 1, 1);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, 0);
		glVertex3f(G_FloorPlaneSize, -G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glTexCoord2f(0, TileCount);
		glVertex3f(G_FloorPlaneSize, G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glTexCoord2f(TileCount, TileCount);
		glVertex3f(-G_FloorPlaneSize, G_FloorPlaneSize, -G_TableAltitude);
		glNormal3f(0, 0, 1);
		glTexCoord2f(TileCount, 0);
		glVertex3f(-G_FloorPlaneSize, -G_FloorPlaneSize, -G_TableAltitude);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		//glPopMatrix();
	}

	// 绘制桌面边缘及桌子边框
	{
		//glPushMatrix();
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
		//glPopMatrix();
	}

	// 绘制桌面
	{
		//glPushMatrix();
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
		glColor3f(0.0863, 0.2549, 0.0118);
		glLineWidth(2);
		glBegin(GL_LINE_STRIP);
		glVertex3f(G_tableWidth * (0.5 - GL_MalletRange), G_tableHeight / 2, 0.005);
		glVertex3f(G_tableWidth * (0.5 - GL_MalletRange), -G_tableHeight / 2, 0.005);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(-G_tableWidth * (0.5 - GL_MalletRange), G_tableHeight / 2, 0.005);
		glVertex3f(-G_tableWidth * (0.5 - GL_MalletRange), -G_tableHeight / 2, 0.005);
		glEnd();
		glBegin(GL_LINE_LOOP);
		double R = GL_KickOffCircle_R * G_tableWidth;
		for (int i = 0; i < 32; ++i)
			glVertex3f(R * cos(2 * PI / 32 * i), R * sin(2 * PI / 32 * i), 0.005);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(0, G_tableHeight / 2, 0.005);
		glVertex3f(0, R, 0.005);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(0, -R, 0.005);
		glVertex3f(0, -G_tableHeight / 2, 0.005);
		glEnd();
		glBegin(GL_TRIANGLE_STRIP);
		R = G_goalWidth / 2 + 0.01;
		for (int i = 8; i <= 24; ++i) {
			glVertex3f(G_tableWidth / 2, 0, 0.005);
			glVertex3f(G_tableWidth / 2 + R * cos(2 * PI / 32 * i), R * sin(2 * PI / 32 * i), 0.005);
		}
		glEnd();
		glBegin(GL_TRIANGLE_STRIP);
		R = G_goalWidth / 2 + 0.01;
		for (int i = -8; i <= 8; ++i) {
			glVertex3f(-G_tableWidth / 2, 0, 0.005);
			glVertex3f(-G_tableWidth / 2 + R * cos(2 * PI / 32 * i), R * sin(2 * PI / 32 * i), 0.005);
		}
		glEnd();
		//glPopMatrix();
	}

	// 绘制冰球及曲棍
	{
		Point pt;
		pt = game->GetPuckPosition();
		glutSolidCylinder(ColorRed[0], ColorRed[1], ColorRed[2],
			pt.x(), pt.y(), 0, G_PuckHeight, G_PuckDiameter / 2, 32, 1);
		pt = game->GetPlayerMalletPosition();
		//std::cout << pt.x() << " " << pt.y() << std::endl;
		glutSolidMallet(ColorBlue[0], ColorBlue[1], ColorBlue[2],
			pt.x(), pt.y(), 0, G_MalletHeight, G_MalletDiameter / 2, 32, 1);
		pt = game->GetOpponentMalletPosition();
		glutSolidMallet(ColorViolet[0], ColorViolet[1], ColorViolet[2],
			pt.x(), pt.y(), 0, G_MalletHeight, G_MalletDiameter / 2, 32, 1);
	}

	if (gameMode == MainWindow) { // 主界面显示
		// title
		double k = 1 / sqrt(look_eye[0] * look_eye[0] + look_eye[1] * look_eye[1]), t;
		selectFont(80, ANSI_CHARSET, "System");
		//glPushMatrix();
		glColor3f(0.2706, 0.1451, 0.0118);
		t = look_eye_dis / look_eye_dis_init * 1.2;
		glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.5 * t);
		drawString(MY_GL_WINDOW_NAME);
		//glPopMatrix();
		// press enter
		if (flashCount < 0.6 * Word_Flash_Time) {
			//glPushMatrix();
			selectFont(45, ANSI_CHARSET, "System");
			glColor3f(0.15, 0.15, 0.15);
			t = look_eye_dis / look_eye_dis_init * 1.145;
			glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 2.2,
				-look_eye[0] * k * t + look_eye[1] * k * t * 2.2, 1 * t);
			drawString("press ENTER to start");
			//glPopMatrix();
		}
		// round set
		//glPushMatrix();
		selectFont(45, ANSI_CHARSET, "System");
		glColor3f(0.2, 0.2, 0.2);
		t = look_eye_dis / look_eye_dis_init * 1.72;
		glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 0.4 * t);
		char buf[35];
		sprintf_s(buf, "reach %2d round to win", roundSet);
		drawString(buf);
		//glPopMatrix();
		// ai choose
		//glPushMatrix();
		selectFont(45, ANSI_CHARSET, "System");
		glColor3f(0.2, 0.2, 0.2);
		t = look_eye_dis / look_eye_dis_init * 1.37;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 1.2,
			-look_eye[0] * k * t + look_eye[1] * k * t * 1.2, 1 * t);
		sprintf_s(buf, "you will meet %s AI", AILevelName[aiLevel]);
		drawString(buf);
		//glPopMatrix();
	}
	else if (gameMode == Gaming) { // 游戏界面显示
		// board
		char buf[5];
		double k = 1 / sqrt(look_eye[0] * look_eye[0] + look_eye[1] * look_eye[1]);
		double t = look_eye_dis / look_eye_dis_init * 0.7;
		
		selectFont(48, ANSI_CHARSET, "System");
		//glPushMatrix();
		glColor3f(ColorBlue[0], ColorBlue[1], ColorBlue[2]);
		glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 3 * t);
		sprintf_s(buf, "%d", score1);
		drawString(buf);
		//glPopMatrix();

		//glPushMatrix();
		glColor3f(ColorViolet[0], ColorViolet[1], ColorViolet[2]);
		glRasterPos3f(-look_eye[1] * k * t, look_eye[0] * k * t, 3 * t);
		sprintf_s(buf, "%d", score2);
		drawString(buf);
		//glPopMatrix();

		//glPushMatrix();
		glColor3f(0, 0, 0);
		glRasterPos3f(0, 0, 3 * t);
		drawString(":");
		//glPopMatrix();

		if (waitingTime > 0) {
			//glPushMatrix();
			selectFont(90, ANSI_CHARSET, "System");
			glColor3f(0.9098, 0.6784, 0.4314);
			t = look_eye_dis / look_eye_dis_init * 0.2;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 0.7 * t);
			char buf[5];
			if (waitingTime > GameStart_ShowingTime)
				sprintf_s(buf, "%d", (waitingTime - GameStart_ShowingTime) / 1000 + 1);
			else
				sprintf_s(buf, "%s", "Go!");
			drawString(buf);
			//glPopMatrix();
		}
	}
	else if (gameMode == Goal) { // 进球界面显示
		double k = 1 / sqrt(look_eye[0] * look_eye[0] + look_eye[1] * look_eye[1]), t;
		selectFont(80, ANSI_CHARSET, "System");
		//glPushMatrix();
		if (winner == 1) {
			glColor3f(0.0902, 0.0902, 0.6980);
			t = look_eye_dis / look_eye_dis_init * 1.3;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.4 * t);
			drawString("You Goaled");
		}
		else {
			glColor3f(0.4039, 0.2588, 0.7725);
			t = look_eye_dis / look_eye_dis_init * 1.4;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.3 * t);
			drawString("You Fumbled");
		}
		//glPopMatrix();

		//glPushMatrix();
		selectFont(45, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 1.22;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 2.2,
			-look_eye[0] * k * t + look_eye[1] * k * t * 2.2, 1 * t);
		drawString("press ENTER to continue");
		//glPopMatrix();

		//glPushMatrix();
		selectFont(42, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 0.7;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 3.6,
			-look_eye[0] * k * t + look_eye[1] * k * t * 3.6, 0.85 * t);
		drawString("press R to return");
		//glPopMatrix();
	}
	else if (gameMode == Win) { // 胜利界面显示
		double k = 1 / sqrt(look_eye[0] * look_eye[0] + look_eye[1] * look_eye[1]), t;
		selectFont(80, ANSI_CHARSET, "System");
		//glPushMatrix();
		if (winner == 1) {
			glColor3f(0.0902, 0.0902, 0.6980);
			t = look_eye_dis / look_eye_dis_init * 1.1;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.4 * t);
			drawString("You Win !");
		}
		else {
			glColor3f(0.4039, 0.2588, 0.7725);
			t = look_eye_dis / look_eye_dis_init * 1.06;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.3 * t);
			drawString("You Lose");
		}
		//glPopMatrix();

		//glPushMatrix();
		selectFont(45, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 1.22;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 2.2,
			-look_eye[0] * k * t + look_eye[1] * k * t * 2.2, 1 * t);
		drawString("press ENTER to restart");
		//glPopMatrix();

		//glPushMatrix();
		selectFont(42, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 0.7;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 3.6,
			-look_eye[0] * k * t + look_eye[1] * k * t * 3.6, 0.85 * t);
		drawString("press R to return");
		//glPopMatrix();
	}
	else if (gameMode == Pause) { //暂停界面显示
		double k = 1 / sqrt(look_eye[0] * look_eye[0] + look_eye[1] * look_eye[1]), t;
		if (flashCount < 0.6 * Word_Flash_Time) {
			selectFont(80, ANSI_CHARSET, "System");
			//glPushMatrix();
			glColor3f(0.25, 0.25, 0.25);
			t = look_eye_dis / look_eye_dis_init * 0.8;
			glRasterPos3f(look_eye[1] * k * t, -look_eye[0] * k * t, 1.7 * t);
			drawString("Pause");
			//glPopMatrix();
		}

		//glPushMatrix();
		selectFont(45, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 1.22;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 2.2,
			-look_eye[0] * k * t + look_eye[1] * k * t * 2.2, 1 * t);
		drawString("press ENTER to continue");
		//glPopMatrix();

		//glPushMatrix();
		selectFont(42, ANSI_CHARSET, "System");
		glColor3f(0.15, 0.15, 0.15);
		t = look_eye_dis / look_eye_dis_init * 0.7;
		glRasterPos3f(look_eye[1] * k * t + look_eye[0] * k * t * 3.6,
			-look_eye[0] * k * t + look_eye[1] * k * t * 3.6, 0.85 * t);
		drawString("press R to return");
		//glPopMatrix();
	}

	glFlush();
	glutSwapBuffers();
}

/* 映射鼠标坐标至openGL坐标
 * 坐标位置为鼠标指向的物体表面点坐标
 * 修正至Z = 0平面
 * 返回值始终为真，无意义
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
	// 修正点击在边界或圆盘上的情况
	if (posZ > 1e-5 || posZ < -1e-5) {
		// 不在桌面上，需要进行修整
		double k = posZ / (posZ - look_eye[2]);
		posX += k * (look_eye[0] - posX);
		posY += k * (look_eye[1] - posY);
		posZ = 0;
	}
	//std::cout << "real " << posX << " " << posY << " " << posZ << std::endl;
	rx = posX, ry = posY;
	return true;
}

void Reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(look_dep_angle, (GLfloat)w / h, 0.01, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

void OnMouseMove(int x, int y)
{
	mouseXrec = x, mouseYrec = y;
}

void Mouse(int button, int state, int x, int y)
{
	mouseXrec = x, mouseYrec = y;
}

void Keyboard(unsigned char key, int x, int y)
{
	if (gameMode == MainWindow) { // 主界面响应回车
		switch (key) {
		case 13: // ENTER
			game->Restart();
			gameMode = Gaming;
			waitingTime = GameStart_WaitingTime;
			score1 = score2 = 0;
			break;
		}
	}
	else if (gameMode == Goal) { // 得分界面响应回车，R
		switch (key) {
		case 13: // ENTER
			game->Restart();
			gameMode = Gaming;
			waitingTime = GameStart_WaitingTime;
			break;
		case 'r':
			game->Restart();
			gameMode = MainWindow;
			flashCount = 0;
			break;
		}
	}
	else if (gameMode == Win) { // 胜利界面响应回车，R
		switch (key) {
		case 13: // ENTER
			game->Restart();
			gameMode = Gaming;
			waitingTime = GameStart_WaitingTime;
			score1 = score2 = 0;
			break;
		case 'r':
			game->Restart();
			gameMode = MainWindow;
			flashCount = 0;
			break;
		}
	}
	else if (gameMode == Gaming) { // 游戏界面响应P
		switch (key) {
		case 'p':
			gameMode = Pause;
			flashCount = 0;
			break;
		}
	}
	else if (gameMode == Pause) { // 暂停界面响应回车，R
		switch (key) {
		case 13: // ENTER
			gameMode = Gaming;
			break;
		case 'r':
			game->Restart();
			gameMode = MainWindow;
			flashCount = 0;
			break;
		}
	}
}

void SpecialKeys(int key, int x, int y)
{
	if (gameMode == Gaming || gameMode == Goal || gameMode == Pause || gameMode == Win) {
		// 除主界面，其余界面响应上下左右进行视点调整
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
	else if (gameMode == MainWindow) {
		// 主界面响应上下左右进行游戏设定
		switch (key) {
		case GLUT_KEY_UP:
			++roundSet;
			if (roundSet > Game_Round_MAX)
				roundSet = Game_Round_MAX;
			break;
		case GLUT_KEY_DOWN:
			--roundSet;
			if (roundSet < Game_Round_MIN)
				roundSet = Game_Round_MIN;
			break;
		case GLUT_KEY_LEFT:
			--aiLevel;
			if (aiLevel < 0)
				aiLevel = 0;
			break;
		case GLUT_KEY_RIGHT:
			++aiLevel;
			if (aiLevel >= Game_AI_NUM)
				aiLevel = Game_AI_NUM - 1;
			break;
		}
	}
}

void Update(int value)
{
	int game_delta_time_real = glutGet(GLUT_ELAPSED_TIME);
	//int game_delta_time_real = GLUT_ELAPSED_TIME;
	if (game_delta_time_real > 2 * GAME_DELTA_TIME)
		game_delta_time_real = 2 * GAME_DELTA_TIME;
	//std::cout << game_delta_time_real << std::endl;

	if (gameMode == Gaming) {
		if (waitingTime > 0) {
			waitingTime -= game_delta_time_real;
		}
		else {
			/* call game with player mallet position & delta-time
			* the coord of mallets has to be transed into 2D
			* the center of plant is (0, 0)
			*/
			GLdouble x, y;
			if (GetOGLPos(mouseXrec, mouseYrec, x, y)) {
				int ret = game->Run(game_delta_time_real, x, y);
				if (ret) {
					if (ret == 1) ++score1, winner = 1; else ++score2, winner = 2;
					if (score1 == roundSet || score2 == roundSet)
						gameMode = Win;
					else
						gameMode = Goal;
				}
			}
		}
	}
	else if (gameMode == MainWindow) {
		// 主界面闪烁提示
		flashCount += game_delta_time_real;
		if (flashCount >= Word_Flash_Time)
			flashCount -= Word_Flash_Time;
		// 主界面视角缓慢旋转
		LookPosMaintain(look_delta_angle_shift, 0);
	}
	else if (gameMode == Pause) {
		// 暂停界面闪烁提示
		flashCount += game_delta_time_real;
		if (flashCount >= Word_Flash_Time)
			flashCount -= Word_Flash_Time;
	}

	/* display */
	glutPostRedisplay();
	/* recursive call */
	glutTimerFunc(game_delta_time_real, Update, 0);
}
