#include "Defines.h"
#include "Menu.h"
#include "Engine.h"
#include "Platform.h"

#define MENU_ENTRY_END 0
#define MENU_STR(x) (const void*)(x)
#define MENU_CALLBACK(x) (const void*)(x)

typedef void (*MenuFn)(void);

const char Str_SinglePlayer[] PROGMEM = "SINGLE PLAYER";
const char Str_Multiplayer[] PROGMEM = "MULTIPLAYER";
const char Str_Demo[] PROGMEM = "DEMO";
const char Str_SerialRelay[] PROGMEM = "SERIAL RELAY"; 
const char Str_LinkCable[] PROGMEM = "FX-C LINK CABLE";
const char Str_Host[] PROGMEM = "HOST GAME";
const char Str_Join[] PROGMEM = "JOIN GAME";


// Main menu
const void* const Menu_Main[] PROGMEM =
{
	Str_SinglePlayer,		MENU_CALLBACK(&Menu::startSinglePlayer),
	Str_Multiplayer,		MENU_CALLBACK(&Menu::openMultiplayerMenu),
	Str_Demo,				MENU_CALLBACK(&Menu::startDemo),
	MENU_ENTRY_END
};

const void* const Menu_Multiplayer[] PROGMEM =
{
	Str_SerialRelay,		MENU_CALLBACK(&Menu::connectSerial),
	Str_LinkCable,			MENU_CALLBACK(&Menu::connectLinkCable),
	MENU_ENTRY_END
};

const void* const Menu_LinkCableMultiplayer[] PROGMEM =
{
	Str_Host,				MENU_CALLBACK(&Menu::hostMultiplayerGame),
	Str_Join,				MENU_CALLBACK(&Menu::joinMultiplayerGame),
	MENU_ENTRY_END
};

void Menu::init()
{
	currentMenu = Menu_Main;
}

void Menu::draw()
{
	int index = 0;
	int x = 14;
	int startY = 8;
	int itemSpacing = 10;
	int y = startY;
	int item = 0;

	while (1)
	{
		if (pgm_read_ptr(&currentMenu[index]) == 0)
			break;

		const char* text = (const char*)pgm_read_ptr(&currentMenu[index]);

		if (item == currentSelection)
		{
			engine.renderer.drawText(smallFont, PSTR(">"), 1, y, 1);
		}

		engine.renderer.drawText(smallFont, text, x, y, 1);
		index += 2;
		y += itemSpacing;
		item++;
	}
}

void Menu::update()
{
	if (!debounceInput)
	{
		if (numMenuItems())
		{
			if (Platform.readInput() & Input_Dpad_Up)
			{
				currentSelection--;
				if (currentSelection == -1)
				{
					currentSelection = numMenuItems() - 1;
				}
			}
			if (Platform.readInput() & Input_Dpad_Down)
			{
				currentSelection++;
				if (currentSelection == numMenuItems())
				{
					currentSelection = 0;
				}
			}
			if (Platform.readInput() & Input_Btn_B)
			{
				MenuFn fn = (MenuFn)pgm_read_ptr(&currentMenu[currentSelection * 2 + 1]);
				fn();
			}

			if (Platform.readInput() & Input_Btn_A)
			{
				if (currentMenu != Menu_Main)
				{
					switchMenu(Menu_Main);
				}
			}
		}
	}
	debounceInput = Platform.readInput() != 0;

}

int8_t Menu::numMenuItems()
{
	int8_t index = 0;
	int8_t count = 0;

	while (1)
	{
		if (pgm_read_ptr(&currentMenu[index]) == 0)
			break;
		index += 2;
		count++;
	}
	return count;
}

void Menu::switchMenu(const MenuData* newMenu)
{
	currentMenu = newMenu;
	currentSelection = 0;
	debounceInput = true;
}

void Menu::startSinglePlayer()
{
	engine.startSinglePlayer();
}

void Menu::startDemo()
{
	engine.startDemo();
}

void Menu::openMultiplayerMenu()
{
	engine.menu.switchMenu(Menu_Multiplayer);
}

void Menu::hostMultiplayerGame()
{
//	engine.startMultiplayer(true);
}

void Menu::joinMultiplayerGame()
{
//	engine.startMultiplayer(false);
}

void Menu::connectSerial()
{
	bool isHost = Platform.connectMultiplayer();
	engine.startMultiplayer(isHost);
}

void Menu::connectLinkCable()
{
	engine.menu.switchMenu(Menu_LinkCableMultiplayer);
}
