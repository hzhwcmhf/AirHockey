#pragma once

#include "glut.h"
#include <GL/gl.h>
#include <GL/glu.h>

const double PI = 3.14159265358979323;

const int MYGL_WINDOW_W = 800; // 窗口宽度
const int MYGL_WINDOW_H = 600; // 窗口高度
const int MYGL_WINDOW_POS_X = 200; // 窗口位置X
const int MYGL_WINDOW_POS_Y = 200; // 窗口位置Y
const char MY_GL_WINDOW_NAME[] = "AirHockey"; // 游戏标题

const GLfloat look_init_angle = 30; // 初始视角45deg
const GLfloat look_delta_angle = 1; // 旋转视角间隔1deg
const GLfloat look_delta_angle_shift = 0.016; // 主界面旋转视角间隔0.016deg

/* 视点距离原点距离
 * 变化范围 5.5~10.1
 * 变化间隔 0.2
 * 初始设置 6.5
 * 基础为桌面长 4
 */
const GLfloat look_eye_dis_init = 6.5;
const GLfloat look_eye_dis_base = 4;
const GLfloat look_eye_dis_max = 10.1;
const GLfloat look_eye_dis_min = 5.5;
const GLfloat look_eye_dis_delta = 0.2;
const GLfloat look_scn_dis = 1; // 视点方向某一距离，无意义常量

const GLfloat look_dep_angle = 45; // 视点俯视45deg

/* 光照及材质设置常量 */
const GLfloat light_pos[] = { 0, 5, 30, 1.0 };
const GLfloat light_ambient[] = { 0.1f, 0.2f, 0.2f, 1.0f };
const GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat light_specular[] = { 0.1f, 0.1f, 0.1f, 1.0f };
const GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
const GLfloat mat_emission[] = { 0.0, 0.0, 0.0, 1.0 };

/* 游戏循环时间间隔 */
const int GAME_DELTA_TIME = 10; // ms

const GLint TileCount = 70; // 影响地板平铺密度

const GLfloat ColorRed[] = { 0.8157f, 0.0157f, 0.0157f }; // 红色
const GLfloat ColorBlue[] = { 0.1765f, 0.1765f, 0.9765f }; // 蓝色
const GLfloat ColorViolet[] = { 0.5686f, 0.0039f, 0.9490f }; // 紫色

//const int WIN_MAIN_WIN_START = 0;
//const int WIN_MAIN_WIN_END = 2;
//const int WIN_LEVEL_CHOOSE_START = 5;
//const int WIN_LEVEL_CHOOSE_END = 8;
//const int WIN_GOAL_START = 8;
//const int WIN_GOAL_END = 10;
//const int WIN_END_START = 10;
//const int WIN_END_END = 12;
//const char LabelString[][15] = {
//	"AirHockey",
//	"single player",
//	"double player",
//	"auto",
//	"help",
//	"sample",
//	"hard",
//	"ferocious",
//	"continue",
//	"return",
//	"restart",
//	"return"
//};

/* 游戏一局胜利所需得分数
 * 范围为 1~99
 * 初始为 3 
 */
const int Game_Round_Init = 3;
const int Game_Round_MIN = 1;
const int Game_Round_MAX = 99;

/* AI难度
 * 分为3级
 */
const int Game_AI_INIT = 1;
const int Game_AI_NUM = 3;
const char AILevelName[][10] = {
	"EASY",
	"NORMAL",
	"CRAZY"
};

const int Word_Flash_Time = 1400; // (ms) 字条闪烁循环时间，必须为GAME_DELTA_TIME的倍数

const double GL_MalletRange = 0.33333333; // =1/3 曲棍移动范围线相对位置
const double GL_KickOffCircle_R = 0.1; // =1/10 开球圈半径相对大小

const int GameStart_WaitingSecond = 3; // (s) 每轮游戏开始延迟等待秒数
const int GameStart_ShowingTime = 500; // (ms) 游戏开始提示显示时间
const int GameStart_WaitingTime = GameStart_WaitingSecond * 1000 + GameStart_ShowingTime; // (ms) 每轮游戏开始延迟等待时间
