#include "Ball.h"

void Ball::update()
{
	if (owner == NO_BALL_OWNER)
	{
		fixedX += velocityX;
		fixedY += velocityY;
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
		else
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


			//velocityX /= 2;
			//velocityY /= 2;
		}

		x = fixedX >> FIXED_SHIFT;
		y = fixedY >> FIXED_SHIFT;
		z = fixedZ >> FIXED_SHIFT;
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
