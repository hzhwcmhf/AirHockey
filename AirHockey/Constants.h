#pragma once


const double G_eps = 1e-6;	//浮点数比较时，小于这个距离认为相等

const double G_tableWidth = 4;	//桌子x长
const double G_tableHeight = 2;	//桌子y长
const double G_malletStartDistance = G_tableWidth / 4;	//双方起点的坐标
const double G_goalWidth = 0.8;	//进球区的长度
const double G_puckRadius = 0.1;	//冰球半径
const double G_malletRadius = 0.15;	//曲棍半径
const double G_puckSpeed = 0.004;	//冰球速度
const double G_calculateMaxTimes = 100;	//单次循环碰撞计算最大次数

const double G_PuckHeight = 0.1; // 冰球高度
const double G_PuckDiameter = G_puckRadius * 2; // 冰球直径
const double G_MalletHeight = 0.15; // 曲棍高度
const double G_MalletDiameter = G_malletRadius * 2; // 曲棍直径 
const double G_TablePly = 0.1; // 桌子边界厚度
const double G_TableAltitude = 2; // 桌子高度
const double G_FloorPlaneSize = 30; // 地面大小
