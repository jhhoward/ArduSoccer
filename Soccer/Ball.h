#ifndef BALL_H_
#define BALL_H_

#include <stdint.h>
#include "Defines.h"

#define NO_BALL_OWNER 0xff

class Ball
{
public:
	void update();
	bool setPosition(int x, int y, int z = 0);

	int x, y, z;
	int16_t fixedX, fixedY, fixedZ;
	int16_t velocityX, velocityY, velocityZ;

	class Person* owner;
	uint16_t ownerTimer;

	bool isInsideTopNet();
	bool isInsideBottomNet();

private:
	bool isColliding();
};

#endif

