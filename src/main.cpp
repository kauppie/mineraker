#include <chrono>
#include <iostream>
#include <random>

#include "SDL2/SDL.h"

#include "gamemanager.hpp"
#include "sweepboard.hpp"
#include "sweepboardformat.hpp"
#include "sweepboardcontroller.hpp"
#include "windowmanager.hpp"

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_VIDEO);
  msgn::WindowManager wm;
  wm.create_window("test", 480,320,100,100,0);

  /*
  tasks.json:
  {
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build release",
            "type": "shell",
            "command": "g++ src/main.cpp -o main -std=c++1z -O3 -Wall `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf"
        },
        {
            "label": "build debug",
            "type": "shell",
            "command": "g++ src/main.cpp -o main-d -g -std=c++1z -Wall `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_image -lSDL2_ttf"
        }
    ]
}
  */
 /*
 launch.json:
 {
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [

        {
            "name": "Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/main-d",
            //"args": ["-g", "-O3", "Wall"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build debug"
        },
        {
            "name": "Release",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/main",
            //"args": ["-g", "-O3", "Wall"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build optimized"
        }
    ]
}
 */
  /*
  c_cpp_properties.json:
  {
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "/usr/include",
                "${workspaceFolder}/src"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-x64"
        }
    ],
    "version": 4
}
  */

  return 0;
}