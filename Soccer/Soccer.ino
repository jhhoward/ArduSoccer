#include <Arduboy2.h>       
#include <ArduboyTones.h>
#include "Engine.h"
#include "ArduboyPlatform.h"
//#include "Generated/Data_Audio.h"

Arduboy2Base arduboy;
ArduboyTones sound(arduboy.audio.enabled);

unsigned long lastTimingSample;

constexpr int noiseBufferSize = 65;
constexpr uint16_t noiseLength = 1;
constexpr uint16_t noiseLowFrequency = 400; //1000;
constexpr uint16_t noiseHighFrequency = 1600;
int16_t noiseFrequencyDelta = 20;
uint16_t noiseBuffer[noiseBufferSize];
uint16_t noiseFrequency = 1000;
bool playingNoise = false;

void ArduboyPlatform::playSound(const uint16_t* pattern)
{
	sound.tones(pattern);
	playingNoise = false;
}

uint16_t generateRandom()
{
	static uint16_t randVal = 0xABC;

    uint16_t lsb = randVal & 1;
    randVal >>= 1;
    if (lsb == 1)
    	randVal ^= 0xB400u;

	return randVal - 1;
}

void ArduboyPlatform::playNoise()
{
	sound.tonesInRAM(noiseBuffer);
	playingNoise = true;
	noiseFrequency = noiseLowFrequency;
	noiseFrequencyDelta = 20;
}

void ArduboyPlatform::playNoiseOut()
{
	noiseFrequencyDelta = -40;
}

void setup() {
	arduboy.boot();
	//arduboy.flashlight();
	//arduboy.systemButtons();
	//arduboy.bootLogo();
	arduboy.setFrameRate(TARGET_FRAMERATE);
	arduboy.audio.begin();
	arduboy.audio.on();

	for(int n = 1; n < noiseBufferSize - 1; n+=2)
	{
		noiseBuffer[n] = noiseLength;
	}
	noiseBuffer[noiseBufferSize - 1] = TONES_REPEAT;

	engine.init();
}

void loop() {
	//static int16_t tickAccum = 0;
	//unsigned long timingSample = millis();
	//tickAccum += (timingSample - lastTimingSample);
	//lastTimingSample = timingSample;

	//Platform.updateNetwork();

	if(playingNoise)
	{
		for(int n = 0; n < noiseBufferSize - 1; n += 2)
		{
			noiseBuffer[n] = noiseFrequency + (generateRandom() & 1023);
		}
	}

	if (!arduboy.nextFrame()) return; 

	Platform.update();

	if(playingNoise)
	{
		if(noiseFrequencyDelta < 0)
		{
			if(noiseFrequency > -noiseFrequencyDelta)
			{
				noiseFrequency += noiseFrequencyDelta;
			}		  
			else
			{
				sound.noTone();
				playingNoise = false;
			}
		}
		else if(noiseFrequency < noiseHighFrequency)
		{
			noiseFrequency += noiseFrequencyDelta;
		}
	}

	/*  constexpr int16_t frameDuration = 1000 / TARGET_FRAMERATE;
	while(tickAccum > frameDuration)
	{
	engine.update();
	tickAccum -= frameDuration;
	}*/
	engine.update();

	engine.draw();

	arduboy.display(true);
}
