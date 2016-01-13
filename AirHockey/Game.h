#pragma once

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


class Game;

class AI
{
private:
	Game* _game;
public:
	AI(Game* game) : _game(game) {};
	Point QueryAction();
};

class Game
{
private:
	Point _playerBoard[4], _opponentBoard[4], _board[4];

	Point _playerPos, _opponentPos;
	Point _puckPos, _puckDirection;
	int _gameStatus;

	AI *_ai;

	const double C_maxTimeInterval = 1;
	

	void QueryBoardDistance(const Point &s, const Point &e, const Point p[4], double res[4]);
	double QueryBoardDistance(const Point &s, const Point &e, const Point p[4]);

	std::tuple<double, std::function<void(Point&)>> TestMalletCollisionWithPuck(const Point &s, const Point &e);
	std::tuple<double, std::function<void(Point&)>> TestMalletCollisionWithBoard(const Point &s, const Point &e, const Point p[4]);
	void MoveMallet(const Point &s, const Point &e, const Point p[4], Point &malletPos);

	std::tuple<double, std::function<void()>> TestPuckCollisionWithBoard(const Point & s, const Point & e, const Point p[4]);
	std::tuple<double, std::function<void()>> TestPuckCollisionWithMallet(const Point &s, const Point &e, const Point &o);
	double MovePuck(double maxTime);
	
	void TestWin(int pos);

public:
	Game() {
		_ai = new AI(this);

		_playerBoard[0] = Point(-G_tableWidth / 2 + G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[1] = Point(                0 - G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[2] = Point(                0 - G_malletRadius, +G_tableHeight / 2 - G_malletRadius);
		_playerBoard[3] = Point(-G_tableWidth / 2 + G_malletRadius, +G_tableHeight / 2 - G_malletRadius);

		_playerBoard[0] = Point(                0 + G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[1] = Point(+G_tableWidth / 2 - G_malletRadius, -G_tableHeight / 2 + G_malletRadius);
		_playerBoard[2] = Point(+G_tableWidth / 2 - G_malletRadius, +G_tableHeight / 2 - G_malletRadius);
		_playerBoard[3] = Point(			    0 + G_malletRadius, +G_tableHeight / 2 - G_malletRadius);

		_board[0] = Point(-G_tableWidth / 2 + G_puckRadius, -G_tableHeight / 2 + G_puckRadius);
		_board[1] = Point(+G_tableWidth / 2 - G_puckRadius, -G_tableHeight / 2 + G_puckRadius);
		_board[2] = Point(+G_tableWidth / 2 - G_puckRadius, +G_tableHeight / 2 - G_puckRadius);
		_board[3] = Point(-G_tableWidth / 2 + G_puckRadius, +G_tableHeight / 2 - G_puckRadius);

		Restart();
	}
	~Game() {
		delete _ai;
	};

	Point GetPlayerMalletPosition() const;
	Point GetOpponentMalletPosition() const;
	Point GetPuckPosition() const;

	//输入：经历时间，鼠标坐标x,y
	//返回：状态 正常0，赢1，输2
	int Run(int times, double x, double y);

	void Restart();
};