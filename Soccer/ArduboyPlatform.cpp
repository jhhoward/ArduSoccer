#include "ArduboyPlatform.h"
//#include "Generated/Data_Audio.h"

ArduboyPlatform Platform;

uint8_t debugValue1 = 0, debugValue2 = 0;

void ArduboyPlatform::updateInput()
{
	lastInputState[LOCAL_PLAYER] = inputState[LOCAL_PLAYER];
	inputState[LOCAL_PLAYER] = 0;
	  
	if(arduboy.pressed(A_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Btn_A;  
	}
	if(arduboy.pressed(B_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Btn_B;  
	}
	if(arduboy.pressed(UP_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Dpad_Up;  
	}
	if(arduboy.pressed(DOWN_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Dpad_Down;  
	}
	if(arduboy.pressed(LEFT_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Dpad_Left;  
	}
	if(arduboy.pressed(RIGHT_BUTTON))
	{
		inputState[LOCAL_PLAYER] |= Input_Dpad_Right;  
	}
}

void ArduboyPlatform::update()
{
	if(arduboy.audio.enabled() != !m_isMuted)
	{
		if(m_isMuted)
		{
			arduboy.audio.off();
		}
		else
		{
			arduboy.audio.on();
		}
	}

	if(connectionStatus == ConnectionStatus::Disconnected)
	{
		updateInput();
	}
	else
	{
		arduboy.setRGBled(0, 0, 0);
		
		updateInput();
		sendNetworkPacket();
		isWaitingForRemote = true;
		
		while(isWaitingForRemote)
		{
			parseNetwork();
			
			if((millis() - lastPacketSentTime) > 1000)
			{
				arduboy.setRGBled(0, 255, 255);
				sendNackPacket();
			}
			
			Serial.flush();
		}
	}
}

constexpr uint8_t FRAME_START = 0xfa;
constexpr uint8_t NACK_START = 0xfb;
constexpr uint8_t PACKET_SIZE = 4;

uint8_t networkBuffer[PACKET_SIZE];
uint8_t networkBufferSize = 0;

void ArduboyPlatform::sendNetworkPacket()
{
	if(Serial.availableForWrite() >= PACKET_SIZE)
	{
		uint8_t checkSum = FRAME_START + networkFrame + inputState[LOCAL_PLAYER];
		
		Serial.write(FRAME_START);
		Serial.write(networkFrame);
		Serial.write(inputState[LOCAL_PLAYER]);
		Serial.write(checkSum);
		
		lastPacketSentTime = millis();
	}
}

void ArduboyPlatform::sendNackPacket()
{
	if(Serial.availableForWrite() >= PACKET_SIZE)
	{
		uint8_t checkSum = NACK_START + networkFrame;
		
		Serial.write(NACK_START);
		Serial.write(networkFrame);
		Serial.write((uint8_t)0);
		Serial.write(checkSum);
		
		lastPacketSentTime = millis();
	}
}

void ArduboyPlatform::parseNetwork()
{
	debugValue1 = networkFrame;
	
	while(Serial.available())
	{
		if(networkBufferSize == 0)
		{
			uint8_t packetType = Serial.read();
			if(packetType == FRAME_START || packetType == NACK_START)
			{
				networkBuffer[networkBufferSize++] = packetType;
			}
		}
		else
		{
			networkBuffer[networkBufferSize++] = Serial.read();
		}
	
		if(networkBufferSize == PACKET_SIZE)
		{
			networkBufferSize = 0;
			
			uint8_t packetType = networkBuffer[0];
			uint8_t remoteFrame = networkBuffer[1];
			uint8_t remoteButtons = networkBuffer[2];
			uint8_t checkSum = networkBuffer[3];
			
			uint8_t testChecksum = packetType + remoteFrame + remoteButtons;

			debugValue2 = remoteFrame;

			if(checkSum != testChecksum)
			{
				// Corrupt packet, ignore
				arduboy.setRGBled(255, 0, 0);
				sendNackPacket();
				continue;
			}
			
			lastPacketSentTime = millis();

			if(packetType == FRAME_START)
			{
				if(remoteFrame == networkFrame)
				{
					// Received successful frame
					arduboy.setRGBled(0, 255, 0);
					lastInputState[REMOTE_PLAYER] = inputState[REMOTE_PLAYER];
					inputState[REMOTE_PLAYER] = remoteButtons;
					networkFrame++;
					isWaitingForRemote = false;
					return;
				}
				else
				{
					sendNackPacket();
				}
			}
			else if(packetType == NACK_START)
			{
				if(remoteFrame == networkFrame)
				{
					// Resend packet
					sendNetworkPacket();
				}
				else if(remoteFrame == (uint8_t)(networkFrame - 1))
				{
					// Resend previous packet
					uint8_t temp = inputState[LOCAL_PLAYER];
					inputState[LOCAL_PLAYER] = lastInputState[LOCAL_PLAYER];
					networkFrame--;
					
					sendNetworkPacket();
					
					networkFrame++;
					inputState[LOCAL_PLAYER] = temp;
				}
				else
				{
					arduboy.setRGBled(255, 255, 255);
					continue;
					//arduboy.displayOff();
					//while(1);
				}
			}
			
			// Flush network buffer
			while(Serial.available())
			{
				Serial.read();
			}
			
		}
	}
}


void ArduboyPlatform::connectMultiplayer(bool isHost)
{
	Serial.begin(115200);

	connectionStatus = isHost ? ConnectionStatus::SerialHost : ConnectionStatus::SerialClient;
	networkFrame = 0;
	isWaitingForRemote = false;
}

