#include "stdafx.h"

#include "Constants.h"
#include "Game.h"

inline double Cross(const Point &a, const Point &b)
{
	return imag(conj(a) * b);
}
inline double Cross(const Point &a, const Point &o, const Point &b)
{
	return Cross(a - o, b - o);
}
inline Point Rotate(const Point &a)
{
	return a * Point(0, 1);
}
inline int dblcmp(const double &x)
{
	if (x > G_eps) return 1;
	if (x < -G_eps) return -1;
	return 0;
}
inline Point QueryLinesIntersection(const Point &a, const Point &b, const Point &c, const Point &d)
{
	double S1 = Cross(c, a, b);
	double S2 = Cross(b, a, d);
	return c + (d - c) * S1 / (S1 + S2);
}
inline double QueryPointInLine(const Point &a, const Point &b, const Point &c)
{
	Point ans = (c - a) / (b - a);
	return real(ans) > 0 ? abs(ans) : -abs(ans);
}

//相切时忽略
inline std::tuple<bool,Point,Point> QueryLineCircleIntersection(const Point &a, const Point &b, const Point &o, double r)
{
	Point h = QueryLinesIntersection(a, b, o, o + Rotate(b - a));
	double perpendicularDis2 = norm(h - o);
	double dis2 = r * r - perpendicularDis2;
	if (dis2 < G_eps*G_eps) return make_tuple(false, Point(), Point());
	Point disVec = (b - a) / abs(b - a) * sqrt(dis2);
	return make_tuple(true, h - disVec, h + disVec);
}

