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

	void setState(Match::State newState);

	State state;
	uint32_t timeInState;
};

#endif
