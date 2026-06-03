// SDL platform
#include <stdio.h>
#include "SDLPlatform.h"
#include "Engine.h"
//#include "Generated/Data_Audio.h"
#include "lodepng.h"

#define TONES_END 0x8000
#define TONES_REPEAT 0x8001

SDLPlatform Platform;

constexpr int audioSampleRate = 48000;

const uint16_t* currentAudioPattern = nullptr;
int currentPatternBufferPos = 0;
bool isAudioEnabled = true;

bool isRecording = false;
int currentRecordingFrame = 0;


// Sound generation testing
const uint8_t soundDither[] =
{
	0, 128, 191, 64
};

const uint16_t soundTest[] =
{
	440, 512,
	100, 512,
	440, 512,
	TONES_END
};

#define PROCEDURAL_BUFFER_SIZE 20480
uint16_t proceduralSoundBuffer[PROCEDURAL_BUFFER_SIZE];

int generateProceduralSound(
	uint16_t* buffer, 
	size_t bufferLength,
	uint16_t totalDuration,
	uint16_t startFreq, uint16_t endFreq,
	uint16_t noisiness, uint8_t startVol, uint8_t endVol) 
{
	int frameSize = 4;
	int totalFrames = totalDuration / frameSize;

	if (totalFrames * 2 >= bufferLength)
	{
		totalFrames = bufferLength / 2 - 1;
	}

	int index = 0;

	for (uint16_t i = 0; i < totalFrames; i++)
	{
		float t = (float)i / (float)totalFrames;
		int32_t currentFreq = startFreq + (int32_t)((endFreq - startFreq) * t);

		if (noisiness > 0) 
		{
			currentFreq += (rand() % noisiness) - (noisiness / 2);
			//currentFreq += random(-noisiness, noisiness + 1);
		}

		// Clamp frequency to Arduboy limits
		if (currentFreq < 16) currentFreq = 16;
		if (currentFreq > 32767) currentFreq = 32767;

		uint8_t currentVol = startVol + (uint8_t)((endVol - startVol) * t);
		uint16_t finalFreq = currentFreq;

		if (currentVol < soundDither[i & 3])
		{
			finalFreq = 0;
		}

		buffer[index++] = finalFreq;
		buffer[index++] = frameSize;
	}

	// Mandatory array terminator for ArduboyTones
	buffer[index++] = TONES_END;

	return index;
}

///

void Play(const uint16_t* pattern)
{
	currentPatternBufferPos = 0;
	currentAudioPattern = pattern;
	currentPatternBufferPos = 0;
}

/*
void FillAudioBufferOld(void* udata, uint8_t* stream, int len)
{
	int feedPos = 0;

	static int waveSamplesLeft = 0;
	static int noteSamplesLeft = 0;
	static int frequency = 0;
	static bool high = false;
	
	if (!isAudioEnabled)
	{
		while (feedPos < len)
		{
			stream[feedPos++] = 0;
		}
		return;
	}

	while (feedPos < len)
	{
		if (currentAudioPattern != nullptr)
		{
			if (noteSamplesLeft == 0)
			{
				frequency = currentAudioPattern[currentPatternBufferPos];
				uint16_t duration = currentAudioPattern[currentPatternBufferPos + 1];

				noteSamplesLeft = (audioSampleRate * duration) / 1024;

				waveSamplesLeft = frequency > 0 ? (audioSampleRate / frequency) / 2 : noteSamplesLeft;

				currentPatternBufferPos += 2;
				if (currentAudioPattern[currentPatternBufferPos] == TONES_END)
				{
					currentAudioPattern = nullptr;
				}
			}
		}

		if (frequency == 0)
		{
			while (feedPos < len && (!currentAudioPattern || noteSamplesLeft > 0))
			{
				stream[feedPos++] = 0;

				if (noteSamplesLeft > 0)
					noteSamplesLeft--;
			}
		}
		else
		{
			while (feedPos < len && waveSamplesLeft > 0 && noteSamplesLeft > 0)
			{
				int volume = 32;
				//stream[feedPos++] = high ? 128 + volume : 128 - volume;
				stream[feedPos++] = high ? volume : 0;
				waveSamplesLeft--;
				noteSamplesLeft--;
			}

			if (waveSamplesLeft == 0)
			{
				high = !high;
				waveSamplesLeft = (audioSampleRate / frequency) / 2;
			}
		}

	}
}
*/

