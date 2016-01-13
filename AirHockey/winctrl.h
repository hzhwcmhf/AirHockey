#pragma once

/* 游戏界面编号 */
enum WinModeType {
	WIN_MAIN_WIN, // 主界面
	WIN_LEVEL_CHOOSE, // 难度选择
	WIN_GAME, // 游戏界面
	WIN_HELP, // 帮助
	WIN_GOAL, // 得分界面
	WIN_END // 游戏结束界面
};

class WinCtrl
{
public:
	/* 返回当前游戏界面编号 */
	WinModeType mode() const;
	/* 给定鼠标选择相对坐标，返回选中的按钮，-1表示未选中任何按钮 */
	int getChoose(double x, double y) const;
	/* 选择某编号按钮 */
	void choose(int label);
	/* 获取当前比分 */
	void getScore(int &player1, int &player2) const;
	/* 设置得分方 */
	void setGoal(int player);
public:
	WinCtrl();
private:
	WinModeType _mode;
	int _score1, _score2;
};