#include "Engine.h"

Engine engine;

void Engine::init()
{
	gameState = GameState_Playing;

	for (uint8_t n = 0; n < NUM_PEOPLE; n++)
	{
		people[n].init(n);
	}

	personPlayer1 = 0;

	ball.setPosition(128, 50, 30);
}


void Engine::update()
{
	//uint8_t input = Platform.readInput();
	//
	//if (input & Input_Dpad_Up)
	//{
	//	camera.y--;
	//}
	//if (input & Input_Dpad_Down)
	//{
	//	camera.y++;
	//}
	//if (input & Input_Dpad_Left)
	//{
	//	camera.x--;
	//}
	//if (input & Input_Dpad_Right)
	//{
	//	camera.x++;
	//}

	switch(gameState)
	{
	case GameState_Playing:
		{
			ball.update();

			for (uint8_t n = 0; n < NUM_PEOPLE; n++)
			{
				people[n].update();
			}
		}
		break;
	case GameState_Menu:
		{
		}
		break;
	}

	camera.x = people[personPlayer1].x - HALF_DISPLAYWIDTH;
	camera.y = people[personPlayer1].y - HALF_DISPLAYHEIGHT;

	int soccerPitchWidth = 256;
	int soccerPitchHeight = 320;

	if (camera.x < 0)
		camera.x = 0;
	if (camera.x > soccerPitchWidth - DISPLAYWIDTH)
		camera.x = soccerPitchWidth - DISPLAYWIDTH;
	if (camera.y < 0)
		camera.y = 0;
	if (camera.y > soccerPitchHeight - DISPLAYHEIGHT)
		camera.y = soccerPitchHeight - DISPLAYHEIGHT;

	frameCount++;
}

void Engine::draw()
{
	renderer.draw();
}

