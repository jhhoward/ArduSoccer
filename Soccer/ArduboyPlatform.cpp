#include "ArduboyPlatform.h"
#include "Engine.h"
//#include "Generated/Data_Audio.h"

ArduboyPlatform Platform;

constexpr uint8_t UPDATE_PACKET = 0xfa;
constexpr uint8_t NACK_PACKET = 0xfb;
constexpr uint8_t SYNC_PACKET = 0xfc;
constexpr uint8_t PACKET_SIZE = 4;

uint8_t networkBuffer[PACKET_SIZE];
uint8_t networkBufferSize = 0;

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
		hostId++;
	}
	else
	{
		arduboy.setRGBled(0, 0, 0);
		
		updateInput();
		sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
		isWaitingForRemote = true;
		
		while(isWaitingForRemote)
		{
			parseNetwork();
			
			if((millis() - lastPacketSentTime) > 1000)
			{
				arduboy.setRGBled(255, 0, 0);
				sendNetworkPacket(NACK_PACKET);
			}
			
			Serial.flush();
		}
	}
}

void ArduboyPlatform::sendNetworkPacket(uint8_t packetType, uint8_t data)
{
	if(Serial.availableForWrite() >= PACKET_SIZE)
	{
		uint8_t checkSum = packetType + networkFrame + data;
		
		Serial.write(packetType);
		Serial.write(networkFrame);
		Serial.write(data);
		Serial.write(checkSum);
		
		lastPacketSentTime = millis();
	}
}

void ArduboyPlatform::parseNetwork()
{
	while(Serial.available())
	{
		if(networkBufferSize == 0)
		{
			uint8_t packetType = Serial.read();
			if(packetType == UPDATE_PACKET || packetType == NACK_PACKET || packetType == SYNC_PACKET)
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
			uint8_t remoteData = networkBuffer[2];
			uint8_t checkSum = networkBuffer[3];
			
			uint8_t testChecksum = packetType + remoteFrame + remoteData;

			if(checkSum != testChecksum)
			{
				// Corrupt packet, ignore
				arduboy.setRGBled(255, 0, 0);
				sendNetworkPacket(NACK_PACKET);
				continue;
			}
			
			lastPacketSentTime = millis();

			if(packetType == SYNC_PACKET && connectionStatus == ConnectionStatus::Disconnected)
			{
				if(hostId > remoteData)
				{
					connectionStatus = ConnectionStatus::SerialHost;
				}
				else if(hostId < remoteData)
				{
					connectionStatus = ConnectionStatus::SerialClient;
				}
			}
			else if(packetType == UPDATE_PACKET && connectionStatus != ConnectionStatus::Disconnected)
			{
				if(remoteFrame == networkFrame)
				{
					// Received successful frame
					lastInputState[REMOTE_PLAYER] = inputState[REMOTE_PLAYER];
					inputState[REMOTE_PLAYER] = remoteData;
					networkFrame++;
					isWaitingForRemote = false;
				}
				else
				{
					sendNetworkPacket(NACK_PACKET);
				}
			}
			else if(packetType == NACK_PACKET && connectionStatus != ConnectionStatus::Disconnected)
			{
				if(remoteFrame == networkFrame)
				{
					// Resend packet
					sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
				}
				else if(remoteFrame == (uint8_t)(networkFrame - 1))
				{
					// Resend previous packet
					networkFrame--;
					sendNetworkPacket(UPDATE_PACKET, lastInputState[LOCAL_PLAYER]);
					networkFrame++;
				}
				else
				{
					arduboy.setRGBled(255, 0, 0);
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

void ArduboyPlatform::disconnectMultiplayer()
{
	Serial.end();
	connectionStatus = ConnectionStatus::Disconnected;
}

bool ArduboyPlatform::connectMultiplayer()
{
	Serial.begin(115200);
	
	// Show connecting screen
	arduboy.display(true);
	
	networkFrame = 0;
	isWaitingForRemote = false;
	connectionStatus = ConnectionStatus::Disconnected;

	sendNetworkPacket(SYNC_PACKET, hostId);

	while(connectionStatus == ConnectionStatus::Disconnected)
	{
		if((millis() - lastPacketSentTime) > 1000)
		{
			sendNetworkPacket(SYNC_PACKET, hostId);
		}
		
		parseNetwork();
	}
	
	sendNetworkPacket(SYNC_PACKET, hostId);
	
	return connectionStatus == ConnectionStatus::SerialHost;
}

