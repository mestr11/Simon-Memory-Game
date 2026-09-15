# Memory-Game
A recreation of the classic simon memory game using an STM32 microcontroller

## Preview

TODO: Insert youtube video

## Overview

* Recreates the Simon memory game using distinct states including `DISPLAY_SEQUENCE` and `USER_INPUT`
* Employs pulse width modulation to determine the frequency of the passive buzzer, allowing each color to have a unique tone.
* Utilizes UART transmissions to output game state transitions.

## Hardware used
* 1 STM32 Nucleo F401RE microcontroller
* 4 Tactile Buttons
* 4 Colored LEDs (Red, Blue, Yellow, Green)
* 4 220-ohm resistors
* 1 passive buzzer 

## Tech Stack
* STM32CubeMX (Project Initialization)
* C Programming Language
* Visual Studio Code
* STM32CubeIDE Extension pack for Visual Studio Code

## Pinout Configuration

<img width="892" height="787" alt="image" src="https://github.com/user-attachments/assets/82e13bdc-7332-4e23-83c6-5454cd048ea2" />

## Building and Flashing

1. **Prerequisites:** Ensure you have STM32CubeMX, VS Code, and the official STM32CubeIDE Extension Pack installed.
2. **Generate Code:** Open `MemoryGame.ioc` in STM32CubeMX and click **GENERATE CODE** to create the necessary HAL and project files.
3. **Open Workspace:** Open the project folder in VS Code. When prompted, select **Yes** to configure the CMake project as an STM32Cube project.
4. **Build:** Change the build variant to **Release** in the status bar. The project will compile automatically.
5. **Flash:** Click **Run and Debug** on the side bar (or press `F5`). Select **`STM32Cube: STM32 launch ST-Link GDB Server`** from the configuration dropdown to flash the board.

## Playing the game

1. Press the blue user button on the nucleo board to start the game.
2. The game will begin by displaying the first color and playing its unique tone.
3. Press the button associated with the displayed color.
4. The sequence length increases by one color each round.
5. If the wrong color is pressed, the correct color LED flashes while a game over tone plays.

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
