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
		FreeKick,
		Scored
	};

	void reset();
	void update();

	void setState(Match::State newState);
	bool shouldAllowFreeMovement();
	bool shouldAllowKicking();

	void setupKickOff(Team* team);	
	void onKick();

	Person* electedKicker;
	Team* electedTeam;

	State state;
	uint32_t timeInState;
};

#endif
