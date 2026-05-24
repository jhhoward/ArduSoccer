#include "Engine.h"
#include "MathsFunctions.h"

Engine engine;

void Engine::init()
{
	gameState = GameState_Playing;

	personPlayer1 = 0;

	teams[WHITE_TEAM].init(people);
	teams[BLACK_TEAM].init(people + PLAYERS_PER_TEAM);

	match.reset();
}


void Engine::update()
{
	switch(gameState)
	{
	case GameState_Playing:
		{
			ball.update();

			for (uint8_t n = 0; n < NUM_PEOPLE; n++)
			{
				people[n].update();
			}

			if (changingPlayer1)
			{
				changingPlayer1 = false;

				// Find player closest to the ball that isn't the current one
				int closest = -1;
				int closestDistance = 0;

				for (int n = 0; n < NUM_PEOPLE; n++)
				{
					Person& person = people[n];
					if (person.team == 0 && n != personPlayer1 && !person.isGoalie())
					{
						int distance = estimateDistance(ball.x, ball.y, person.x, person.y);

						if (closest == -1 || distance < closestDistance)
						{
							closest = n;
							closestDistance = distance;
						}
					}
				}
				personPlayer1 = closest;
			}

			match.update();
			teams[WHITE_TEAM].update();
			teams[BLACK_TEAM].update();

			updateCamera();
		}
		break;
	case GameState_Menu:
		{
		}
		break;
	}

	frameCount++;
}

void Engine::updateCamera()
{
	int targetCameraX, targetCameraY;

	targetCameraX = ball.x;
	targetCameraY = ball.y;

	int targetCameraOffsetX = 0, targetCameraOffsetY = 0;

	if (!ball.owner)
	{
		targetCameraOffsetX = ball.velocityX >> 2;
		targetCameraOffsetY = ball.velocityY >> 2;

		if (targetCameraOffsetX == 0 && targetCameraOffsetY == 0 && 0)
		{
			// Try get the player controlled character in view instead
			Person& person = people[personPlayer1];
			targetCameraOffsetX = (person.x - ball.x) / 2;
			targetCameraOffsetY = (person.y - ball.y) / 2;

			int maxX = DISPLAYWIDTH / 3;
			int maxY = DISPLAYHEIGHT / 3;

			if (targetCameraOffsetX < -maxX)
			{
				targetCameraOffsetX = -maxX;
			}
			else if (targetCameraOffsetX > maxX)
			{
				targetCameraOffsetX = maxX;
			}
			if (targetCameraOffsetY < -maxY)
			{
				targetCameraOffsetY = -maxY;
			}
			else if (targetCameraOffsetY > maxY)
			{
				targetCameraOffsetY = maxY;
			}
		}
	}
	else
	{
		int8_t deltaX, deltaY;
		Person::getDirectionOffset(ball.owner->direction, deltaX, deltaY);
		targetCameraOffsetX = deltaX * 5;
		targetCameraOffsetY = deltaY * 12;
	}

	if (camera.offsetX < targetCameraOffsetX)
		camera.offsetX++;
	else if (camera.offsetX > targetCameraOffsetX)
		camera.offsetX--;
	if (camera.offsetY < targetCameraOffsetY)
		camera.offsetY++;
	else if (camera.offsetY > targetCameraOffsetY)
		camera.offsetY--;

	targetCameraX += camera.offsetX;
	targetCameraY += camera.offsetY;

	targetCameraX -= HALF_DISPLAYWIDTH;
	targetCameraY -= HALF_DISPLAYHEIGHT;

	//camera.x = people[personPlayer1].x - HALF_DISPLAYWIDTH;
	//camera.y = people[personPlayer1].y - HALF_DISPLAYHEIGHT;
	//camera.x = targetCameraX - HALF_DISPLAYWIDTH;
	//camera.y = targetCameraY - HALF_DISPLAYHEIGHT;

	int cameraDeltaX = (targetCameraX - camera.x) >> 1;
	int cameraDeltaY = (targetCameraY - camera.y) >> 1;

	if (cameraDeltaX < -MAX_CAMERA_DELTA)
		cameraDeltaX = -MAX_CAMERA_DELTA;
	if (cameraDeltaX > MAX_CAMERA_DELTA)
		cameraDeltaX = MAX_CAMERA_DELTA;
	if (cameraDeltaY < -MAX_CAMERA_DELTA)
		cameraDeltaY = -MAX_CAMERA_DELTA;
	if (cameraDeltaY > MAX_CAMERA_DELTA)
		cameraDeltaY = MAX_CAMERA_DELTA;

	camera.x += cameraDeltaX;
	camera.y += cameraDeltaY;

	if (camera.x < 0)
		camera.x = 0;
	if (camera.x > BACKGROUND_WIDTH - DISPLAYWIDTH)
		camera.x = BACKGROUND_WIDTH - DISPLAYWIDTH;
	if (camera.y < 0)
		camera.y = 0;
	if (camera.y > BACKGROUND_HEIGHT - DISPLAYHEIGHT)
		camera.y = BACKGROUND_HEIGHT - DISPLAYHEIGHT;

}

void Engine::draw()
{
	renderer.draw();
}

