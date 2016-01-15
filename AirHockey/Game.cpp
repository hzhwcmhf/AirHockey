#include "stdafx.h"


#include "Game.h"

//计算几何部分

//叉积 aXb
inline double Cross(const Point &a, const Point &b)
{
	return imag(conj(a) * b);
}
//叉积 (a-o)X(b-o)
inline double Cross(const Point &a, const Point &o, const Point &b)
{
	return Cross(a - o, b - o);
}
//向量逆时针旋转90度
inline Point Rotate(const Point &a)
{
	return a * Point(0, 1);
}
//在误差范围内判断大于小于等于
inline int dblcmp(const double &x)
{
	if (x > G_eps) return 1;
	if (x < -G_eps) return -1;
	return 0;
}
//求直线ab、cd交点
inline Point QueryLinesIntersection(const Point &a, const Point &b, const Point &c, const Point &d)
{
	double S1 = Cross(c, a, b);
	double S2 = Cross(b, a, d);
	//面积法
	return c + (d - c) * S1 / (S1 + S2);
}
//求点c在直线ab上的位置，即c=a+(b-a)*ans
inline double QueryPointInLine(const Point &a, const Point &b, const Point &c)
{
	Point ans = (c - a) / (b - a);
	return real(ans) > 0 ? abs(ans) : -abs(ans);
}

//求直线和圆的交点，返回是否<相交，交点1，交点2>。交点顺序由a到b方向去顶。相切时忽略
inline std::tuple<bool,Point,Point> QueryLineCircleIntersection(const Point &a, const Point &b, const Point &o, double r)
{
	Point h = QueryLinesIntersection(a, b, o, o + Rotate(b - a));//垂足
	double perpendicularDis2 = norm(h - o);//圆心垂足距离
	double dis2 = r * r - perpendicularDis2;//交点垂足距离
	if (dis2 < G_eps*G_eps) return make_tuple(false, Point(), Point());
	Point disVec = (b - a) / abs(b - a) * sqrt(dis2);
	return make_tuple(true, h - disVec, h + disVec);
}

//求圆和直线的第一交点在直线上的位置
//如果交点是直线正方向则返回位置，否则返回1
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
		}else if (pos > -G_eps) {	//第一交点在a处
			if (QueryPointInLine(a, b, secondIntersection) > G_eps)//若两交点不重合（否则为相切情况）
				return 0;
		}
	}
	return 1;
}

//求直线和直线的交点在直线ab上的位置
//如果交点在直线正方向就返回位置，否则返回1
inline double QueryLinesIntersectionPos(const Point &a, const Point &b, const Point &c, const Point &d)
{
	if (dblcmp(Cross(b - a, d - c)) <= 0) return 1;
	//面积法
	double S1 = Cross(a, c, d);
	double S2 = Cross(d, c, b);
	double ans = S1 / (S1 + S2);
	if (dblcmp(ans) < 0) return 1;
	return ans;
}


//求s到e正方向上到边界的距离（按和ab的比例为单位），存入res
void Game::QueryBoardDistance(const Point & s, const Point & e, const Point p[4], double res[4])
{
	for (int i = 0;i < 4;i++) {
		res[i] = QueryLinesIntersectionPos(s, e, p[i], p[(i+1)%4]);
	}

}

//求s到e正方向上到边界的距离（按和ab的比例为单位）中的最小值
double Game::QueryBoardDistance(const Point & s, const Point & e, const Point p[4])
{
	double res[4];
	QueryBoardDistance(s, e, p, res);
	return std::min(std::min(res[0], res[1]), std::min(res[2], res[3]));
}

//求曲棍从s移动到e是否会撞到冰球
//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
//返回碰撞后对曲棍和冰球的影响函数
std::tuple<double, std::function<void(Point&)>> Game::TestMalletCollisionWithPuck(const Point & s, const Point & e)
{
	double puckDistance = QueryLineCircleIntersectionPos(s, e, _puckPos, G_malletRadius + G_puckRadius);
	if (dblcmp(puckDistance - 1) < 0) {
		//曲棍位置变化到碰撞位置
		Point malletPos = s + (e - s) * puckDistance;
		//冰球受到推力
		Point puckDirection = _puckDirection;
		if (QueryLineCircleIntersectionPos(_puckPos, _puckPos + puckDirection, malletPos, G_malletRadius + G_puckRadius)) {
			puckDirection += e - s;
			puckDirection /= abs(puckDirection);
		}

		return std::make_tuple(puckDistance, [=](Point &_malletPos) {
			_malletPos = malletPos;
			_puckDirection = puckDirection;
		});
		
	}
	return std::make_tuple(1, std::function<void(Point&)>());
}

