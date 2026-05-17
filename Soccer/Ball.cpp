#include "Ball.h"

void Ball::update()
{
	// If nobody is controlling the ball, let physics control it
	if (owner == NO_BALL_OWNER)
	{
		fixedX += velocityX;
		fixedY += velocityY;
		x = fixedX >> FIXED_SHIFT;
		y = fixedY >> FIXED_SHIFT;

		if (fixedZ == 0)
		{
			if (velocityX < -BALL_FRICTION)
				velocityX += BALL_FRICTION;
			else if (velocityX > BALL_FRICTION)
				velocityX -= BALL_FRICTION;
			else
				velocityX = 0;

			if (velocityY < -BALL_FRICTION)
				velocityY += BALL_FRICTION;
			else if (velocityY > BALL_FRICTION)
				velocityY -= BALL_FRICTION;
			else
				velocityY = 0;
		}
	}

	// Apply Z physics
	fixedZ += velocityZ;

	if (fixedZ < 0)
	{
		fixedZ = 0;
		if (velocityZ < -(2 << FIXED_SHIFT))
		{
			velocityZ = -velocityZ / 2;
		}
		else
		{
			velocityZ = 0;
		}
	}
	else if (fixedZ > 0)
	{
		velocityZ -= BALL_GRAVITY;
	}

	z = fixedZ >> FIXED_SHIFT;

	// Clamp to play field
	if (x < 0)
	{
		x = 0;
		fixedX = x << FIXED_SHIFT;
		if (velocityX < 0)
			velocityX = -(velocityX >> 1);
	}
	if (x >= BACKGROUND_WIDTH)
	{
		x = BACKGROUND_WIDTH - 1;
		fixedX = x << FIXED_SHIFT;
		if (velocityX > 0)
			velocityX = -(velocityX >> 1);
	}
	if (y < 0)
	{
		y = 0;
		fixedY = y << FIXED_SHIFT;
		if (velocityY < 0)
			velocityY = -(velocityY >> 1);
	}
	if (y >= BACKGROUND_HEIGHT)
	{
		y = BACKGROUND_HEIGHT - 1;
		fixedY = y << FIXED_SHIFT;
		if (velocityY > 0)
			velocityY = -(velocityY >> 1);
	}

}

void Ball::setPosition(int newX, int newY, int newZ)
{
	x = newX;
	y = newY;
	z = newZ;

	fixedX = x << FIXED_SHIFT;
	fixedY = y << FIXED_SHIFT;
	fixedZ = z << FIXED_SHIFT;
}
