#include "Engine.h"
#include "Person.h"

const int16_t startingPositions[] PROGMEM =
{
	128, 40,
	64, 64,
	192, 64,
	64, 96,
	192, 96,

	128, 200,
	64, 180,
	192, 180,
	64, 140,
	192, 140,

	128, 128
};

enum FrameNames
{
	SOUTH_STAND = 0,
	SOUTH_WALK1,
	SOUTH_WALK2,
	EAST_STAND,
	EAST_WALK1,
	EAST_WALK2,
	WEST_STAND,
	WEST_WALK1,
	WEST_WALK2,
	NORTH_STAND,
	NORTH_WALK1,
	NORTH_WALK2,
	FALL_RIGHT,
	EAST_TACKLE,
	SOUTH_TACKLE,
	NORTH_TACKLE,
	WEST_TACKLE
};

const uint8_t walkAnimations[] PROGMEM =
{
	NORTH_STAND, NORTH_WALK1, NORTH_STAND, NORTH_WALK2,
	EAST_STAND, EAST_WALK1, EAST_STAND, EAST_WALK2,
	SOUTH_STAND, SOUTH_WALK1, SOUTH_STAND, SOUTH_WALK2,
	WEST_STAND, WEST_WALK1, WEST_STAND, WEST_WALK2,
};

const uint8_t inputToDirection[16] PROGMEM =
{
	NoDirection,	// 0000
	North,			// 0001
	East,			// 0010
	NorthEast,		// 0011
	South,			// 0100
	NoDirection,	// 0101
	SouthEast,		// 0110
	NoDirection,	// 0111
	West,			// 1000
	NorthWest,		// 1001
	NoDirection,	// 1010
	NoDirection,	// 1011
	SouthWest,		// 1100
	NoDirection,	// 1101
	NoDirection,	// 1110
	NoDirection,	// 1111
};

const uint8_t directionToWalkAnimation[] PROGMEM =
{
	0,
	0,
	1,
	2,
	2,
	2,
	3,
	0
};

const int8_t directionToX[] PROGMEM =
{
	0,
	1,
	1,
	1,
	0,
	-1,
	-1,
	-1
};

const int8_t directionToY[] PROGMEM =
{
	-1,
	-1,
	0,
	1,
	1,
	1,
	0,
	-1,
};

int8_t Person::ballDeltaX;
int8_t Person::ballDeltaY;

void Person::init(uint8_t startIndex)
{
	index = startIndex;
	x = pgm_read_word(&startingPositions[startIndex * 2]);
	y = pgm_read_word(&startingPositions[startIndex * 2 + 1]);
	animation = 0;
	state = Person::Standing;
	if (index < PLAYERS_PER_TEAM)
	{
		team = 0;
		direction = South;
		displayFrame = SOUTH_STAND;
	}
	else if (index < PLAYERS_PER_TEAM * 2)
	{
		team = 1;
		direction = North;
		displayFrame = NORTH_STAND;
	}
	else
	{
		team = 2;
		direction = East;
		displayFrame = EAST_STAND;
	}
}

void Person::update()
{
	if (index == engine.personPlayer1 && state != Person::KickRecovery)
	{
		uint8_t input = Platform.readInput();
		
		uint8_t inputDirection = pgm_read_byte(&inputToDirection[input & 0xf]);

		if (inputDirection != NoDirection)
		{
			state = Person::Walking;

			// Slow turn if dribbling the ball
			if (engine.ball.owner == index)
			{
				int directionDelta = inputDirection - direction;
				if (directionDelta > 4 || directionDelta < -4)
				{
					if (inputDirection < direction)
					{
						if (direction == NorthWest)
							direction = North;
						else direction++;
					}
					else if (inputDirection > direction)
					{
						if (direction == North)
							direction = NorthWest;
						else direction--;
					}
				}
				else
				{
					if (inputDirection < direction)
						direction--;
					else if (inputDirection > direction)
						direction++;
				}
			}
			else
			{
				direction = inputDirection;
			}
		}
		else
		{
			state = Person::Standing;
		}


		if (input & Input_Btn_A)
		{
			if (engine.ball.owner == index)
			{
				int8_t deltaX = ((int8_t)pgm_read_byte(&directionToX[direction]));
				int8_t deltaY = ((int8_t)pgm_read_byte(&directionToY[direction]));

				engine.ball.owner = NO_BALL_OWNER;
				engine.ball.velocityX = deltaX * 50;
				engine.ball.velocityY = deltaY * 50;
				engine.ball.velocityZ = 50;

				state = Person::KickRecovery;
				animationFrame = 0;
				animation = pgm_read_byte(&directionToWalkAnimation[direction]);
				displayFrame = pgm_read_byte(&walkAnimations[animation * 4 + 1]);
			}
		}
		if (input & Input_Btn_B)
		{
			engine.ball.owner = index;
		}
	}

	switch (state)
	{
	case Person::Standing:
		animationFrame = 0;
		break;
	case Person::Walking:
		if ((engine.frameCount & 3) == 0)
		{
			animationFrame++;
			if (animationFrame == 4)
			{
				animationFrame = 0;
			}
		}

		//if ((engine.frameCount & 1) == 0)
		{
			x += (int8_t) pgm_read_byte(&directionToX[direction]);
			y += (int8_t) pgm_read_byte(&directionToY[direction]);
		}
		break;
	case Person::KickRecovery:
		animationFrame++;
		if (animationFrame >= KICK_RECOVERY_FRAMES)
		{
			state = Person::Standing;
			animationFrame = 0;
		}
		break;
	}

	if (state == Person::Standing || state == Person::Walking)
	{
		animation = pgm_read_byte(&directionToWalkAnimation[direction]);
		displayFrame = pgm_read_byte(&walkAnimations[animation * 4 + animationFrame]);
	}

	if (engine.ball.owner == index)
	{
		int8_t deltaX = 6 * ((int8_t)pgm_read_byte(&directionToX[direction]));
		int8_t deltaY = 4 * ((int8_t)pgm_read_byte(&directionToY[direction]));

		if (ballDeltaX < deltaX)
			ballDeltaX++;
		else if (ballDeltaX > deltaX)
			ballDeltaX--;
		if (ballDeltaY < deltaY)
			ballDeltaY++;
		else if (ballDeltaY > deltaY)
			ballDeltaY--;

		engine.ball.setPosition(x + ballDeltaX, y + ballDeltaY);
	}
}
