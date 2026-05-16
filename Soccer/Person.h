#ifndef PERSON_H_
#define PERSON_H_

enum Direction
{
	North,
	NorthEast,
	East,
	SouthEast,
	South,
	SouthWest,
	West,
	NorthWest,
	NoDirection
};

class Person
{
public:
	enum State
	{
		Standing,
		Walking,
		KickRecovery
	};

	uint8_t index;
	int x, y;
	uint8_t team;

	uint8_t direction : 8;
	State state : 8;

	uint8_t animation;
	uint8_t animationFrame;
	uint8_t displayFrame;

	void init(uint8_t index);
	void update();

	static int8_t ballDeltaX, ballDeltaY;
};

#endif

