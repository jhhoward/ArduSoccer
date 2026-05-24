#include "Defines.h"
#include "Team.h"
#include "Person.h"
#include "MathsFunctions.h"
#include "Engine.h"

static const int16_t formationPositions[] PROGMEM =
{
	0, -64,
	64, -32,
	-64, -32,
	0, 32
};

void Team::init(Person* inPlayers)
{
	players = inPlayers;
}

Person* Team::getClosestPlayer(int16_t x, int16_t y)
{
	Person* closest = nullptr;
	int closestDistance = -1;

	for (int n = 0; n < PLAYERS_PER_TEAM; n++)
	{
		Person& player = players[n];
		int distance = estimateDistance(player.x, player.y, x, y);

		if (closest == nullptr || distance < closestDistance)
		{
			closest = &player;
			closestDistance = distance;
		}
	}

	return closest;
}

#define FORMATION_EDGE_SPACING 32
#define MAX_FORMATION_OFFSET_X 32
#define MAX_FORMATION_OFFSET_Y 64
#define ATTACK_Y_OFFSET 20
#define DEFEND_Y_OFFSET -64

void Team::calculateFormationPosition(uint8_t index, int16_t& outX, int16_t& outY)
{
	int multiplier = isTopHalf() ? 1 : -1;

	if (index >= PLAYERS_PER_TEAM)
		index -= PLAYERS_PER_TEAM;

	if (index == 0)
	{
		// Goalie
		outX = BACKGROUND_WIDTH / 2;
		outY = CENTER_MARK_Y - 125 * multiplier;
		return;
	}

	index--;

	outX = (int16_t)pgm_read_word(&formationPositions[index * 2]);
	outY = (int16_t)pgm_read_word(&formationPositions[index * 2] + 1);
	outY *= multiplier;

	outX += CENTER_MARK_X + formationOffsetX;
	outY += CENTER_MARK_Y + formationOffsetY;

	if (outX < PITCH_LEFT + FORMATION_EDGE_SPACING)
	{
		outX = PITCH_LEFT + FORMATION_EDGE_SPACING;
	}
	if (outY < PITCH_TOP + FORMATION_EDGE_SPACING)
	{
		outY = PITCH_TOP + FORMATION_EDGE_SPACING;
	}
	if (outX > PITCH_RIGHT - FORMATION_EDGE_SPACING)
	{
		outX = PITCH_RIGHT - FORMATION_EDGE_SPACING;
	}
	if (outY > PITCH_BOTTOM - FORMATION_EDGE_SPACING)
	{
		outY = PITCH_BOTTOM - FORMATION_EDGE_SPACING;
	}
}

bool Team::isTopHalf()
{
	return this == &engine.teams[0];
}

void Team::update()
{
	int multiplier = isTopHalf() ? 1 : -1;

	formationOffsetX = engine.ball.x - CENTER_MARK_X;
	formationOffsetY = engine.ball.y - CENTER_MARK_Y;

	if (engine.ball.owner)
	{
		if (&engine.teams[engine.ball.owner->team] == this)
		{
			formationOffsetY += ATTACK_Y_OFFSET * multiplier;
		}
		else
		{
			formationOffsetY += DEFEND_Y_OFFSET * multiplier;
		}
	}

	if (formationOffsetX < -MAX_FORMATION_OFFSET_X)
		formationOffsetX = -MAX_FORMATION_OFFSET_X;
	if (formationOffsetX > MAX_FORMATION_OFFSET_X)
		formationOffsetX = MAX_FORMATION_OFFSET_X;
	if (formationOffsetY < -MAX_FORMATION_OFFSET_Y)
		formationOffsetY = -MAX_FORMATION_OFFSET_Y;
	if (formationOffsetY > MAX_FORMATION_OFFSET_Y)
		formationOffsetY = MAX_FORMATION_OFFSET_Y;

}
