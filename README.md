# A Game of Life Implemented in C

This is a progam wich implements the Conway's "Game of life" using C and the graphic library SDL2. The game is a Cellular Automaton that involves cells that can be dead or alive, the simulation evolves over time based on simple rules. The program was coded for educational purposes.

## Features

- Implementation of Conway's "Game of Life" in the C language.
- Simple graphical interface using the SDL2 library.
- Ability to interact with the simulation, add and remove cells, and control the simulation.

## Building

### Prerequisites

- Compatible C comiler (e.g., GCC).
- Installed SDL2 library

### Compilation and execution

1. Clone this repo: `$ git clone https://github.com/Matig-lab/agolic.git`
2. Enter to the repo directory: `$ cd agolic`
3. Compile the program: run `./build.sh` or execute `$ gcc -o agolic gameoflife.c -lm -lSDL2`
4. Run the program: `$ ./agolic`

## Instructions

- **SPACE:** Start or pause the simulation.
- **R**: Restart the simulation.
- **Left Click**: Place live cells.
- **Right Click**: Remove live cells.
- **Mouse Wheel** or **+**, **-**: Adjust zoom.
- **C**: Center the grid in screen.
- **ESC** or **Q**: Quits the program.
