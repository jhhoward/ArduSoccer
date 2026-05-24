#include <stdint.h>
#include "Engine.h"
#include "Renderer.h"
#include "Generated/Pitch.inc.h"


Renderer::Renderer() 
{
	for (int n = 0; n < NUM_DRAWABLES; n++)
	{
		drawOrder[n] = n;
	}
}

void Renderer::draw()
{
	int tileX = engine.camera.x / TILE_SIZE;
	int tileY = engine.camera.y / TILE_SIZE;
	int offsetX = engine.camera.x & (TILE_SIZE - 1);
	int offsetY = engine.camera.y & (TILE_SIZE - 1);
	int outY = -offsetY;

	for (int y = 0; y < (DISPLAYHEIGHT / TILE_SIZE) + 1; y++)
	{
		int outX = -offsetX;
		for (int x = 0; x < (DISPLAYWIDTH / TILE_SIZE) + 1; x++)
		{
			int tile = pgm_read_byte(&bgTileMap[(tileY + y) * (256 / 8) + tileX + x]);

			drawBitmap(outX, outY, bgTiles + (TILE_SIZE_BYTES * tile), TILE_SIZE, TILE_SIZE, 1);
			outX += TILE_SIZE;
		}
		outY += TILE_SIZE;
	}

	for (int n = 0; n < NUM_DRAWABLES; n++)
	{
		uint8_t index = drawOrder[n];

		// Sort
		if (n < NUM_DRAWABLES - 1)
		{
			uint8_t next = drawOrder[n + 1];

			if (getSortPosition(index) > getSortPosition(next))
			{
				drawOrder[n] = next;
				drawOrder[n + 1] = index;
				index = next;
			}
		}

		switch (index)
		{
		default:
			drawPerson(index);
			break;
		case BALL_DRAWABLE:
			drawBall();
			break;
		case BALL_SHADOW_DRAWABLE:
			drawBallShadow();
			break;
		case LOWER_GOAL_DRAWABLE:
			drawLowerGoal();
			break;
		case UPPER_GOAL_DRAWABLE:
			drawUpperGoal();
			break;
		}
	}

	if (Platform.readInputDown() & Input_Btn_A)
	{
		//showLargeMessage(PSTR("GOAL!"));
	}

	if (largeMessageCounter > 0)
	{
		int largeMessageHeight = 18;
		int transitionSize = largeMessageHeight / 2;
		bool showText = false;

		if (largeMessageCounter < transitionSize)
		{
			fillRect(0, DISPLAYHEIGHT / 2 - largeMessageCounter, DISPLAYWIDTH, largeMessageCounter * 2, 0);
			showText = largeMessageCounter >= transitionSize - 2;
		}
		else if (largeMessageCounter > 60 - transitionSize)
		{
			int y = (60 - largeMessageCounter);
			fillRect(0, DISPLAYHEIGHT / 2 - y, DISPLAYWIDTH, y * 2, 0);
			showText = y >= transitionSize - 2;
		}
		else
		{
			fillRect(0, DISPLAYHEIGHT / 2 - largeMessageHeight / 2, DISPLAYWIDTH, largeMessageHeight, 0);
			showText = true;
		}

		if (showText)
		{
			drawText(largeMessage, largeMessageX, DISPLAYHEIGHT / 2 - 7, 1);
		}

		largeMessageCounter--;
	}
}

void Renderer::showLargeMessage(const char* message)
{
	int messageLength = 0;

	const char* ptr = message;
	while (1)
	{
		char c = pgm_read_byte(ptr);
		ptr++;
		if (!c)
			break;
		if (c == ' ')
			messageLength += 6;
		else
			messageLength += 15;
	}

	largeMessageX = HALF_DISPLAYWIDTH - messageLength / 2;
	largeMessage = message;
	largeMessageCounter = 60;
}

void Renderer::drawLowerGoal()
{
	int goalWidth = 52;
	int goalHeight = 32;
	int outX = 102 - engine.camera.x;
	int outY = 263 - engine.camera.y;

	if (outX >= DISPLAYWIDTH)
		return;
	if (outY >= DISPLAYHEIGHT)
		return;
	if (outX <= -goalWidth)
		return;
	if (outY <= -goalHeight)
		return;

	drawBitmap(outX, outY, lowerGoalSpriteMask, goalWidth, goalHeight, 0);
	drawBitmap(outX, outY, lowerGoalSprite, goalWidth, goalHeight, 1);
}

