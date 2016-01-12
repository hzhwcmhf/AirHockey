#pragma once

const double PI = 3.14159265358979323;

const int MYGL_WINDOW_W = 800;
const int MYGL_WINDOW_H = 600;
const int MYGL_WINDOW_POS_X = 200;
const int MYGL_WINDOW_POS_Y = 200;
const char MY_GL_WINDOW_NAME[] = "AirHockey";

const GLfloat look_init_angle = 45; // 初始视角30deg
const GLfloat look_delta_angle = 2; // 旋转视角间隔5deg

const GLfloat look_eye_dis_init = 6.5;
const GLfloat look_eye_dis_max = 9;
const GLfloat look_eye_dis_min = 5;
const GLfloat look_eye_dis_delta = 0.2;
const GLfloat look_scn_dis = 1;

const GLfloat look_dep_angle = 45; // 俯视45deg

const GLfloat light_pos[] = { 0, 1, 30, 1.0 };
const GLfloat light_ambient[] = { 0.1, 0.2, 0.2, 1.0 };
const GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat light_specular[] = { 0.1, 0.1, 0.1, 1.0 };
const GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
const GLfloat mat_emission[] = { 0.0, 0.0, 0.0, 1.0 };

const int GAME_DELTA_TIME = 10; // ms