//求曲棍从s移动到e是否会撞到边界
//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
//返回碰撞后对曲棍的影响函数
std::tuple<double, std::function<void(Point&)>> Game::TestMalletCollisionWithBoard(const Point & s, const Point & e, const Point p[4])
{
	double boardDistance = QueryBoardDistance(s, e, p);
	if (dblcmp(boardDistance - 1) < 0) {
		//若出界，修正回界内
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

//移动曲棍从s到e，p为边界，malletPos为移动后的曲棍位置（输出变量）
void Game::MoveMallet(const Point & s, const Point & e, const Point p[4], Point &malletPos)
{
	//若基本不变则不管
	if (norm(s - e) < G_eps * G_eps) return;

	Point ee;
	//与桌边界交点
	//先将桌外的目标修正到桌内
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

}

//求冰球从s移动到e是否会撞到边界
//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
//返回碰撞后对冰球的影响、胜负的判断函数
std::tuple<double, std::function<void()>> Game::TestPuckCollisionWithBoard(const Point & s, const Point & e, const Point p[4])
{
	double res[4];
	QueryBoardDistance(s, e, p, res);
	int minPos = 0;
	double minVal = res[0];
	for (int i = 1; i < 4;i++) {
		if (minVal > res[i]) minVal = res[i], minPos = i;
	}

	//s到e无碰撞
	if (dblcmp(minVal - 1) >= 0) {
		return std::make_tuple(1, std::function<void()>());
	}
	
	//计算反射
	Point reflectVec = p[(minPos + 1) % 4] - p[minPos];
	reflectVec /= abs(reflectVec);
	Point puckDirection = reflectVec * reflectVec / _puckDirection;
	Point puckPos = s + (e - s) * minVal;

	return std::make_tuple(minVal, [=]() {
		_puckDirection = puckDirection;
		_puckPos = puckPos;
		//与边界碰撞检测胜负
		if (minPos == 1 || minPos == 3) TestWin(minPos);
	}
	);
}


//求冰球从s移动到e是否会撞到曲棍
//返回碰撞距离（按和se的比例为单位）（若不碰撞返回1）
//返回碰撞后对冰球的影响函数
std::tuple<double, std::function<void()>> Game::TestPuckCollisionWithMallet(const Point & s, const Point & e, const Point & o)
{
	double malletDistance = QueryLineCircleIntersectionPos(s, e, o, G_malletRadius + G_puckRadius);
	if (dblcmp(malletDistance-1) < 0) {
		//计算反射
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

//移动曲棍，输入模拟运行时间
//若有碰撞，处理碰撞，返回碰撞时间
//无碰撞返回输入时间
double Game::MovePuck(double maxTime)
{
	Point s = _puckPos;
	Point e = _puckPos + _puckDirection * maxTime * G_puckSpeed;

	//Puck与Mallet交点
	auto mallet1Distance = TestPuckCollisionWithMallet(s, e, _playerPos);
	auto mallet2Distance = TestPuckCollisionWithMallet(s,e, _opponentPos);

	//Puck与board交点
	auto boardDistance = TestPuckCollisionWithBoard(s, e, _board);

	//求第一碰撞点
	auto minDis = mallet1Distance;
	if (std::get<0>(mallet2Distance) < std::get<0>(minDis)) minDis = mallet2Distance;
	if (std::get<0>(boardDistance) < std::get<0>(minDis)) minDis = boardDistance;
	
	//若有碰撞，则运行返回的函数，进行碰撞计算
	if (dblcmp(std::get<0>(minDis) - 1) < 0) {
		std::get<1>(minDis)();
		return std::get<0>(minDis) * maxTime;
	}

	_puckPos = e;

	return maxTime;
}

//碰撞后检测是否胜利
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

//运行times时间(ms)，己方操作为指向（x,y)
int Game::Run(int times, double x, double y)
{
	Point playerTarget = Point(x, y);
	//获取AI操作
	Point opponentTarget = _ai->QueryAction(times);

	/*_puckDirection = Point(-1, 0);
	_puckPos = Point(x, y);*/


	//先移动一次mallet
	 MoveMallet(_playerPos, playerTarget, _playerBoard, _playerPos);
	 MoveMallet(_opponentPos, opponentTarget, _opponentBoard, _opponentPos);

	//移动puck，进行碰撞检测
	double now = 0;
	int calculateCount = 0;
	//重复碰撞检测操作，一直到模拟到指定时间，或碰撞次数过多
	while (dblcmp(now - times) < 0 && ++calculateCount < G_calculateMaxTimes) {
		//double passTime = MovePuck(std::min(times - now, C_maxTimeInterval));
		double passTime = MovePuck(times - now);
		now += passTime;

		if (_gameStatus > 0) return _gameStatus;
	}

	//再次移动mallet
	 MoveMallet(_playerPos, playerTarget, _playerBoard, _playerPos);
	 MoveMallet(_opponentPos, opponentTarget, _opponentBoard, _opponentPos);
	return 0;
}

//重新开始游戏
void Game::Restart()
{
	//初始化位置、状态
	_gameStatus = 0;
	_playerPos = Point(G_malletStartDistance, 0);
	_opponentPos = Point(-G_malletStartDistance, 0);
	_puckPos = Point(0, 0);

	//随机出射角度
	srand((int)time(0));
	_puckDirection = Point(G_tableWidth / 2, rand() * G_tableHeight / RAND_MAX - G_tableHeight / 2);
	_puckDirection /= abs(_puckDirection);
	if (rand() & 1) _puckDirection = -_puckDirection;

	//重新生成AI
	if (_ai) delete _ai;
	if(_aiLevel  == 0)
		_ai = new SimpleAI(this);
	else if(_aiLevel == 1)
		_ai = new NormalAI(this);
	else if (_aiLevel == 2)
		_ai = new CrazyAI(this);
}


//简单AI：沿路径巡逻
Point SimpleAI::QueryAction(int times)
{
	//std::cerr << times << " " << _nowStep << std::endl;
	_nowStep += times;
	if ((_nowStep / C_step) & 1) {
		return C_s + (C_e - C_s) * (double)(_nowStep % C_step) / (double)C_step;
	}else {
		return C_e + (C_s - C_e) * (double)(_nowStep % C_step) / (double)C_step;
	}
}

//普通AI：向冰球移动
Point NormalAI::QueryAction(int times)
{
	Point mallet = _game->GetOpponentMalletPosition();
	Point puck = _game->GetPuckPosition();

	//若冰球被逼到角落，避免卡死，进行特判移动
	if(abs(puck - _game->_board[0]) < G_malletRadius * 2 || 
		abs(puck - _game->_board[3]) < G_malletRadius * 2)
		puck = Point(-G_tableWidth/2, 0);

	//若太接近则不变
	if (abs(mallet - puck) < G_eps) return mallet;

	return mallet + (puck - mallet) / abs(puck - mallet) * (double)times * C_speed;
}

//疯狂AI：进行球路判断
Point CrazyAI::QueryAction(int times)
{
	Point mallet = _game->GetOpponentMalletPosition();
	Point puck = _game->GetPuckPosition();
	Point target;
	
	double speed = C_speed;

	
	if (puck.x() < -G_tableWidth / 6 - G_malletRadius) {
		//若冰球在AI能活动的范围内
		
		Point goal = Point(-G_tableWidth/2 + G_malletRadius, 0);//AI的球门中心位置

		if (_game->_puckDirection.x() > 0) {
			//冰球背离球门移动
			if (abs(puck - goal) < abs(mallet - goal)) {
				//如果AI的曲棍比冰球离球门远，则躲开，避免挡住冰球。
				Point a = puck;
				Point b = puck + _game->_puckDirection;
				if (Cross(a, b, mallet) < 0) target = mallet + Rotate(b - a);
				else target = mallet + Rotate(a - b);
				//std::cerr << "4" << std::endl;
			} else {
				//否则向冰球撞去，速度改为高速
				target = puck;
				speed = C_highSpeed;
				//std::cerr << "3" << std::endl;
			}
		} else {
			//冰球向球门运动
			if (abs(puck - goal) > abs(mallet - goal)) {	
				//如果AI的曲棍比冰球离球门近，则回球门进行防守，速度改为高速
				//std::cerr << "1" << std::endl;
				target = goal;
				speed = C_highSpeed;
			} else {
				//否则背向冰球运动，避免挡住反弹的冰球
				//std::cerr << "2" << std::endl;
				target = mallet + (mallet - puck) * 10.;
				speed = C_slowSpeed;
				/*if (abs(puck - _game->_board[0]) < G_malletRadius * 2 ||
					abs(puck - _game->_board[3]) < G_malletRadius * 2)
					puck = Point(-G_tableWidth, 0);*/
			}
		}

	} else {
		//std::cerr << "5" << std::endl;
		//若冰球不在AI能活动的范围内

		//判断冰球球路与己方底线的交点
		Point p = QueryLinesIntersection(puck, puck + _game->_puckDirection, _game->_board[0], _game->_board[3]);
		if (QueryPointInLine(puck, puck + _game->_puckDirection, p) > 0) {
			double y = p.y();
			//可以求出一次反射后的位置
			if (y > _game->_board[3].y()) y = _game->_board[3].y() * 2 - y;
			if (y < _game->_board[0].y()) y = _game->_board[0].y() * 2 - y;
			//如果可能进球，速度改为急速
			if (y > -G_goalWidth / 2 && y < G_goalWidth / 2) speed = C_superSpeed;
		}

		//向冰球移动
		target = puck;
	}

	if (abs(mallet - target) < times * speed) return target;

	return mallet + (target - mallet) / abs(target - mallet) * (double)times * speed;
}