#include "stdafx.h"

#include "Constants.h"
#include "Game.h"

inline double Cross(const Point &a, const Point &b)
{
	return imag(a*conj(b));
}
inline double Cross(const Point &a, const Point &o, const Point &b)
{
	return Cross(a - o, b - o);
}
inline Point Rotate(const Point &a)
{
	return a*Point(0, 1);
}
inline Point QueryLinesIntersection(const Point &a, const Point &b, const Point &c, const Point &d)
{
	double S1 = Cross(c, a, b);
	double S2 = Cross(b, a, d);
	return c + (d - c) * S1 / (S1 + S2);
}
inline std::tuple<bool,Point,Point> QueryLineCircleIntersection(const Point &a, const Point &b, const Point &o, double r)
{
	Point h = QueryLinesIntersection(a, b, o, o + Rotate(b - a));
	double perpendicularDis2 = norm(h - o);
	double dis2 = r * r - perpendicularDis2;
	if (dis2 < 0) return make_tuple(false, Point(), Point());
	Point disVec = (b - a) / abs(b - a) * sqrt(dis2);
	return make_tuple(true, h - disVec, h + disVec);
}
inline double QueryPointInLine(const Point &a, const Point &b, const Point &c)
{
	Point ans = (c - a) / (b - a);
	return real(ans) > 0 ? abs(ans) : -abs(ans);
}
inline double QueryLineCircleIntersectionPos(const Point &a, const Point &b, const Point &o, double r)
{
	Point firstIntersection;
	bool ifIntersect;
	std::tie(ifIntersect, firstIntersection, std::ignore) =
		QueryLineCircleIntersection(a, b, o, r);
	if (ifIntersect) {
		double pos = QueryPointInLine(a, b, firstIntersection);
		if (pos > 0) return pos;
		return 1;
	}
	return 1;
}
inline double QueryLinesIntersectionPos(const Point &a, const Point &b, const Point &c, const Point &d)
{
	if (Cross(b - a, d - c) > 0) return 1;
	double S1 = Cross(a, c, d);
	double S2 = Cross(d, c, b);
	double ans = S1 / (S1 + S2);
	if (ans < 0) return 1;
	return ans;
}


void Game::QueryBoardDistance(const Point & s, const Point & e, const Point p[4], double res[4])
{
	for (int i = 0;i < 4;i++) {
		res[i] = QueryLinesIntersectionPos(s, e, p[i], p[(i+1)%4]);
	}

}
double Game::QueryBoardDistance(const Point & s, const Point & e, const Point p[4])
{
	double res[4];
	QueryBoardDistance(s, e, p, res);
	return std::min(std::min(res[0], res[1]), std::min(res[2], res[3]));
}

Point Game::MoveMallet(const Point & s, const Point & e, const Point p[4])
{
	if (norm(s - e) < C_minDistance*C_minDistance) return e;

	//Mallet与Puck交点
	double puckDistance = QueryLineCircleIntersectionPos(s, e, _puckPos, G_malletRadius + G_puckRadius);

	//与桌边界交点
	double boardDistance = QueryBoardDistance(s, e, p);

	double distance = std::min(puckDistance, boardDistance);

	return s + (e - s) * distance;
}

double Game::TestPuckCollisionWithBoard(const Point & s, const Point & e, const Point p[4])
{
	double res[4];
	QueryBoardDistance(s, e, p, res);
	int minPos = 0;
	double minVal = res[0];
	for (int i = 1; i < 4;i++) {
		if (minVal > res[i]) minVal = res[i], minPos = i;
	}

	if (minVal >= 1) return 1;
	
	Point reflectVec = p[(minPos + 1) % 4] - p[minPos];
	reflectVec /= abs(reflectVec);
	_puckDirection = reflectVec * reflectVec / _puckDirection;
	_puckPos = s + (e - s) * minVal;

	if(minPos == 1 || minPos ==3 ) TestWin(minPos);

	return minVal;
}

double Game::TestPuckCollisionWithMallet(const Point & s, const Point & e, const Point & o)
{
	double malletDistance = QueryLineCircleIntersectionPos(s, e, o, G_malletRadius + G_puckRadius);
	if (malletDistance < 1) {
		Point p = s + (e - s) * malletDistance;
		Point reflectVec = Rotate(o - p);
		reflectVec /= abs(reflectVec);
		_puckDirection = reflectVec * reflectVec / _puckDirection;
		_puckPos = p;
		return malletDistance;
	}
	return 1;
}

double Game::MovePuck(double maxTime)
{
	Point s = _puckPos;
	Point e = _puckPos + _puckDirection * maxTime * G_puckSpeed;

	//Puck与Mallet交点
	double mallet1Distance = TestPuckCollisionWithMallet(s, e, _playerPos);
	if (mallet1Distance < 1) return mallet1Distance * maxTime;
	double mallet2Distance = TestPuckCollisionWithMallet(s,e, _opponentPos);
	if (mallet2Distance < 1) return mallet2Distance * maxTime;

	//Puck与board交点
	
	double boardDistance = TestPuckCollisionWithBoard(s, e, _board);
	if (boardDistance < 1) return boardDistance * maxTime;

	_puckPos = e;

	return maxTime;
}

void Game::TestWin(int pos)
{
	if (abs(_puckPos.y()) + G_puckRadius < G_goalWidth / 2) {
		_gameStatus = pos == 1 ? 1 : 2;
	}
}

Point Game::GetPlayerMalletPosition() const
{
	return _playerPos;
}

Point Game::GetOpponentMalletPosition() const
{
	return _opponentPos;
}

Point Game::GetPuckPosition() const
{
	return _puckPos;
}

int Game::Run(int times, double x, double y)
{
	Point playerTarget = Point(x, y);
	Point opponentTarget = _ai->QueryAction();
	
	//先放置mallet
	_playerPos = MoveMallet(_playerPos, playerTarget, _playerBoard);
	_opponentPos = MoveMallet(_opponentPos, opponentTarget, _opponentBoard);

	//进行碰撞检测
	double now = 0;
	while (now < times) {
		double passTime = MovePuck(std::min(times - now, C_maxTimeInterval));
		now += passTime;

		if (_gameStatus > 0) return _gameStatus;
	}

	//再次放置mallet
	_playerPos = MoveMallet(_playerPos, playerTarget, _playerBoard);
	_opponentPos = MoveMallet(_opponentPos, opponentTarget, _opponentBoard);
	return 0;
}

void Game::Restart()
{
	_gameStatus = 0;

	_playerPos = Point(G_malletStartDistance, 0);
	_opponentPos = Point(-G_malletStartDistance, 0);

	_puckPos = Point(0, 0);

	srand((int)time(0));

	_puckDirection = Point(G_tableHeight / 2, rand() * G_goalWidth / RAND_MAX - G_goalWidth / 2);
	_puckDirection /= abs(_puckDirection);
}



Point AI::QueryAction()
{
	return _game->GetOpponentMalletPosition();
}
