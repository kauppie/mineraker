# Mineraker
Mineraker is derived from the Minesweeper game, but only a *pre-alpha like* version is available, at this time.

## Installing
If you already have CMake and SDL2 installed, you can skip to building.

### CMake and SDL2
Run following commands to install CMake and required development libraries on Debian based systems.
```shell
sudo apt update
sudo apt install cmake libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```

### Building

Build in-place using following commands from the project directory.
```shell
cmake .
make
```