void FillAudioBuffer(void* udata, uint8_t* stream, int len)
{
	int feedPos = 0;

	static int noteSamplesLeft = 0;
	static int frequency = 0;
	static double time = 0;

	if (!isAudioEnabled)
	{
		while (feedPos < len)
		{
			stream[feedPos++] = 0;
		}
		return;
	}

	while (feedPos < len)
	{
		if (noteSamplesLeft == 0)
		{
			if (currentAudioPattern != nullptr)
			{
				frequency = currentAudioPattern[currentPatternBufferPos];
				uint16_t duration = currentAudioPattern[currentPatternBufferPos + 1];

				noteSamplesLeft = (audioSampleRate * duration) / 1024;
				//time = 0;

				currentPatternBufferPos += 2;
				if (currentAudioPattern[currentPatternBufferPos] == TONES_END)
				{
					currentAudioPattern = nullptr;
				}
				else if (currentAudioPattern[currentPatternBufferPos] == TONES_REPEAT)
				{
					currentPatternBufferPos = 0;
				}
			}
			else
			{
				while (feedPos < len)
				{
					stream[feedPos++] = 0;
				}
			}
		}
		else
		{
			if (frequency == 0)
			{
				while (feedPos < len && noteSamplesLeft > 0)
				{
					stream[feedPos++] = 0;
					noteSamplesLeft--;
				}
			}
			else
			{
				while (feedPos < len && noteSamplesLeft > 0)
				{
					time += 1.0 / audioSampleRate;
					bool high = sin(2 * 3.1415 * time * frequency) > 0;
					int volume = 32;
					stream[feedPos++] = high ? volume : 0;
					//stream[feedPos++] = (uint8_t)(32 + 32 * sin(2 * 3.1415 * time * frequency));

					noteSamplesLeft--;
				}
			}
		}
	}
}

void SDLPlatform::init()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer( DISPLAYWIDTH * ZOOM_SCALE, DISPLAYHEIGHT * ZOOM_SCALE, SDL_WINDOW_RESIZABLE, &m_appWindow, &m_appRenderer );
	SDL_RenderSetLogicalSize(m_appRenderer, DISPLAYWIDTH, DISPLAYHEIGHT);
	
	m_screenSurface = SDL_CreateRGBSurface(0, DISPLAYWIDTH, DISPLAYHEIGHT, 32, 
											0x000000ff,
											0x0000ff00, 
											0x00ff0000, 
											0xff000000
											);
	m_screenTexture = SDL_CreateTexture(m_appRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_screenSurface->w, m_screenSurface->h);

	SDL_AudioSpec wanted;
	wanted.freq = audioSampleRate;
	wanted.format = AUDIO_U8;
	wanted.channels = 1;
	wanted.samples = 4096;
	wanted.callback = FillAudioBuffer;

	if (SDL_OpenAudio(&wanted, NULL) < 0) {
		printf("Error: %s\n", SDL_GetError());
	}
	SDL_PauseAudio(0);

	m_isRunning = true;
}

typedef struct
{
	SDL_Scancode key;
	uint8_t mask;
} KeyMap;

#define NUM_KEY_MAPPINGS sizeof(KeyMappings) / sizeof(KeyMap)

KeyMap KeyMappings[] =
{
	{ SDL_SCANCODE_LEFT, Input_Dpad_Left },
	{ SDL_SCANCODE_RIGHT, Input_Dpad_Right },
	{ SDL_SCANCODE_UP, Input_Dpad_Up },
	{ SDL_SCANCODE_DOWN, Input_Dpad_Down },
	{ SDL_SCANCODE_X, Input_Btn_B },
	{ SDL_SCANCODE_Z, Input_Btn_A },

	{ SDL_SCANCODE_LSHIFT, Input_Btn_A },
	{ SDL_SCANCODE_LCTRL, Input_Btn_B },
};

