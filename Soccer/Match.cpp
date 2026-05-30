#include "Engine.h"
#include "Match.h"

void Match::reset()
{
	for (uint8_t n = 0; n < NUM_PEOPLE; n++)
	{
		engine.people[n].init(n);
	}

	matchTimer = 0;
	matchHalf = 0;
	setupKickOff(&engine.teams[WHITE_TEAM]);
	engine.renderer.showLargeMessage(PSTR("KICK OFF!"));
	engine.setCameraFocus(CENTER_MARK_X, CENTER_MARK_Y);
	regenerateScoreText();
}

void Match::onGoalScored(Team* team)
{
	team->score++;
	setState(Match::Scored);
	electedTeam = team;
	
	if (engine.ball.lastOwner && engine.ball.lastOwner->getTeam() == team)
	{
		electedKicker = engine.ball.lastOwner;
	}

	regenerateScoreText();
	engine.renderer.showLargeMessage(PSTR("GOAL!"));
}

void Match::onHalfTime()
{
	setState(Match::HalfTime);
	engine.renderer.showLargeMessage(PSTR("HALF TIME"));
	matchHalf = SECOND_HALF;
}

void Match::onMatchEnd()
{
	setState(Match::MatchEnd);

	if (engine.teams[0].score == engine.teams[1].score)
	{
		engine.renderer.showLargeMessage(PSTR("DRAW!"));
	}
	else
	{
		int winningTeam = engine.teams[0].score < engine.teams[1].score ? 1 : 0;

		if (engine.teams[winningTeam].controllerType == Team::LocalPlayer)
		{
			engine.renderer.showLargeMessage(PSTR("YOU WIN!"));
		}
		else
		{
			engine.renderer.showLargeMessage(PSTR("YOU LOSE!"));
		}
	}
}

void Match::update()
{
	switch(state)
	{
	case Match::Playing:
		if (!engine.ball.owner || !engine.ball.owner->isHoldingBall())
		{
			if (engine.ball.isInsideTopNet())
			{
				if (engine.teams[WHITE_TEAM].isTopHalf())
				{
					onGoalScored(&engine.teams[BLACK_TEAM]);
				}
				else
				{
					onGoalScored(&engine.teams[WHITE_TEAM]);
				}
			}
			if (engine.ball.isInsideBottomNet())
			{
				if (engine.teams[WHITE_TEAM].isTopHalf())
				{
					onGoalScored(&engine.teams[WHITE_TEAM]);
				}
				else
				{
					onGoalScored(&engine.teams[BLACK_TEAM]);
				}
			}
		}
		matchTimer++;

		if (matchHalf == FIRST_HALF && matchTimer > engine.settings.matchHalfLength * 60 * TARGET_FRAMERATE)
		{
			onHalfTime();
		}
		if (matchHalf == SECOND_HALF && matchTimer > engine.settings.matchHalfLength * 60 * TARGET_FRAMERATE * 2)
		{
			onMatchEnd();
		}

		break;

	case Match::Scored:
		if (timeInState > 60 * 3)
		{
			if (electedTeam == &engine.teams[WHITE_TEAM])
			{
				setupKickOff(&engine.teams[BLACK_TEAM]);
			}
			else
			{
				setupKickOff(&engine.teams[WHITE_TEAM]);
			}
			
			engine.renderer.showLargeMessage(nullptr);
		}
		break;

	case Match::HalfTime:
		if (timeInState > 60 * 3)
		{
			setupKickOff(&engine.teams[BLACK_TEAM]);
			engine.renderer.showLargeMessage(PSTR("2ND HALF"));
		}
		break;

	case Match::MatchEnd:
		if (timeInState > 60 * 3)
		{
			reset();
		}
		break;
	}

	timeInState++;
}

void Match::setState(Match::State newState)
{
	state = newState;
	timeInState = 0;
	electedKicker = nullptr;
	electedTeam = nullptr;
	engine.ball.setOwner(nullptr);
}

bool Match::shouldAllowFreeMovement()
{
	return state == Match::Playing;
}

bool Match::shouldAllowKicking()
{
	if (engine.renderer.isShowingLargeMessage())
	{
		return false;
	}
	switch (state)
	{
	case Match::Scored:
		return false;
	case Match::Playing:
		return true;
	default:
		return timeInState > 60;
	}
}

void Match::onKick()
{
	switch (state)
	{
	default:
		setState(Match::Playing);
		break;
	case Match::Playing:
		break;
	}
}

void Match::setupKickOff(Team* team)
{
	engine.ball.setPosition(CENTER_MARK_X, CENTER_MARK_Y);
	setState(Match::KickOff);

	electedTeam = team;
	electedKicker = &team->players[PLAYERS_PER_TEAM - 1];

	for (int n = 0; n < PLAYERS_PER_TEAM * 2; n++)
	{
		Person& person = engine.people[n];
		int16_t formationX, formationY;
		person.getTeam()->calculateFormationPosition(person.index, formationX, formationY);
		person.x = formationX;
		person.y = formationY;
		person.direction = person.getTeam()->isTopHalf() ? South : North;
	}

	electedKicker->takeBall();
}

void Match::regenerateScoreText()
{
	char* ptr = scoreText;

	ptr = printInt(ptr, engine.teams[0].score);
	*ptr++ = '-';
	ptr = printInt(ptr, engine.teams[1].score);
}

char* Match::printInt(char* buffer, uint8_t number, bool leadingZeroes)
{
	if (number > 99)
	{
		number = 99;
	}
	if (number >= 10 || leadingZeroes)
	{
		int tens = number / 10;
		*buffer++ = '0' + tens;
		number -= tens * 10;
	}
	*buffer++ = '0' + number;

	return buffer;
}

char* Match::getMatchTimeString()
{
	int totalSeconds = matchTimer / TARGET_FRAMERATE;
	int minutes = totalSeconds / 60;
	int seconds = totalSeconds - minutes * 60;

	static char buffer[6];
	char* ptr = buffer;
	ptr = printInt(ptr, minutes);
	*ptr++ = ':';
	ptr = printInt(ptr, seconds, true);

	return buffer;
}
