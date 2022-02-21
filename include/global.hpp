#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_gamecontroller.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include "json.hpp"
#include "picopng.hpp"
using json = nlohmann::json;

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define ui_height 80

// #define range (double x, double in_min, double in_max, double out_min, double out_max)((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)
// #define constrain (double x, double min, double max)(std::min(std::max(x, min), max));

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

double range (double x, double in_min, double in_max, double out_min, double out_max);
double constrain (double x, double min, double max);

#endif
