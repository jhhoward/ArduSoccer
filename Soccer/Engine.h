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

class Engine
{
public:
	void init();
	void update();
	void draw();

	void cycleSelectedPerson() { changingPlayer1 = true; }
	
	int16_t frameCount;
	uint8_t gameState;

	Renderer renderer;
	Camera camera;
	Person people[NUM_PEOPLE];
	Ball ball;

	uint8_t personPlayer1;
	bool changingPlayer1;
};

extern Engine engine;

#endif
