#pragma once

#include "glut.h"
#include <GL/gl.h>
#include <GL/glu.h>

const double PI = 3.14159265358979323;

const int MYGL_WINDOW_W = 800;
const int MYGL_WINDOW_H = 600;
const int MYGL_WINDOW_POS_X = 200;
const int MYGL_WINDOW_POS_Y = 200;
const char MY_GL_WINDOW_NAME[] = "AirHockey";

const GLfloat look_init_angle = 45; // 初始视角45deg
const GLfloat look_delta_angle = 1; // 旋转视角间隔1deg
const GLfloat look_delta_angle_shift = 0.016; // 旋转视角间隔1deg

const GLfloat look_eye_dis_init = 6.5;
const GLfloat look_eye_dis_max = 9.1;
const GLfloat look_eye_dis_min = 5.5;
const GLfloat look_eye_dis_delta = 0.2;
const GLfloat look_scn_dis = 1;

const GLfloat look_dep_angle = 45; // 俯视45deg

const GLfloat light_pos[] = { 0, 5, 30, 1.0 };
const GLfloat light_ambient[] = { 0.1f, 0.2f, 0.2f, 1.0f };
const GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat light_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
const GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
const GLfloat mat_emission[] = { 0.0, 0.0, 0.0, 1.0 };

const int GAME_DELTA_TIME = 10; // ms

const GLint TileCount = 70; // 影响地板平铺密度

const GLfloat ColorRed[] = { 0.8157f, 0.0157f, 0.0157f };
const GLfloat ColorBlue[] = { 0.1765f, 0.1765f, 0.9765f };
const GLfloat ColorViolet[] = { 0.5686f, 0.0039f, 0.9490f };

const int WIN_MAIN_WIN_START = 0;
const int WIN_MAIN_WIN_END = 2;
const int WIN_LEVEL_CHOOSE_START = 5;
const int WIN_LEVEL_CHOOSE_END = 8;
const int WIN_GOAL_START = 8;
const int WIN_GOAL_END = 10;
const int WIN_END_START = 10;
const int WIN_END_END = 12;
const char LabelString[][15] = {
	"AirHockey",
	"single player",
	"double player",
	"auto",
	"help",
	"sample",
	"hard",
	"ferocious",
	"continue",
	"return",
	"restart",
	"return"
};

const int Game_Round_Init = 3;
const int Game_Round_MIN = 1;
const int Game_Round_MAX = 99;
const int Word_Flash_Time = 1400; // ms