#include "Engine.h"
#include "Match.h"

void Match::reset()
{
	for (uint8_t n = 0; n < NUM_PEOPLE; n++)
	{
		engine.people[n].init(n);
	}
	engine.ball.setPosition(CENTER_MARK_X, CENTER_MARK_Y);

	state = Match::KickOff;
}

void Match::update()
{

}
