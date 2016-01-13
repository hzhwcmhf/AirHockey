#include "stdafx.h"
#include "winctrl.h"
#include "openglConst.h"

WinCtrl::WinCtrl() : _mode(WIN_MAIN_WIN), _score1(0), _score2(0)
{
}

WinModeType WinCtrl::mode() const
{
	return _mode;
}

int WinCtrl::getChoose(double x, double y) const
{
	switch (_mode) {
	case WIN_MAIN_WIN:
		for (int i = WIN_MAIN_WIN_START; i < WIN_MAIN_WIN_END; ++i) {
			if (x > LabelPos[i][0] && x < LabelPos[i][1] && y > LabelPos[i][2] && y < LabelPos[i][3])
				return i;
		}
		return -1;
	case WIN_GOAL:
		for (int i = WIN_GOAL_START; i < WIN_GOAL_END; ++i) {
			if (x > LabelPos[i][0] && x < LabelPos[i][1] && y > LabelPos[i][2] && y < LabelPos[i][3])
				return i;
		}
		return -1;
	case WIN_END:
		for (int i = WIN_END_START; i < WIN_END_END; ++i) {
			if (x > LabelPos[i][0] && x < LabelPos[i][1] && y > LabelPos[i][2] && y < LabelPos[i][3])
				return i;
		}
		return -1;
	default:
		return -1;
	}
}

void WinCtrl::choose(int label)
{
	switch (_mode) {
	case WIN_MAIN_WIN:
		if (label == WIN_MAIN_WIN_START + 1) {
			_mode = WIN_GAME;
			_score1 = _score2 = 0;
		}
		break;
	case WIN_GOAL:
		if (label == WIN_GOAL_START)
			_mode = WIN_GAME;
		else if (label == WIN_GOAL_START + 1)
			_mode = WIN_MAIN_WIN;
		break;
	case WIN_END:
		if (label == WIN_END_START) {
			_mode = WIN_GAME;
			_score1 = _score2 = 0;
		}
		else if (label == WIN_END_START + 1)
			_mode = WIN_MAIN_WIN;
		break;
	}
}

void WinCtrl::getScore(int &player1, int &player2) const
{
	player1 = _score1, player2 = _score2;
}

void WinCtrl::setGoal(int player)
{
	if (player == 1)
		++_score1;
	else if (player == 2)
		++_score2;
}