void Renderer::drawUpperGoal()
{
	int goalWidth = 52;
	int goalHeight = 32;
	int outX = 102 - engine.camera.x;
	int outY = 2 - engine.camera.y;

	if (outX >= DISPLAYWIDTH)
		return;
	if (outY >= DISPLAYHEIGHT)
		return;
	if (outX <= -goalWidth)
		return;
	if (outY <= -goalHeight)
		return;

	drawBitmap(outX, outY, upperGoalSpriteMask, goalWidth, goalHeight, 0);
	drawBitmap(outX, outY, upperGoalSprite, goalWidth, goalHeight, 1);
}


int Renderer::getSortPosition(uint8_t drawable)
{
	switch (drawable)
	{
	default:
		return engine.people[drawable].y;
	case BALL_DRAWABLE:
		return engine.ball.y;
	case BALL_SHADOW_DRAWABLE:
		return engine.ball.y - 1;
	case LOWER_GOAL_DRAWABLE:
		return 295;
	case UPPER_GOAL_DRAWABLE:
		return 27;
	}
}

void Renderer::drawBall()
{
	int outX = engine.ball.x - engine.camera.x - 4;
	int outY = engine.ball.y - engine.ball.z - engine.camera.y - 5;

	if (outX >= DISPLAYWIDTH)
		return;
	if (outY >= DISPLAYHEIGHT)
		return;
	if (outX <= -8)
		return;
	if (outY <= -8)
		return;

	drawBitmap(outX, outY, ballSpriteMask, 8, 8, 0);
	drawBitmap(outX, outY, ballSprite, 8, 8, 1);
}

void Renderer::drawBallShadow()
{
	int outX = engine.ball.x - engine.camera.x - 4;
	int outY = engine.ball.y - engine.camera.y - 5;

	if (outX >= DISPLAYWIDTH)
		return;
	if (outY >= DISPLAYHEIGHT)
		return;
	if (outX <= -8)
		return;
	if (outY <= -8)
		return;

	drawBitmap(outX, outY, ballSpriteMask + 8, 8, 8, 0);
}

void Renderer::drawPerson(int index)
{
	Person& person = engine.people[index];
	int outX = person.x - engine.camera.x - 8;
	int outY = person.y - engine.camera.y - 14 - person.z;

	if (outX >= DISPLAYWIDTH)
		return;
	if (outY >= DISPLAYHEIGHT)
		return;
	if (outX <= -16)
		return;
	if (outY <= -16)
		return;

	const uint8_t* sprite;
	const uint8_t* mask;

	switch (person.team)
	{
	case 0:
		sprite = whitePlayerSprite;
		mask = whitePlayerSpriteMask;
		break;
	case 1:
		sprite = blackPlayerSprite;
		mask = blackPlayerSpriteMask;
		break;
	default:
		sprite = refereeSprite;
		mask = refereeSpriteMask;
		break;
	}

	sprite += person.displayFrame * 32;
	mask += person.displayFrame * 32;

	drawBitmap(outX, outY, mask, 16, 16, 0);
	drawBitmap(outX, outY, sprite, 16, 16, 1);

	if (index == engine.personPlayer1)
	{
		drawBitmap(outX + 5, outY - 8, selectionArrowSpriteMask, 8, 8, 0);
		drawBitmap(outX + 5, outY - 8, selectionArrowSprite, 8, 8, 1);
	}
}

void Renderer::drawText(const char* text, int16_t x, int16_t y, uint8_t colour)
{
	while (1)
	{
		char c = pgm_read_byte(text);
		if (!c)
			break;
		text++;

		int index = c - ' ';

		if (index == 0)
		{
			x += 6;
		}
		else
		{
			const uint8_t* data = largeFontData + (32 * index);
			drawBitmap(x, y, data, 16, 16, colour);
			x += 15;
		}
	}
}