void SDLPlatform::updateInputState()
{
	lastInputState[REMOTE_PLAYER] = 0;
	inputState[REMOTE_PLAYER] = 0;

	lastInputState[LOCAL_PLAYER] = inputState[LOCAL_PLAYER];
	inputState[LOCAL_PLAYER] = 0;

	const uint8_t* keyStates = SDL_GetKeyboardState(NULL);

	for (unsigned int n = 0; n < NUM_KEY_MAPPINGS; n++)
	{
		if (keyStates[KeyMappings[n].key])
		{
			inputState[LOCAL_PLAYER] |= KeyMappings[n].mask;
		}
	}
}

void SDLPlatform::run()
{
	m_isRunning = true;
	
	engine.init();
	
	while(m_isRunning)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type) 
			{
				case SDL_QUIT:
				m_isRunning = false;
				break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_F5:
						{
							lodepng_encode32_file("screenshot.png", (unsigned char*)(m_screenSurface->pixels), m_screenSurface->w, m_screenSurface->h);
							break;
						}
						case SDLK_F11:
							isRecording = !isRecording;
							break;

						case SDLK_ESCAPE:
							m_isRunning = false;
							break;

						case SDLK_0:		// Kick
							currentAudioPattern = nullptr;
							generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE, 
								160, 180, 30, 0, 255, 128);
							//generateProceduralSound(proceduralSoundBuffer, 20480, 1024, 440, 100, 0, 255, 128);
							Play(proceduralSoundBuffer);
							break;

						case SDLK_9:	 // Crowd
							currentAudioPattern = nullptr;
							generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE,
								600, 450, 200, 30, 255, 0);
							//generateProceduralSound(proceduralSoundBuffer, 2048, 1024, 600, 500, 250, 255, 0);
							Play(proceduralSoundBuffer);
							break;

						case SDLK_8:		// Whistle
							currentAudioPattern = nullptr;
							generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE,
													400, 2500, 2550, 150, 255, 255);
							//generateProceduralSound(proceduralSoundBuffer, 2048, 1024, 600, 500, 250, 255, 0);
							Play(proceduralSoundBuffer);
							break;

						case SDLK_7:		// Crowd
							currentAudioPattern = nullptr;
							{
								int size = generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE,
									500, 200, 450, 180, 0, 255) - 1;
								size += generateProceduralSound(proceduralSoundBuffer + size, PROCEDURAL_BUFFER_SIZE,
									2000, 450, 450, 180, 255, 255) - 1;
								size += generateProceduralSound(proceduralSoundBuffer + size, PROCEDURAL_BUFFER_SIZE,
									2000, 450, 450, 180, 255, 0);
							}
							Play(proceduralSoundBuffer);
							break;

						case SDLK_6:		// Ball land
							currentAudioPattern = nullptr;
							generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE,
								40, 180, 30, 0, 255, 128);
							//generateProceduralSound(proceduralSoundBuffer, 20480, 1024, 440, 100, 0, 255, 128);
							Play(proceduralSoundBuffer);
							break;

						case SDLK_5:		// Hit post
							currentAudioPattern = nullptr;
							generateProceduralSound(proceduralSoundBuffer, PROCEDURAL_BUFFER_SIZE,
								150, 300, 300, 10, 255, 128);
							//generateProceduralSound(proceduralSoundBuffer, 20480, 1024, 440, 100, 0, 255, 128);
							Play(proceduralSoundBuffer);
							break;

						case SDLK_1:
							break;
					}
				break;
			}

		}

		updateInputState();
		isAudioEnabled = !m_isMuted;

		SDL_SetRenderDrawColor ( m_appRenderer, 206, 221, 231, 255 );
		SDL_RenderClear ( m_appRenderer );

		clearDisplay(0);
		engine.update();
		engine.draw();

		for (int n = 0; n < 5; n++)
		{
			int16_t formationX, formationY;

			engine.teams[0].calculateFormationPosition(n, formationX, formationY);
			drawPixel(formationX, formationY, 3);
			drawPixel(formationX + 1, formationY, 3);
			drawPixel(formationX + 1, formationY + 1, 3);
			drawPixel(formationX, formationY + 1, 3);

			engine.teams[1].calculateFormationPosition(n, formationX, formationY);
			drawPixel(formationX, formationY, 2);
			drawPixel(formationX + 1, formationY, 2);
			drawPixel(formationX + 1, formationY + 1, 2);
			drawPixel(formationX, formationY + 1, 2);
		}

		SDL_UpdateTexture(m_screenTexture, NULL, m_screenSurface->pixels, m_screenSurface->pitch);
		SDL_RenderCopy(m_appRenderer, m_screenTexture, NULL, NULL);
		SDL_RenderPresent(m_appRenderer);

		if (isRecording)
		{
			char filename[50];
			snprintf(filename, 50, "capture/frame%03d.png", currentRecordingFrame);
			lodepng_encode32_file(filename, (unsigned char*)(m_screenSurface->pixels), m_screenSurface->w, m_screenSurface->h);
			currentRecordingFrame++;
		}

		SDL_Delay(1000 / TARGET_FRAMERATE);
	}

	SDL_Quit();
}

