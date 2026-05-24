#ifndef TEAM_H_
#define TEAM_H_

#include <stdint.h>
#include "Defines.h"
class Person;

class Team
{
public:
	void init(Person* players);
	void update();

	Person* getClosestPlayer(int16_t x, int16_t y);

	Person* players;

	void calculateFormationPosition(uint8_t index, int16_t& outX, int16_t& outY);
	bool isTopHalf();

	int16_t formationOffsetX, formationOffsetY;
	const int16_t* formation;
};

#endif
