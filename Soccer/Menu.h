#ifndef MENU_H_
#define MENU_H_

#include <stdlib.h>

typedef const void* MenuData;

class Menu
{
public:
	void init();
	void draw();
	void update();

	const MenuData* currentMenu;
	int8_t currentSelection;
	int8_t debounceInput;
	int8_t selectionDelta;

	int8_t numMenuItems();

	void switchMenu(const MenuData* newMenu);

	static void startSinglePlayer();
	static void startDemo();
	static void openMultiplayerMenu();
	static void hostMultiplayerGame();
	static void joinMultiplayerGame();
	static void connectSerial();
	static void connectLinkCable();
	static void toggleSound();
	static void toggleDifficulty();
};

#endif

