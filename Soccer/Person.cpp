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
	East,			// 0111
	West,			// 1000
	NorthWest,		// 1001
	NoDirection,	// 1010
	North,			// 1011
	SouthWest,		// 1100
	West,			// 1101
	South,			// 1110
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

const uint8_t directionToSlideTackleFrame[] PROGMEM =
{
	NORTH_TACKLE,
	EAST_TACKLE,
	EAST_TACKLE,
	EAST_TACKLE,
	SOUTH_TACKLE,
	WEST_TACKLE,
	WEST_TACKLE,
	WEST_TACKLE
};

const uint8_t directionToFallFrame[] PROGMEM =
{
	FALL_RIGHT,
	FALL_RIGHT, 
	FALL_RIGHT,
	FALL_RIGHT,
	FALL_RIGHT,
	FALL_RIGHT,
	FALL_RIGHT,
	FALL_RIGHT,
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

void Person::stun(uint8_t frames, bool shouldFall)
{
	animationFrame = frames;
	state = Person::Stunned;

	if (shouldFall)
	{
		displayFrame = pgm_read_byte(&directionToFallFrame[direction]);
	}
}

bool Person::isOnScreen()
{
	int displayX = x - engine.camera.x;
	int displayY = y - engine.camera.y;

	return displayX >= -4 && displayY >= 0 && displayX < DISPLAYWIDTH + 4 && displayY < DISPLAYHEIGHT + 16;
}

void Person::kickBall(int velocityX, int velocityY, int velocityZ)
{
	engine.ball.owner = NO_BALL_OWNER;
	engine.ball.velocityX = velocityX;
	engine.ball.velocityY = velocityY;
	engine.ball.velocityZ = velocityZ;

	stun(KICK_RECOVERY_FRAMES);
	animation = pgm_read_byte(&directionToWalkAnimation[direction]);
	displayFrame = pgm_read_byte(&walkAnimations[animation * 4 + 1]);
}

void Person::update()
{
	if (state == Person::Stunned)
	{
		if (animationFrame == 0)
		{
			state = Person::Standing;
		}
		else animationFrame--;
		return;
	}

	bool canControl = state == Person::Standing || state == Person::Walking;

	if (canControl)
	{
		uint8_t input = 0;

		if (index == engine.personPlayer1)
		{
			input = Platform.readInput();

			if (engine.ball.owner != index)
			{
				bool swapBecauseOffScreen = false;

				if (!isOnScreen())
				{
					// Check if any other players on the team are on screen instead
					for (int n = 0; n < NUM_PEOPLE; n++)
					{
						Person& other = engine.people[n];
						if (other.team == team && other.isOnScreen())
						{
							swapBecauseOffScreen = true;
							break;
						}
					}
				}

				if (swapBecauseOffScreen || (Platform.readInputDown() & Input_Btn_A))
				{
					// Swap selected player if pressing A or off screen
					engine.cycleSelectedPerson();
				}
			}
		}

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
				// Pass
				int8_t deltaX = ((int8_t)pgm_read_byte(&directionToX[direction]));
				int8_t deltaY = ((int8_t)pgm_read_byte(&directionToY[direction]));

				kickBall(deltaX * 50, deltaY * 50, 50);
				return;
			}
		}
		if (input & Input_Btn_B)
		{
			if (engine.ball.owner == index)
			{
				// Shoot
				int8_t deltaX = ((int8_t)pgm_read_byte(&directionToX[direction]));
				int8_t deltaY = ((int8_t)pgm_read_byte(&directionToY[direction]));

				kickBall(deltaX * 60, deltaY * 60, 75);
				return;
			}
			else
			{
				// Slide tackle
				animationFrame = 0;
				state = Person::SlideTackle;
				displayFrame = pgm_read_byte(&directionToSlideTackleFrame[direction]);
			}
			//engine.ball.owner = index;
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
			int8_t deltaX, deltaY;
			getDirectionOffset(direction, deltaX, deltaY);
			x += deltaX;
			y += deltaY;
		}
		break;
	case Person::SlideTackle:
		{
			int8_t deltaX, deltaY;
			getDirectionOffset(direction, deltaX, deltaY);
			if (animationFrame < SLIDE_TACKLE_FRAMES / 3)
			{
				x += deltaX * 2;
				y += deltaY * 2;
			}
			else if (animationFrame < 2 * SLIDE_TACKLE_FRAMES / 3)
			{
				x += deltaX;
				y += deltaY;
			}

			// Check if we are fouling anyone
			if (engine.ball.owner != index)
			{
				for (int n = 0; n < PLAYERS_PER_TEAM * 2; n++)
				{
					Person& other = engine.people[n];
					if (other.team != team && engine.ball.owner != other.index && other.state != Person::Stunned)
					{
						int diffX = other.x - x;
						int diffY = other.y - y;

						if (diffX >= -TACKLE_DISTANCE && diffX <= TACKLE_DISTANCE && diffY >= -TACKLE_DISTANCE && diffY <= TACKLE_DISTANCE)
						{
							other.stun(SLIDE_TACKLE_RECOVERY_FRAMES, true);
						}
					}
				}
			}

			animationFrame++;
			if (animationFrame >= SLIDE_TACKLE_FRAMES)
			{
				state = Person::Standing;
				animationFrame = 0;
			}
		}
		break;
	}

	// Animate standing / walking
	if (state == Person::Standing || state == Person::Walking)
	{
		animation = pgm_read_byte(&directionToWalkAnimation[direction]);
		displayFrame = pgm_read_byte(&walkAnimations[animation * 4 + animationFrame]);
	}

	// Check for tackling / gaining control of the ball
	if (team != 2)
	{
		int diffX = engine.ball.x - x;
		int diffY = engine.ball.y - y;

		if (engine.ball.owner == NO_BALL_OWNER && diffX >= -GET_BALL_DISTANCE && diffX <= GET_BALL_DISTANCE && diffY >= -GET_BALL_DISTANCE && diffY <= GET_BALL_DISTANCE)
		{
			engine.ball.owner = index;
			ballDeltaX = diffX;
			ballDeltaY = diffY;
		}
		else if (engine.people[engine.ball.owner].team != team && diffX >= -TACKLE_DISTANCE && diffX <= TACKLE_DISTANCE && diffY >= -TACKLE_DISTANCE && diffY <= TACKLE_DISTANCE)
		{
			if (state == Person::SlideTackle)
			{
				engine.people[engine.ball.owner].stun(SLIDE_TACKLE_RECOVERY_FRAMES, true);
  			}
			else
			{
				engine.people[engine.ball.owner].stun(TACKLE_RECOVERY_FRAMES);
			}
			engine.ball.owner = index;
			ballDeltaX = diffX;
			ballDeltaY = diffY;
		}
	}

	// Dribbling the ball
	if (engine.ball.owner == index)
	{
		int8_t deltaX, deltaY;
		getDirectionOffset(direction, deltaX, deltaY);

		deltaX *= 6;
		deltaY *= 4;

		if (ballDeltaX < deltaX)
			ballDeltaX++;
		else if (ballDeltaX > deltaX)
			ballDeltaX--;
		if (ballDeltaY < deltaY)
			ballDeltaY++;
		else if (ballDeltaY > deltaY)
			ballDeltaY--;

		engine.ball.setPosition(x + ballDeltaX, y + ballDeltaY, engine.ball.z);

		if (team == 0)
		{
			engine.personPlayer1 = index;
		}
	}
}

void Person::getDirectionOffset(uint8_t direction, int8_t& dx, int8_t& dy)
{
	dx = ((int8_t)pgm_read_byte(&directionToX[direction]));
	dy = ((int8_t)pgm_read_byte(&directionToY[direction]));
}
