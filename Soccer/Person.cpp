#include "Engine.h"
#include "Person.h"

const int16_t startingPositions[] PROGMEM =
{
	128, 35,
	128, 86,
	64, 112,
	148, 151,
	128, 155,

	128, 282,
	128, 232,
	64, 209,
	192, 209,
	128, 189,

	75, 148
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
	WEST_TACKLE,
	SOUTH_DIVE_RIGHT1,
	SOUTH_DIVE_RIGHT2,
	SOUTH_DIVE_LEFT1,
	SOUTH_DIVE_LEFT2,
	NORTH_DIVE_RIGHT1,
	NORTH_DIVE_RIGHT2,
	NORTH_DIVE_LEFT1,
	NORTH_DIVE_LEFT2
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

const uint8_t goalieDiveZ[] PROGMEM =
{
	0,
	2,
	4,
	6,
	7,
	8,
	8,
	8,
	7,
	6,
	4,
	2,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
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
	state = shouldFall ? Person::Fallen : Person::Stunned;

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
	engine.ball.owner = NULL;
	engine.ball.velocityX = velocityX;
	engine.ball.velocityY = velocityY;
	engine.ball.velocityZ = velocityZ;

	stun(KICK_RECOVERY_FRAMES);
	animation = pgm_read_byte(&directionToWalkAnimation[direction]);
	displayFrame = pgm_read_byte(&walkAnimations[animation * 4 + 1]);
}

void Person::update()
{
	if (state == Person::Stunned || state == Person::Fallen)
	{
		if (animationFrame == 0)
		{
			if (!isColliding())
			{
				state = Person::Standing;
			}
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

			if (engine.ball.owner != this)
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
			if (engine.ball.owner == this)
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
			if (1)
			{
				state = Person::DiveLeft;
				//direction = South;
				animationFrame = 0;
				return;
			}
			if (engine.ball.owner == this)
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
			if (engine.ball.owner == this)
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
		}
	}

	switch (state)
	{
	case Person::Standing:
		animationFrame = 0;
		break;
	case Person::Walking:
		//if ((engine.frameCount & 1) == 0)
		{
			int8_t deltaX, deltaY;
			getDirectionOffset(direction, deltaX, deltaY);

			if (tryMove(deltaX, deltaY))
			{
				if ((engine.frameCount & 3) == 0)
				{
					animationFrame++;
					if (animationFrame == 4)
					{
						animationFrame = 0;
					}
				}
			}
		}
		break;
	case Person::DiveLeft:
		{
			if (animationFrame < GOALIE_DIVE_FRAMES / 2)
			{
				displayFrame = direction == South ? SOUTH_DIVE_LEFT1 : NORTH_DIVE_LEFT1;
				tryMove(-2, 0);
			}
			else if(animationFrame < GOALIE_DIVE_FRAMES)
			{
				displayFrame = direction == South ? SOUTH_DIVE_LEFT2 : NORTH_DIVE_LEFT2;
				tryMove(-1, 0);
			}
			else
			{
				stun(0);
				z = 0;
				return;
			}
			
			z = pgm_read_byte(&goalieDiveZ[animationFrame]);
			animationFrame++;
		}
		break;
	case Person::DiveRight:
		{
			if (animationFrame < GOALIE_DIVE_FRAMES / 2)
			{
				displayFrame = direction == South ? SOUTH_DIVE_RIGHT1 : NORTH_DIVE_RIGHT1;
				tryMove(2, 0);
			}
			else if(animationFrame < GOALIE_DIVE_FRAMES)
			{
				displayFrame = direction == South ? SOUTH_DIVE_RIGHT2 : NORTH_DIVE_RIGHT2;
				tryMove(1, 0);
			}
			else
			{
				stun(0);
				z = 0;
				return;
			}
			
			z = pgm_read_byte(&goalieDiveZ[animationFrame]);
			animationFrame++;
		}
		break;
	case Person::SlideTackle:
		{
			int8_t deltaX, deltaY;
			getDirectionOffset(direction, deltaX, deltaY);
			if (animationFrame < SLIDE_TACKLE_FRAMES / 3)
			{
				tryMove(deltaX, deltaY);
			}
			if (animationFrame < 2 * SLIDE_TACKLE_FRAMES / 3)
			{
				tryMove(deltaX, deltaY);
			}

			// Check if we are fouling anyone
			if (engine.ball.owner != this)
			{
				for (int n = 0; n < PLAYERS_PER_TEAM * 2; n++)
				{
					Person& other = engine.people[n];
					if (other.team != team && engine.ball.owner != &other && other.state != Person::Stunned && other.state != Person::Fallen)
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
	if (team != 2 && engine.ball.z < PERSON_HEIGHT)
	{
		int diffX = engine.ball.x - x;
		int diffY = engine.ball.y - y;

		if (engine.ball.owner == NULL && diffX >= -GET_BALL_DISTANCE && diffX <= GET_BALL_DISTANCE && diffY >= -GET_BALL_DISTANCE && diffY <= GET_BALL_DISTANCE)
		{
			engine.ball.owner = this;
			ballDeltaX = diffX;
			ballDeltaY = diffY;
		}
		else if (engine.ball.owner && engine.ball.owner->team != team && diffX >= -TACKLE_DISTANCE && diffX <= TACKLE_DISTANCE && diffY >= -TACKLE_DISTANCE && diffY <= TACKLE_DISTANCE)
		{
			if (state == Person::SlideTackle)
			{
				engine.ball.owner->stun(SLIDE_TACKLE_RECOVERY_FRAMES, true);
  			}
			else
			{
				engine.ball.owner->stun(TACKLE_RECOVERY_FRAMES);
			}
			engine.ball.owner = this;
			ballDeltaX = diffX;
			ballDeltaY = diffY;
		}
	}

	if (engine.ball.owner == this)
	{
		int8_t deltaX, deltaY;
		getDirectionOffset(direction, deltaX, deltaY);

		if (state == Person::DiveRight)
		{
			// Saving during a dive
			engine.ball.setPosition(x + 2, y + deltaY, z + 3);
		}
		else if (state == Person::DiveLeft)
		{
			// Saving during a dive
			engine.ball.setPosition(x - 2, y + deltaY, z + 3);
		}
		else
		{
			// Dribbling the ball
			int ballZ = engine.ball.z;

			if (0)
			{
				// Goalie Holding ball in hands
				ballZ = z + 4;
				deltaX *= 5;
				deltaY *= 2;
			}
			else
			{
				// Dribbling on floor
				deltaX *= 6;
				deltaY *= 4;
			}

			if (ballDeltaX < deltaX)
				ballDeltaX++;
			else if (ballDeltaX > deltaX)
				ballDeltaX--;
			if (ballDeltaY < deltaY)
				ballDeltaY++;
			else if (ballDeltaY > deltaY)
				ballDeltaY--;

			if (team == 0)
			{
				engine.personPlayer1 = index;
			}



			engine.ball.setPosition(x + ballDeltaX, y + ballDeltaY, ballZ);
		}
	}
}

void Person::getDirectionOffset(uint8_t direction, int8_t& dx, int8_t& dy)
{
	dx = ((int8_t)pgm_read_byte(&directionToX[direction]));
	dy = ((int8_t)pgm_read_byte(&directionToY[direction]));
}

bool Person::tryMove(int deltaX, int deltaY)
{
	bool moved = false;

	if (deltaX != 0)
	{
		x += deltaX;
		if (isColliding())
		{
			x -= deltaX;
		}
		else moved = true;
	}
	if (deltaY != 0)
	{
		y += deltaY;
		if (isColliding())
		{
			y -= deltaY;
		}
		else moved = true;
	}

	return moved;
}

bool Person::isColliding()
{
	// Left post
	if (x >= LEFT_POST_X1 && x <= LEFT_POST_X2)
	{
		// Top goal
		if (y >= TOP_GOAL_POST_Y1 && y <= TOP_GOAL_POST_Y2)
		{
			return true;
		}
		// Bottom goal
		if (y >= BOTTOM_GOAL_POST_Y1 && y <= BOTTOM_GOAL_POST_Y2)
		{
			return true;
		}
	}

	// Right post
	if (x >= RIGHT_POST_X1 && x <= RIGHT_POST_X2)
	{
		// Top goal
		if (y >= TOP_GOAL_POST_Y1 && y <= TOP_GOAL_POST_Y2)
		{
			return true;
		}
		// Bottom goal
		if (y >= BOTTOM_GOAL_POST_Y1 && y <= BOTTOM_GOAL_POST_Y2)
		{
			return true;
		}
	}

	if (x >= GOAL_NET_X1 && x <= GOAL_NET_X2 && y >= TOP_GOAL_NET_Y1 && y <= TOP_GOAL_NET_Y2)
		return true;

	if (x >= GOAL_NET_X1 && x <= GOAL_NET_X2 && y >= BOTTOM_GOAL_NET_Y1 && y <= BOTTOM_GOAL_NET_Y2)
		return true;

	for (int n = 0; n < NUM_PEOPLE; n++)
	{
		Person& other = engine.people[n];
		if (&other != this && other.state != Person::Fallen)
		{
			if (y >= other.y - 1 && y <= other.y + 1 && x >= other.x - PERSON_HALF_WIDTH && x <= other.x + PERSON_HALF_WIDTH)
			{
				return true;
			}
		}
	}

	return false;
}