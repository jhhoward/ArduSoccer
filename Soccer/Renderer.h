#ifndef RENDERER_H_
#define RENDERER_H_

#include "Defines.h"

enum
{
	BALL_DRAWABLE = NUM_PEOPLE,
	BALL_SHADOW_DRAWABLE,
	UPPER_GOAL_DRAWABLE,
	LOWER_GOAL_DRAWABLE,
	NUM_DRAWABLES
};

class Renderer
{
public:
	Renderer();

	void draw();

	void drawPerson(int index);
	void drawBall();
	void drawBallShadow();
	void drawLowerGoal();
	void drawUpperGoal();

	uint8_t drawOrder[NUM_DRAWABLES];

	int getSortPosition(uint8_t drawable);
};

#endif
