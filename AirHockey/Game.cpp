#include "stdafx.h"

#include "Constants.h"
#include "Game.h"

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
	_playerPos = Point(x, y);
	return 0;
}

void Game::Restart()
{
	_playerPos = Point(C_malletStartDistance, 0);
	_opponentPos = Point(-C_malletStartDistance, 0);
	_puckPos = Point(0, 0);

	srand((int)time(0));

	_puckDirection = Point(G_tableHeight / 2, rand() * G_goalWidth / RAND_MAX - G_goalWidth / 2);
	_puckDirection /= abs(_puckDirection);
}
