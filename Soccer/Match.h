#ifndef MATCH_H_
#define MATCH_H_

#include "Defines.h"

class Match
{
public:
	enum State
	{
		KickOff,
		Playing,
		Corner,
		ThrowIn,
		GoalKick,
		Penalty,
		FreeKick
	};

	void reset();
	void update();

	State state;
};

#endif
