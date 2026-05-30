#ifndef MATCH_H_
#define MATCH_H_

#include "Defines.h"

#define MAX_SCORE_TEXT_SIZE 8

enum
{
	FIRST_HALF = 0,
	SECOND_HALF = 1
};

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
		Scored,
		HalfTime,
		MatchEnd
	};

	void reset();
	void update();

	void setState(Match::State newState);
	bool shouldAllowFreeMovement();
	bool shouldAllowKicking();

	void setupKickOff(Team* team);	
	void onKick();
	void onGoalScored(Team* team);
	void onHalfTime();
	void onMatchEnd();

	void regenerateScoreText();
	char* getMatchTimeString();

	char scoreText[MAX_SCORE_TEXT_SIZE];

	Person* electedKicker;
	Team* electedTeam;

	State state;
	uint32_t timeInState;

	uint32_t matchTimer;
	uint8_t matchHalf;
	
	char* printInt(char* buffer, uint8_t number, bool leadingZeroes = false);
};

#endif
