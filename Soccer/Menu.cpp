#include "Defines.h"
#include "Menu.h"
#include "Engine.h"
#include "Platform.h"
#include "Generated/MenuAssets.inc.h"

#define WITH_LINK_CABLE 0

#define MENU_ENTRY_END 0
#define MENU_STR(x) (const void*)(x)
#define MENU_CALLBACK(x) (const void*)(x)

typedef void (*MenuFn)(void);

const char Str_SinglePlayer[] PROGMEM = "PLAY MATCH";
const char Str_Multiplayer[] PROGMEM = "MULTIPLAYER";
//const char Str_Demo[] PROGMEM = "DEMO";
const char Str_SerialRelay[] PROGMEM = "SERIAL RELAY"; 
const char Str_SoundOn[] PROGMEM = "SOUND FX:ON";
const char Str_SoundOff[] PROGMEM = "SOUND FX:OFF";
const char Str_Easy[] PROGMEM = "DIFFICULTY:NORMAL";
const char Str_Hard[] PROGMEM = "DIFFICULTY:HARD";

#if WITH_LINK_CABLE
const char Str_LinkCable[] PROGMEM = "FX-C LINK CABLE";
const char Str_Host[] PROGMEM = "HOST GAME";
const char Str_Join[] PROGMEM = "JOIN GAME";
#endif


// Main menu
const void* const Menu_Main[] PROGMEM =
{
	Str_SinglePlayer,		MENU_CALLBACK(&Menu::startSinglePlayer),
	Str_Easy,				MENU_CALLBACK(&Menu::toggleDifficulty),
	Str_SoundOn,			MENU_CALLBACK(&Menu::toggleSound),
	Str_Multiplayer,		MENU_CALLBACK(&Menu::openMultiplayerMenu),
	//Str_Demo,				MENU_CALLBACK(&Menu::startDemo),
	MENU_ENTRY_END
};

#if WITH_LINK_CABLE
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
#endif

void Menu::init()
{
	currentMenu = Menu_Main;
	currentSelection = 0;
}

void Menu::draw()
{
	int index = 0;
	int x = 14;
	int startY = 25;
	int itemSpacing = 10;
	int y = startY;
	int item = 0;

	engine.renderer.drawText(smallFont, PSTR("ARDU"), DISPLAYWIDTH / 2 - 3 * 4, 0, 1);
	engine.renderer.drawText(largeFont, PSTR("SOCCER"), DISPLAYWIDTH / 2 - 15 * 3, 8, 1);

	while (1)
	{
		if (pgm_read_ptr(&currentMenu[index]) == 0)
			break;

		const char* text = (const char*)pgm_read_ptr(&currentMenu[index]);

		if (text == Str_SoundOn && Platform.isMuted())
		{
			text = Str_SoundOff;
		}
		if (text == Str_Easy && engine.settings.difficulty)
		{
			text = Str_Hard;
		}

		if (item == currentSelection)
		{
			drawBitmap(3, y, menuBallSprite, 8, 8, 1);
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
#if WITH_LINK_CABLE
	engine.menu.switchMenu(Menu_Multiplayer);
#else
	connectSerial();
#endif
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
	drawBitmap(DISPLAYWIDTH / 2 - 22, 0, relaySprite, 45, 24, 1);
	engine.renderer.drawText(smallFont, PSTR("CONNECT 2 ARDUBOYS"), 10, 30, 1);
	engine.renderer.drawText(smallFont, PSTR("TO SERIAL RELAY"), 19, 38, 1);

	drawBitmap(DISPLAYWIDTH / 2 - 111 / 2, DISPLAYHEIGHT - 8, urlSprite, 111, 8, 1);

	bool isHost = Platform.connectMultiplayer();
	engine.startMultiplayer(isHost);
}

void Menu::connectLinkCable()
{
#if WITH_LINK_CABLE
	engine.menu.switchMenu(Menu_LinkCableMultiplayer);
#endif
}

void Menu::toggleSound()
{
	Platform.setMuted(!Platform.isMuted());
}

void Menu::toggleDifficulty()
{
	engine.settings.difficulty = !engine.settings.difficulty;
}
