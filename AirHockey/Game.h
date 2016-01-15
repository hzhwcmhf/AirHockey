#pragma once

#include "Constants.h"

//向量、点类型
class Point : public std::complex<double>
{
public:
	Point() : std::complex<double>(0, 0) {}
	Point(double x, double y) : std::complex<double>(x, y) {}
	Point(const std::complex<double> &p) : std::complex<double>(p) {}
	double x() const
	{
		return real();
	}
	double y() const
	{
		return imag();
	}
	std::tuple<double, double> pack() const
	{
		return std::make_tuple(real(), imag());
	}

	template<class T>
	Point& operator=(const T &p)
	{
		std::complex<double>::operator=(p);
		return *this;
	}

	/*Point& operator=(const std::complex<double> &p)
	{
		std::complex<double>::operator=(p);
		return *this;
	}*/
};

//前置声明
class Game;

//AI基类
class AI
{
protected:
	//AI所针对的Game对象
	Game* _game;
public:
	AI(Game* game) : _game(game) {}
	virtual ~AI() {}
	//询问AI操作的接口，参数为经过时间（单位ms）
	virtual Point QueryAction(int times) = 0;
};

//简单AI
class SimpleAI : public AI
{
private:
	//巡逻起点
	const Point C_s = Point(-G_malletStartDistance, -G_tableHeight / 4);
	//巡逻终点
	const Point C_e = Point(-G_malletStartDistance, G_tableHeight / 4);
	//巡逻时间
	const int C_step = 1000;
	//总巡逻时间
	int _nowStep;

public:
	SimpleAI(Game* game) : AI(game) { _nowStep = C_step / 2; }
	virtual Point QueryAction(int times);
};

//中等AI
class NormalAI : public AI
{
private:
	//AI速度，每ms移动距离
	const double C_speed = 0.002;
public:
	NormalAI(Game* game) : AI(game) {}
	virtual Point QueryAction(int times);
};

//疯狂AI
class CrazyAI : public AI
{
private:
	const double C_slowSpeed = 0.0005;	//慢速速度
	const double C_speed = 0.0015;	//普通速度
	const double C_highSpeed = 0.003;	//高速速度
	const double C_superSpeed = 0.004;	//急速速度
public:
	CrazyAI(Game* game) : AI(game) {}
	virtual Point QueryAction(int times);
};

//游戏逻辑类
class Game
{
private:
	//己方曲棍边界，敌方曲棍边界，冰球边界
	Point _playerBoard[4], _opponentBoard[4], _board[4];

	//己方曲棍位置，敌方曲棍位置
	Point _playerPos, _opponentPos;
	//冰球位置，冰球方向（单位向量）
	Point _puckPos, _puckDirection;

	
	int _gameStatus; //当前游戏胜负状态(0正常，1赢，2输）
	int _aiLevel; //AI级别

	
	AI *_ai;	//AI对象
	friend class NormalAI;
	friend class CrazyAI;

	
	//const double C_maxTimeInterval = 1;

	//求s到e正方向上到边界的距离（按和ab的比例为单位），存入res
	void QueryBoardDistance(const Point &s, const Point &e, const Point p[4], double res[4]);
	//求s到e正方向上到边界的距离（按和ab的比例为单位）中的最小值
	double QueryBoardDistance(const Point &s, const Point &e, const Point p[4]);

	//求曲棍从s移动到e是否会撞到冰球
	//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
	//返回碰撞后对曲棍和冰球的影响函数
	std::tuple<double, std::function<void(Point&)>> TestMalletCollisionWithPuck(const Point &s, const Point &e);
	//求曲棍从s移动到e是否会撞到边界
	//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
	//返回碰撞后对曲棍的影响函数
	std::tuple<double, std::function<void(Point&)>> TestMalletCollisionWithBoard(const Point &s, const Point &e, const Point p[4]);
	//移动曲棍从s到e，p为边界，malletPos为移动后的曲棍位置（输出变量）
	void MoveMallet(const Point &s, const Point &e, const Point p[4], Point &malletPos);

	//求冰球从s移动到e是否会撞到边界
	//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
	//返回碰撞后对冰球的影响、胜负的判断函数
	std::tuple<double, std::function<void()>> TestPuckCollisionWithBoard(const Point & s, const Point & e, const Point p[4]);
	//求冰球从s移动到e是否会撞到曲棍
	//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
	//返回碰撞后对冰球的影响函数
	std::tuple<double, std::function<void()>> TestPuckCollisionWithMallet(const Point &s, const Point &e, const Point &o);
	//移动曲棍，输入模拟运行时间
	//若有碰撞，处理碰撞，返回碰撞时间
	//无碰撞返回输入时间
	double MovePuck(double maxTime);
	
	//碰撞后检测是否胜利
	void TestWin(int pos);

public:
	Game(int aiLevel) {
		_ai = 0;
		_aiLevel = aiLevel;

		//初始化边界位置
		_opponentBoard[0] = Point(-G_tableWidth / 2 + G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_opponentBoard[1] = Point(-G_tableWidth / 6 - G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_opponentBoard[2] = Point(-G_tableWidth / 6 - G_malletRadius, +G_tableHeight / 2 - G_malletRadius);
		_opponentBoard[3] = Point(-G_tableWidth / 2 + G_malletRadius, +G_tableHeight / 2 - G_malletRadius);

		_playerBoard[0] = Point(+G_tableWidth / 6 + G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[1] = Point(+G_tableWidth / 2 - G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[2] = Point(+G_tableWidth / 2 - G_malletRadius, +G_tableHeight / 2 - G_malletRadius);
		_playerBoard[3] = Point(+G_tableWidth / 6 + G_malletRadius, +G_tableHeight / 2 - G_malletRadius);

		_board[0] = Point(-G_tableWidth / 2 + G_puckRadius, -G_tableHeight / 2 + G_puckRadius);
		_board[1] = Point(+G_tableWidth / 2 - G_puckRadius, -G_tableHeight / 2 + G_puckRadius);
		_board[2] = Point(+G_tableWidth / 2 - G_puckRadius, +G_tableHeight / 2 - G_puckRadius);
		_board[3] = Point(-G_tableWidth / 2 + G_puckRadius, +G_tableHeight / 2 - G_puckRadius);

		Restart();
	}
	~Game() {
		if(_ai) delete _ai;
	};

	//获取自己曲棍位置
	Point GetPlayerMalletPosition() const;
	//获取对手曲棍位置
	Point GetOpponentMalletPosition() const;
	//获取冰球位置
	Point GetPuckPosition() const;

	//运行times时间(ms)，己方操作为指向（x,y)
	//输入：经历时间，鼠标坐标x,y
	//返回：状态 正常0，赢1，输2
	int Run(int times, double x, double y);

	//重新开始
	void Restart();
};