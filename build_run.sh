#!/bin/bash

clear

g++ -c src/*.cpp -std=c++14 -m64 -g -Wall && g++ *.o -o bin/debug/main -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer && ./bin/debug/main
