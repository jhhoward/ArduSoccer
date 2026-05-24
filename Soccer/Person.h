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
		Stunned,
		Fallen,
		SlideTackle,
		DiveLeft,
		DiveRight
	};

	uint8_t index;
	int x, y;
	uint8_t z;
	uint8_t team;

	uint8_t direction : 8;
	State state : 8;

	uint8_t animation;
	uint8_t animationFrame;
	uint8_t displayFrame;

	void init(uint8_t index);
	void update();
	void stun(uint8_t frames, bool shouldFall = false);
	void kickBall(int velocityX, int velocityY, int velocityZ);

	bool isOnScreen();

	static void getDirectionOffset(uint8_t direction, int8_t& dx, int8_t& dy);

	bool tryMove(int deltaX, int deltaY);
	bool isColliding();

	static int8_t ballDeltaX, ballDeltaY;
};

#endif

