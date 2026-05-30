#ifndef ENGINE_H_
#define ENGINE_H_

#ifdef _WIN32
#include "../Windows/SDLPlatform.h"
#else
#include "ArduboyPlatform.h"
#endif

#include "Platform.h"
#include "Renderer.h"
#include "Person.h"
#include "Ball.h"
#include "Match.h"
#include "Team.h"

enum
{
	GameState_Menu,
	GameState_Playing,
};

struct Camera
{
	int x, y;
	int offsetX, offsetY;
};

struct GameSettings
{
	uint8_t matchHalfLength;
};

class Engine
{
public:
	void init();
	void update();
	void draw();

	void setCameraFocus(int focusX, int focusY);

	int16_t frameCount;
	uint8_t gameState;

	Renderer renderer;
	Camera camera;
	Person people[NUM_PEOPLE];
	Ball ball;
	Match match;
	Team teams[2];
	GameSettings settings;

private:
	void updateCamera();
};

extern Engine engine;

#endif