int main(int, char**)
{
	Platform.init();

	Platform.run();

	return 0;
}

void SDLPlatform::drawPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    *(Uint32 *)p = pixel;
}

uint8_t paletteColours[] =
{
#if defined(EMULATE_GAMEBUINO)
	0, 0, 0,
	206, 221, 231,
#elif defined(EMULATE_ARDUBOY)
	0, 0, 0,
	255, 255, 255,
	255, 0, 0,
	0, 255, 0
#elif 1
	0, 0, 0,
	170, 170, 170,
	0, 170, 170,
	170, 0, 0
#else
	0, 0, 0,
	255, 255, 255,
	109, 109, 85, //145, 145, 170,
	182, 182, 170, //182, 182, 170
#endif
};

void SDLPlatform::drawPixel(int x, int y, uint8_t colour)
{
	if (x < 0 || y < 0 || x >= DISPLAYWIDTH || y >= DISPLAYHEIGHT)
	{
		return;
	}

	Uint32 col = SDL_MapRGBA(m_screenSurface->format, paletteColours[colour * 3], paletteColours[colour * 3 + 1], paletteColours[colour * 3 + 2], 255);
	drawPixel(m_screenSurface, x, y, col);

	/*if(!colour)
	{
		Uint32 black = SDL_MapRGBA(m_screenSurface->format, 0, 0, 0, 255);
		drawPixel(m_screenSurface, x, y, black);
	}
	else
	{
		Uint32 white = SDL_MapRGBA(m_screenSurface->format, 206, 221, 231, 255);
		drawPixel(m_screenSurface, x, y, white);
	}*/
}

void SDLPlatform::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, uint8_t color)
{
	const uint8_t* ptr = bitmap;

	for (int j = 0; j < h; j += 8)
	{
		for (int i = 0; i < w; i++)
		{
			uint8_t values = *ptr++;
			for (int n = 0; n < 8; n++)
			{
				if (values & (1 << n))
				{
					drawPixel(x + i, y + j + n, color);
				}
			}
		}
	}
}

void SDLPlatform::fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t colour)
{
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			Platform.drawPixel(x + i, y + j, colour);
		}
	}
}

void clearDisplay(uint8_t colour)
{
	for(int y = 0; y < DISPLAYHEIGHT; y++)
	{
		for(int x = 0; x < DISPLAYWIDTH; x++)
		{
			Platform.drawPixel(x, y, colour);
		}
	}
}


void SDLPlatform::playSound(const uint16_t* sound)
{
	Play(sound);
//	Play((const uint16_t*) (diskContents + Data_audio + Data_AudioPatterns[id]));
}

void writeSaveFile(uint8_t* buffer, int length)
{
	FILE* fs;
	
	if (!fopen_s(&fs, "save.sav", "wb"))
	{
		fwrite(buffer, 1, length, fs);
		fclose(fs);
	}
}

bool readSaveFile(uint8_t* buffer, int length)
{
	FILE* fs;

	if (!fopen_s(&fs, "save.sav", "rb"))
	{
		fread(buffer, 1, length, fs);
		fclose(fs);
		return true;
	}

	return false;
}
