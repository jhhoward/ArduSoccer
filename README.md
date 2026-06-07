# ArduSoccer

![Demo](Assets/demo.gif)

ArduSoccer is a 5-a-side soccer game for the [Arduboy](https://www.arduboy.com). You can play against a computer opponent, or if you have 2 Arduboys you can play multiplayer.

Download the latest package from the [releases page](https://github.com/jhhoward/ArduSoccer/releases). You can also [play in your browser using the Ardens emulator](https://tiberiusbrown.github.io/Ardens/player.html?blah=https://raw.githubusercontent.com/jhhoward/ArduSoccer/refs/heads/master/Package/ArduSoccer.hex)

## Controls
When your player has the ball:
* A: Pass to another player
* B: Shoot in the direction the player is facing

When your player doesn't have the ball:
* A: Select a different player
* B: Tackle

## Multiplayer instructions
To play ArduSoccer in multiplayer mode you will need two Arduboys with the game installed and a computer to connect them to. You can use the computer to act as a bridge to help the two Arduboys communicate.

* Connect the Arduboys to your computer's USB ports and switch them on
* On your computer's browser go to https://jhhoward.github.io/SerialRelay/
* In the Arduboy Serial Relay web app, use the 'Connect Player A' and 'Connect Player B' and select your connected Arduboys
* On both Arduboys choose 'Multiplayer' from the menu
* If everything is set up correctly, the match will start!

## Build instructions
To compile from source you will need the [Arduboy2 library](https://github.com/MLXXXp/Arduboy2) and the [ArduboyTones library](https://github.com/MLXXXp/ArduboyTones) as well as the Arduino IDE


