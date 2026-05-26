#include "Engine.h"
#include "Match.h"

void Match::reset()
{
	for (uint8_t n = 0; n < NUM_PEOPLE; n++)
	{
		engine.people[n].init(n);
	}

	setupKickOff(&engine.teams[WHITE_TEAM]);
	engine.renderer.showLargeMessage(PSTR("KICK OFF!"));
	engine.setCameraFocus(CENTER_MARK_X, CENTER_MARK_Y);
}

void Match::update()
{
	switch(state)
	{
	case Match::Playing:
		if (engine.ball.isInsideTopNet())
		{
			if (engine.teams[WHITE_TEAM].isTopHalf())
			{
				engine.teams[BLACK_TEAM].score++;
			}
			else
			{
				engine.teams[WHITE_TEAM].score++;
			}
			setState(Match::Scored);
			engine.renderer.showLargeMessage(PSTR("GOAL!"));
		}
		if (engine.ball.isInsideBottomNet())
		{
			if (engine.teams[WHITE_TEAM].isTopHalf())
			{
				engine.teams[WHITE_TEAM].score++;
			}
			else
			{
				engine.teams[BLACK_TEAM].score++;
			}
			setState(Match::Scored);
			engine.renderer.showLargeMessage(PSTR("GOAL!"));
		}
		break;

	case Match::Scored:
		if (timeInState > 60 * 3)
		{
			setupKickOff(&engine.teams[WHITE_TEAM]);
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