//如果交点是正方向则返回位置，否则返回1
inline double QueryLineCircleIntersectionPos(const Point &a, const Point &b, const Point &o, double r)
{
	Point firstIntersection, secondIntersection;
	bool ifIntersect;
	std::tie(ifIntersect, firstIntersection, secondIntersection) =
		QueryLineCircleIntersection(a, b, o, r);
	if (ifIntersect) {
		double pos = QueryPointInLine(a, b, firstIntersection);
		if (pos > G_eps) {
			return pos;
		}else if (pos > -G_eps) {
			if (QueryPointInLine(a, b, secondIntersection) > G_eps)
				return 0;
		}
	}
	return 1;
}
inline double QueryLinesIntersectionPos(const Point &a, const Point &b, const Point &c, const Point &d)
{
	if (dblcmp(Cross(b - a, d - c)) <= 0) return 1;
	double S1 = Cross(a, c, d);
	double S2 = Cross(d, c, b);
	double ans = S1 / (S1 + S2);
	if (dblcmp(ans) < 0) return 1;
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

std::tuple<double, std::function<void(Point&)>> Game::TestMalletCollisionWithPuck(const Point & s, const Point & e)
{
	double puckDistance = QueryLineCircleIntersectionPos(s, e, _puckPos, G_malletRadius + G_puckRadius);
	if (dblcmp(puckDistance - 1) < 0) {
		Point malletPos = s + (e - s) * puckDistance;
		Point puckDirection = _puckDirection + e - s;
		puckDirection /= abs(puckDirection);

		return std::make_tuple(puckDistance, [=](Point &_malletPos) {
			_malletPos = malletPos;
			_puckDirection = puckDirection;
		});
		
	}
	return std::make_tuple(1, std::function<void(Point&)>());
}

std::tuple<double, std::function<void(Point&)>> Game::TestMalletCollisionWithBoard(const Point & s, const Point & e, const Point p[4])
{
	double boardDistance = QueryBoardDistance(s, e, p);
	if (dblcmp(boardDistance - 1) < 0) {
		Point malletPos = e;
		if (malletPos.x() < p[0].x()) malletPos += p[0].x() - malletPos.x();
		if (malletPos.y() < p[0].y()) malletPos += Point(0, p[0].y() - malletPos.y());
		if (malletPos.x() > p[2].x()) malletPos += p[2].x() - malletPos.x();
		if (malletPos.y() > p[2].y()) malletPos += Point(0, p[2].y() - malletPos.y());
		return std::make_tuple(boardDistance, [=](Point &_malletPos) {
			_malletPos = malletPos;
		});
	}
	return std::make_tuple(1, std::function<void(Point&)>());
}

void Game::MoveMallet(const Point & s, const Point & e, const Point p[4], Point &malletPos)
{
	if (norm(s - e) < G_eps * G_eps) return;

	Point ee;
	//与桌边界交点
	auto boardDistance = TestMalletCollisionWithBoard(s, e, p);
	if (dblcmp(std::get<0>(boardDistance) - 1) < 0) {
		std::get<1>(boardDistance)(ee);
	} else {
		ee = e;
	}

	//Mallet与Puck交点
	auto puckDistance = TestMalletCollisionWithPuck(s, ee);
	if (dblcmp(std::get<0>(puckDistance) - 1) < 0) {
		std::get<1>(puckDistance)(malletPos);
	} else {
		malletPos = ee;
	}


	/*//与桌边界交点
	auto boardDistance = TestMalletCollisionWithBoard(s, e, p);

	auto minDis = puckDistance;
	if (std::get<0>(boardDistance) < std::get<0>(minDis)) minDis = boardDistance;

	if (dblcmp(std::get<0>(minDis) - 1) < 0) {
		std::get<1>(minDis)(malletPos);
		return;
	}*/
}

std::tuple<double, std::function<void()>> Game::TestPuckCollisionWithBoard(const Point & s, const Point & e, const Point p[4])
{
	double res[4];
	QueryBoardDistance(s, e, p, res);
	int minPos = 0;
	double minVal = res[0];
	for (int i = 1; i < 4;i++) {
		if (minVal > res[i]) minVal = res[i], minPos = i;
	}

	if (dblcmp(minVal - 1) >= 0) {
		return std::make_tuple(1, std::function<void()>());
	}
	
	Point reflectVec = p[(minPos + 1) % 4] - p[minPos];
	reflectVec /= abs(reflectVec);
	Point puckDirection = reflectVec * reflectVec / _puckDirection;
	Point puckPos = s + (e - s) * minVal;

	return std::make_tuple(minVal, [=]() {
		_puckDirection = puckDirection;
		_puckPos = puckPos;
		if (minPos == 1 || minPos == 3) TestWin(minPos);
	}
	);
}

std::tuple<double, std::function<void()>> Game::TestPuckCollisionWithMallet(const Point & s, const Point & e, const Point & o)
{
	double malletDistance = QueryLineCircleIntersectionPos(s, e, o, G_malletRadius + G_puckRadius);
	if (dblcmp(malletDistance-1) < 0) {
		Point puckPos = s + (e - s) * malletDistance;
		Point reflectVec = Rotate(o - puckPos);
		reflectVec /= abs(reflectVec);
		Point puckDirection = reflectVec * reflectVec / _puckDirection;
		return std::make_tuple(malletDistance, [=]() {
			_puckDirection = puckDirection;
			_puckPos = puckPos;
		});
	}
	return std::make_tuple(1, std::function<void()>());
}

double Game::MovePuck(double maxTime)
{
	Point s = _puckPos;
	Point e = _puckPos + _puckDirection * maxTime * G_puckSpeed;

	//Puck与Mallet交点
	auto mallet1Distance = TestPuckCollisionWithMallet(s, e, _playerPos);
	auto mallet2Distance = TestPuckCollisionWithMallet(s,e, _opponentPos);

	//Puck与board交点
	auto boardDistance = TestPuckCollisionWithBoard(s, e, _board);

	auto minDis = mallet1Distance;
	if (std::get<0>(mallet2Distance) < std::get<0>(minDis)) minDis = mallet2Distance;
	if (std::get<0>(boardDistance) < std::get<0>(minDis)) minDis = boardDistance;
		
	if (dblcmp(std::get<0>(minDis) - 1) < 0) {
		std::get<1>(minDis)();
		return std::get<0>(minDis) * maxTime;
	}

	_puckPos = e;

	return maxTime;
}

void Game::TestWin(int pos)
{
	if (abs(_puckPos.y()) + G_puckRadius < G_goalWidth / 2) {
		_gameStatus = pos == 1 ? 2 : 1;
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
	 MoveMallet(_playerPos, playerTarget, _playerBoard, _playerPos);
	 MoveMallet(_opponentPos, opponentTarget, _opponentBoard, _opponentPos);

	//进行碰撞检测
	double now = 0;
	int calculateCount = 0;
	while (dblcmp(now - times) < 0 && ++calculateCount < G_calculateMaxTimes) {
		double passTime = MovePuck(std::min(times - now, C_maxTimeInterval));
		now += passTime;

		if (_gameStatus > 0) return _gameStatus;
	}

	//再次放置mallet
	 MoveMallet(_playerPos, playerTarget, _playerBoard, _playerPos);
	 MoveMallet(_opponentPos, opponentTarget, _opponentBoard, _opponentPos);
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
